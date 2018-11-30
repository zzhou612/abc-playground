#ifndef ECTL_SIMULATOR_H
#define ECTL_SIMULATOR_H

#include <unordered_map>
#include <abc_api.h>
#include <ectl_network.h>
#include <ectl_utils.h>

namespace ECTL {
    double _SimER(const NtkPtr &origin, const NtkPtr &approx, bool show_progress_bar = false, int sim_time = 100000);

    double SimER(const NtkPtr &origin, const NtkPtr &approx, bool show_progress_bar = false, int sim_time = 100000);

    std::unordered_map<ObjPtr, std::vector<int>> SimTruthVec(const NtkPtr &ntk, bool show_progress_bar = false, int sim_time = 100000);
}

#endif
