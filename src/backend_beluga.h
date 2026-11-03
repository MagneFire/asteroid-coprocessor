// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef BACKEND_BELUGA_H
#define BACKEND_BELUGA_H

#include <QTimer>

#include "backend.h"
#include "mcumessages.h"
#include "sensorhubclient.h"

namespace Coprocessor {

class BelugaBackend : public Backend
{
    Q_OBJECT

public:
    explicit BelugaBackend(QObject *parent = nullptr);

    unsigned int capabilities() const override;
    bool prepareTimepiece() override;
    bool enterTimepiece() override;
    void leaveTimepiece() override;
    void aboutToShutDown() override;

private:
    void onConnected();
    void onMessage(int msgId, const QByteArray &payload);
    void sendHandshake();
    void onHandshakeTimeout();
    bool sendWorkMode(Mcu::WorkMode mode);

    SensorHubClient m_client;
    QTimer m_handshakeTimer;
    QTimer m_resetSettleTimer;
    int m_handshakeAttempts = 0;
    bool m_mcuReady = false;
    bool m_shuttingDown = false;
    bool m_wristBandEntered = false;
};

}

#endif
