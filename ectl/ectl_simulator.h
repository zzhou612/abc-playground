#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "abc_api.h"
#include "ectl_types.h"

namespace ECTL {
    void ComSimSopOnce(Network pNtk);
    void PrintSimRes(Network pNtk);
}

#endif
