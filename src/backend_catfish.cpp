#include "backend_catfish.h"

#include <QDebug>

using namespace AsteroidOS::CoprocessorDaemon;

int CatfishBackend::SynchronizeTime()
{
    qDebug() << "catfish: Syncing time";
    return 1;
}