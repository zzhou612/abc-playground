#ifndef ECTL_SIMULATOR_H
#define ECTL_SIMULATOR_H

#include <unordered_map>
#include <abc_api.h>
#include <ectl_network.h>
#include <ectl_utils.h>

namespace ECTL {
    double SimErrorRate(const NetworkPtr &origin, const NetworkPtr &approx, bool show_progress_bar = false,
                        int sim_time = 100000);

    std::unordered_map<ObjectPtr, std::vector<int>> SimTruthVector(const NetworkPtr &ntk,
                                                                   bool show_progress_bar = false,
                                                                   int sim_time = 100000);

    void SimTest(NetworkPtr ntk);
}

#endif
