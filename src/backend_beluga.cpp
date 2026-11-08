// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend_beluga.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QFile>
#include <QThread>

namespace Coprocessor {

namespace {

constexpr const char *BatteryCapacityPath = "/sys/class/power_supply/battery/capacity";
constexpr const char *McuResetPath = "/sys/devices/platform/soc/soc:oppo,sensor-hub/reset";
constexpr int HandshakeTimeoutMs = 3000;
// After a power off the MCU may not answer until it is reset.
constexpr int HandshakeMaxAttempts = 4;
constexpr int ResetSettleMs = 1000;
// Writing 0 hands the panel to the MCU.
constexpr const char *ReleasePanelPath = "/sys/devices/virtual/graphics/fb0/mdss_fb_disp_en_gpio";
// The MCU only takes over a panel that is powered on.
constexpr unsigned long UnblankSettleMs = 1000;
constexpr unsigned long SendRetryMs = 100;
// The reply is not awaited, so give the MCU a moment.
constexpr unsigned long PowerOffSettleMs = 500;

int batteryCapacity()
{
    QFile file(BatteryCapacityPath);
    if (!file.open(QIODevice::ReadOnly))
        return 0;
    return file.readAll().trimmed().toInt();
}

bool unblankDisplay()
{
    const QDBusMessage request = QDBusMessage::createMethodCall(
        "com.nokia.mce", "/com/nokia/mce/request", "com.nokia.mce.request", "req_display_state_on");
    const QDBusMessage reply = QDBusConnection::systemBus().call(request);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Unable to unblank the display:" << reply.errorMessage();
        return false;
    }
    QThread::msleep(UnblankSettleMs);
    return true;
}

bool resetMcu()
{
    QFile file(McuResetPath);
    if (!file.open(QIODevice::WriteOnly) || file.write("1") != 1) {
        qWarning() << "Unable to reset the MCU:" << file.errorString();
        return false;
    }
    return true;
}

bool releasePanel()
{
    QFile file(ReleasePanelPath);
    if (!file.open(QIODevice::WriteOnly) || file.write("0") != 1) {
        qWarning() << "Unable to release the panel:" << file.errorString();
        return false;
    }
    return true;
}

}

BelugaBackend::BelugaBackend(QObject *parent)
    : Backend(parent)
{
    m_handshakeTimer.setSingleShot(true);
    m_handshakeTimer.setInterval(HandshakeTimeoutMs);
    connect(&m_handshakeTimer, &QTimer::timeout, this, &BelugaBackend::onHandshakeTimeout);
    m_resetSettleTimer.setSingleShot(true);
    m_resetSettleTimer.setInterval(ResetSettleMs);
    connect(&m_resetSettleTimer, &QTimer::timeout, this, &BelugaBackend::sendHandshake);
    connect(&m_client, &SensorHubClient::connected, this, &BelugaBackend::onConnected);
    connect(&m_client, &SensorHubClient::disconnected, this, [this] {
        m_handshakeTimer.stop();
        m_resetSettleTimer.stop();
        m_mcuReady = false;
        emit capabilitiesChanged();
    });
    connect(&m_client, &SensorHubClient::messageReceived, this, &BelugaBackend::onMessage);
    m_client.connectToService();
}

void BelugaBackend::onConnected()
{
    m_handshakeAttempts = 0;
    sendHandshake();
}

void BelugaBackend::sendHandshake()
{
    ++m_handshakeAttempts;
    sendWorkMode(Mcu::WorkMode::Normal);
    m_handshakeTimer.start();
}

void BelugaBackend::onHandshakeTimeout()
{
    if (m_shuttingDown)
        return;
    if (m_handshakeAttempts >= HandshakeMaxAttempts) {
        qWarning() << "The MCU did not answer after" << m_handshakeAttempts << "attempts";
        return;
    }
    if (m_handshakeAttempts == 1) {
        qWarning() << "The MCU did not answer, sending again";
        sendHandshake();
        return;
    }
    qWarning() << "The MCU did not answer, resetting it";
    resetMcu();
    m_resetSettleTimer.start();
}

void BelugaBackend::onMessage(int msgId, const QByteArray &payload)
{
    if (msgId != Mcu::WorkModeReply) {
        qDebug() << "MCU message" << msgId << payload.toHex();
        return;
    }
    Mcu::WorkModeResponse response;
    if (!Mcu::parseWorkModeResponse(payload, &response)) {
        qWarning() << "Malformed work mode response from the MCU";
        return;
    }
    qInfo() << "MCU work mode" << response.mode << "last mode" << response.lastMode << "reboot reason"
            << response.rebootReason << "clock" << response.time << response.timezone;
    if (m_shuttingDown || (!m_handshakeTimer.isActive() && m_mcuReady))
        return;
    m_handshakeTimer.stop();
    m_resetSettleTimer.stop();
    m_client.send(Mcu::BootAck);
    if (!sendRetrying(Mcu::BootNotify, Mcu::bootNotifyPayload())
        || !sendRetrying(Mcu::OobeNotify, Mcu::oobeNotifyPayload()))
        qWarning() << "Unable to tell the MCU the setup is done";
    m_mcuReady = true;
    emit capabilitiesChanged();
}

unsigned int BelugaBackend::capabilities() const
{
    return m_client.isConnected() && m_mcuReady ? Capability::TimepieceMode : 0;
}

bool BelugaBackend::sendRetrying(Mcu::MessageId message, const QByteArray &payload)
{
    if (m_client.send(message, payload))
        return true;
    QThread::msleep(SendRetryMs);
    return m_client.send(message, payload);
}

bool BelugaBackend::sendWorkMode(Mcu::WorkMode mode)
{
    return sendRetrying(Mcu::workModeMessage(mode), Mcu::workModePayload(mode));
}

bool BelugaBackend::prepareTimepiece()
{
    return m_client.connectToService();
}

bool BelugaBackend::enterTimepiece()
{
    if (!unblankDisplay() || !m_client.send(Mcu::BatteryInfo, Mcu::batteryInfoPayload(batteryCapacity()))
        || !sendWorkMode(Mcu::WorkMode::WristBand))
        return false;
    if (!releasePanel()) {
        sendWorkMode(Mcu::WorkMode::Normal);
        return false;
    }
    m_wristBandEntered = true;
    return true;
}

void BelugaBackend::leaveTimepiece()
{
    if (!m_wristBandEntered)
        return;
    qWarning() << "The power off failed, the panel may stay with the MCU";
    m_wristBandEntered = false;
    sendWorkMode(Mcu::WorkMode::Normal);
}

void BelugaBackend::aboutToShutDown()
{
    m_shuttingDown = true;
    m_handshakeTimer.stop();
    m_resetSettleTimer.stop();
    if (m_wristBandEntered || !m_client.isConnected())
        return;
    if (sendWorkMode(Mcu::WorkMode::PowerOff))
        QThread::msleep(PowerOffSettleMs);
}

}
