#ifndef ECTL_SIMULATOR_H
#define ECTL_SIMULATOR_H

#include <ectl_utils.h>

namespace ECTL {
//    double SimErrorRate(Network_t origin, Network_t approx, bool show_progress_bar = false, int simu_time = 100000);
    void SimTest(NetworkPtr ntk);
}

#endif
