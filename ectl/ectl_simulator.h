#ifndef ECTL_SIMULATOR_H
#define ECTL_SIMULATOR_H

#include <abc_api.h>
#include <ectl_network.h>
#include <ectl_utils.h>

namespace ECTL {
    double SimErrorRate(const NetworkPtr &origin, const NetworkPtr &approx, bool show_progress_bar = false, int sim_time = 100000);
    void SimTest(NetworkPtr ntk);
}

#endif
