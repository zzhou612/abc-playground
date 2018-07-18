#ifndef ECTL_SIMULATOR_H
#define ECTL_SIMULATOR_H

#include <ectl_utils.h>

namespace ECTL {
    double SimErrorRate(const NetworkPtr &origin, const NetworkPtr &approx, bool show_progress_bar = false, int sim_time = 100000);
    void SimTest(NetworkPtr ntk);
}

#endif
