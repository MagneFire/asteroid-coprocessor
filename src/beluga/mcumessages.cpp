// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "mcumessages.h"

#include <cstdint>

namespace Coprocessor::Mcu {

namespace {

enum WireType { Varint = 0, LengthDelimited = 2 };

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
namespace WorkModeResponseField {
enum { Mode = 1, LastMode = 3, SysTime = 4, RebootReason = 5 };
}
namespace ReadSysTimeField {
enum { Time = 2, Timezone = 3 };
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
    appendVarint(out, (static_cast<std::uint64_t>(field) << 3) | Varint);
    appendVarint(out, static_cast<std::uint64_t>(static_cast<std::int64_t>(value)));
}

void appendSInt32(QByteArray &out, int field, std::int32_t value)
{
    appendInt32(out, field, (value << 1) ^ (value >> 31));
}

std::int32_t decodeSInt32(std::uint64_t value)
{
    const auto raw = static_cast<std::uint32_t>(value);
    return static_cast<std::int32_t>((raw >> 1) ^ -(raw & 1));
}

bool readVarint(const QByteArray &in, int *position, std::uint64_t *value)
{
    *value = 0;
    for (int shift = 0; shift < 64 && *position < in.size(); shift += 7) {
        const auto byte = static_cast<std::uint8_t>(in.at((*position)++));
        *value |= static_cast<std::uint64_t>(byte & 0x7f) << shift;
        if (!(byte & 0x80))
            return true;
    }
    return false;
}

// Calls field(number, value, bytes) for each varint or length-delimited field.
template<typename Field>
bool readFields(const QByteArray &in, Field field)
{
    int position = 0;
    while (position < in.size()) {
        std::uint64_t key = 0;
        if (!readVarint(in, &position, &key))
            return false;
        const auto number = static_cast<int>(key >> 3);
        const auto type = static_cast<int>(key & 7);
        std::uint64_t value = 0;
        if (!readVarint(in, &position, &value))
            return false;
        if (type == Varint) {
            field(number, value, QByteArray());
        } else if (type == LengthDelimited) {
            if (value > static_cast<std::uint64_t>(in.size() - position))
                return false;
            field(number, 0, in.mid(position, static_cast<int>(value)));
            position += static_cast<int>(value);
        } else {
            return false;
        }
    }
    return true;
}

void appendMessage(QByteArray &out, int field, const QByteArray &body)
{
    appendVarint(out, (static_cast<std::uint64_t>(field) << 3) | LengthDelimited);
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
    appendSInt32(systemTime, SysTimeField::Timezone, now.offsetFromUtc() / 60);

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

bool parseWorkModeResponse(const QByteArray &payload, WorkModeResponse *response)
{
    *response = WorkModeResponse();
    return readFields(payload, [response](int number, std::uint64_t value, const QByteArray &bytes) {
        switch (number) {
        case WorkModeResponseField::Mode:
            response->mode = static_cast<int>(value);
            break;
        case WorkModeResponseField::LastMode:
            response->lastMode = static_cast<int>(value);
            break;
        case WorkModeResponseField::SysTime:
            readFields(bytes, [response](int timeField, std::uint64_t timeValue, const QByteArray &) {
                if (timeField == ReadSysTimeField::Time)
                    response->time = static_cast<qint64>(static_cast<std::int32_t>(timeValue));
                else if (timeField == ReadSysTimeField::Timezone)
                    response->timezone = decodeSInt32(timeValue);
            });
            break;
        case WorkModeResponseField::RebootReason:
            response->rebootReason = static_cast<int>(value);
            break;
        default:
            break;
        }
    });
}

}
