#ifndef ECTL_UTILS_H
#define ECTL_UTILS_H

#include <iostream>
#include <vector>
#include <ectl_types.h>

namespace ECTL {
    Frame StartAbc();

    void StopABC();

    Network ReadBlif(std::string ifile);

    std::string GetNetworkName(Network ntk);

    void ShowNetworkInfo(Network ntk);

    void SetNetworkName(Network ntk, const std::string &new_name);

    void DeleteNetwork(Network ntk);

    std::vector<Object> TopologicalSort(Network ntk);

    std::vector<Object> GetPrimaryInputs(Network ntk);

    std::vector<Object> GetPrimaryOutputs(Network ntk);

    std::vector<Object> GetInternalNodes(Network ntk);

    std::string GetNodeName(Object obj);

}

#endif
