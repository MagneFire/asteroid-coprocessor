// SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COPROCESSOR_TYPES_H
#define COPROCESSOR_TYPES_H

namespace Coprocessor {

inline constexpr char ServiceName[] = "org.asteroid.Coprocessor";
inline constexpr char DisplayPath[] = "/Display";
inline constexpr char HandsPath[] = "/Hands";
inline constexpr char DisplayInterface[] = "org.asteroid.Coprocessor.Display";
inline constexpr char HandsInterface[] = "org.asteroid.Coprocessor.Hands";

namespace Capability {
enum : unsigned int {
    TimeSync = 1u << 0,
    TimepieceMode = 1u << 1,
    StepCounter = 1u << 2,
    Hands = 1u << 6,
};
}

}

#endif
