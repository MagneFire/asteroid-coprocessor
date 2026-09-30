// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend.h"

#include <QDebug>

namespace Coprocessor {

std::unique_ptr<Backend> createBackend(const QString &machine)
{
    qInfo() << machine << "has no coprocessor support";
    return std::make_unique<Backend>();
}

}
