// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef MCUMESSAGES_H
#define MCUMESSAGES_H

#include <QByteArray>
#include <QDateTime>

// Payloads are the wear.framework.mcuproto.nano protobufs.
namespace Coprocessor::Mcu {

enum MessageId {
    BatteryInfo = 372,
    WorkModeNormal = 373,
    WorkModePowerOff = 374,
    WorkModeWristBand = 375,
    WorkModeReply = 377,
    OobeNotify = 384,
    BootNotify = 386,
    BootAck = 387,
};

enum class WorkMode { Normal = 0, WristBand = 1, PowerOff = 2 };

MessageId workModeMessage(WorkMode mode);

QByteArray workModePayload(WorkMode mode, const QDateTime &now = QDateTime::currentDateTime());

QByteArray batteryInfoPayload(int capacity, int voltage = 0);

QByteArray oobeNotifyPayload();

QByteArray bootNotifyPayload(const QDateTime &now = QDateTime::currentDateTime());

struct WorkModeResponse
{
    int mode = 0;
    int lastMode = 0;
    // 0 debug, 1 external pin, 2 fault, 3 other.
    int rebootReason = 0;
    // Unix seconds and UTC offset minutes, 0 if absent.
    qint64 time = 0;
    int timezone = 0;
};

bool parseWorkModeResponse(const QByteArray &payload, WorkModeResponse *response);

}

#endif
