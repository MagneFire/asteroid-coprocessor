// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "mcumessages.h"

#include <cstdint>

namespace Coprocessor::Mcu {

namespace {

// Field numbers of the protobuf messages.
namespace SysTimeField {
enum { Time = 1, Timezone = 3 };
}
namespace WorkModeField {
enum { Mode = 1, SysTime = 2 };
}
namespace BatteryInfoField {
enum { Voltage = 1, Capacity = 2 };
}
namespace PreferenceField {
enum { Cmd = 1, BatteryInfo = 15 };
}

constexpr int PreferenceCmdBatteryInfo = 17;

void appendVarint(QByteArray &out, std::uint64_t value)
{
    do {
        std::uint8_t byte = value & 0x7f;
        value >>= 7;
        if (value)
            byte |= 0x80;
        out.append(static_cast<char>(byte));
    } while (value);
}

void appendInt32(QByteArray &out, int field, std::int32_t value)
{
    if (!value)
        return;
    appendVarint(out, static_cast<std::uint64_t>(field) << 3);
    appendVarint(out, static_cast<std::uint64_t>(static_cast<std::int64_t>(value)));
}

void appendMessage(QByteArray &out, int field, const QByteArray &body)
{
    appendVarint(out, (static_cast<std::uint64_t>(field) << 3) | 2);
    appendVarint(out, body.size());
    out.append(body);
}

}

MessageId workModeMessage(WorkMode mode)
{
    switch (mode) {
    case WorkMode::WristBand:
        return WorkModeWristBand;
    case WorkMode::PowerOff:
        return WorkModePowerOff;
    case WorkMode::Normal:
        break;
    }
    return WorkModeNormal;
}

QByteArray workModePayload(WorkMode mode, const QDateTime &now)
{
    QByteArray systemTime;
    appendInt32(systemTime, SysTimeField::Time, static_cast<std::int32_t>(now.toSecsSinceEpoch()));
    appendInt32(systemTime, SysTimeField::Timezone, now.offsetFromUtc() / 60);

    QByteArray payload;
    appendInt32(payload, WorkModeField::Mode, static_cast<std::int32_t>(mode));
    appendMessage(payload, WorkModeField::SysTime, systemTime);
    return payload;
}

QByteArray batteryInfoPayload(int capacity, int voltage)
{
    QByteArray info;
    appendInt32(info, BatteryInfoField::Voltage, voltage);
    appendInt32(info, BatteryInfoField::Capacity, capacity);

    QByteArray payload;
    appendInt32(payload, PreferenceField::Cmd, PreferenceCmdBatteryInfo);
    appendMessage(payload, PreferenceField::BatteryInfo, info);
    return payload;
}

}
