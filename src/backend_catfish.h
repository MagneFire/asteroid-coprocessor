#ifndef BACKEND_CATFISH_H
#define BACKEND_CATFISH_H

#include "backend.h"

namespace AsteroidOS::CoprocessorDaemon
{
class CatfishBackend : public Backend
{
   public:
    CatfishBackend(){};
    virtual ~CatfishBackend(){};
    virtual int SynchronizeTime();
};
};      // namespace AsteroidOS::CoprocessorDaemon
#endif  // BACKEND_CATFISH_H