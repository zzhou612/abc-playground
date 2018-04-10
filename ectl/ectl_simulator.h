#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <map>
#include <ectl_utils.h>
#include <ectl_types.h>

namespace ECTL {
    double SimError(Network origin, Network approx, bool show_progress_bar = false, int simu_time = 100000);
    std::map<Node, double> SimZeroProb(Network ntk, int simu_time = 100000);
    void SimTest(Network ntk);
}

#endif
