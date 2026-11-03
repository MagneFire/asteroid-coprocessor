// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "login1.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>

namespace Coprocessor {

namespace {

constexpr const char *Login1Service = "org.freedesktop.login1";
constexpr const char *Login1Path = "/org/freedesktop/login1";
constexpr const char *Login1Manager = "org.freedesktop.login1.Manager";

}

bool requestPowerOff()
{
    QDBusMessage powerOff = QDBusMessage::createMethodCall(Login1Service, Login1Path, Login1Manager, "PowerOff");
    powerOff << false;
    const QDBusMessage reply = QDBusConnection::systemBus().call(powerOff);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "login1 PowerOff failed:" << reply.errorMessage();
        return false;
    }
    return true;
}

ShutdownWatcher::ShutdownWatcher(Callback callback, QObject *parent)
    : QObject(parent)
    , m_callback(std::move(callback))
{
    QDBusConnection::systemBus().connect(Login1Service, Login1Path, Login1Manager, "PrepareForShutdown", this,
                                         SLOT(onPrepareForShutdown(bool)));
    takeLock();
}

void ShutdownWatcher::takeLock()
{
    QDBusMessage inhibit = QDBusMessage::createMethodCall(Login1Service, Login1Path, Login1Manager, "Inhibit");
    inhibit << QStringLiteral("shutdown") << QStringLiteral("asteroid-coprocd")
            << QStringLiteral("Prepare the coprocessor for power off") << QStringLiteral("delay");
    const QDBusReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(inhibit);
    if (!reply.isValid()) {
        qWarning() << "login1 Inhibit failed:" << reply.error().message();
        return;
    }
    m_lock = reply.value();
}

void ShutdownWatcher::onPrepareForShutdown(bool start)
{
    if (!start) {
        // The shutdown was cancelled; be ready for the next one.
        if (!m_lock.isValid())
            takeLock();
        return;
    }
    m_callback();
    // Closing the descriptor releases the lock and lets the shutdown go on.
    m_lock = QDBusUnixFileDescriptor();
}

}
