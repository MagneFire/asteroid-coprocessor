// SPDX-FileCopyrightText: 2023-2026 Darrel Griët <dgriet@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "backend.h"
#include "backend_casio.h"
#include "backend_narwhal.h"

#ifdef HAVE_HYBRIS
#include "backend_mobvoi.h"
#endif

#ifdef HAVE_GBINDER
#include "backend_beluga.h"
#endif

#include <QDebug>

namespace Coprocessor {

std::unique_ptr<Backend> createBackend(const QString &machine)
{
    if (machine == "koi")
        return std::make_unique<CasioBackend>(Koi);
    if (machine == "medaka")
        return std::make_unique<CasioBackend>(Medaka);
    if (machine == "narwhal")
        return std::make_unique<NarwhalBackend>();
#ifdef HAVE_HYBRIS
    if (machine == "catfish")
        return std::make_unique<MobvoiBackend>(Catfish);
    if (machine == "rubyfish")
        return std::make_unique<MobvoiBackend>(Rubyfish);
#endif
#ifdef HAVE_GBINDER
    if (machine == "beluga")
        return std::make_unique<BelugaBackend>();
#endif
    qInfo() << machine << "has no coprocessor support";
    return std::make_unique<Backend>();
}

}
