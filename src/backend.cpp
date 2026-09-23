// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend.h"
#include "backend_casio.h"

#include <QDebug>

namespace Coprocessor {

std::unique_ptr<Backend> createBackend(const QString &machine)
{
    if (machine == "koi")
        return std::make_unique<CasioBackend>(Koi);
    if (machine == "medaka")
        return std::make_unique<CasioBackend>(Medaka);
    qInfo() << machine << "has no coprocessor support";
    return std::make_unique<Backend>();
}

}
