#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <ectl_utils.h>
#include <ectl_types.h>

namespace ECTL {
    double SimError(Network origin, Network approx, int simu_time = 100000);
    void SimTest(Network ntk);
}

#endif
