#ifndef ECTL_UTILS_H
#define ECTL_UTILS_H

#include <iostream>
#include <vector>
#include <ectl_types.h>

namespace ECTL {
    Frame StartAbc();

    void StopABC();

    Network ReadBlif(std::string ifile);

    void WriteBlif(Network ntk, std::string ofile);

    std::string GetNetworkName(Network ntk);

    void ShowNetworkInfo(Network ntk);

    void SetNetworkName(Network ntk, const std::string &new_name);

    void DeleteNetwork(Network ntk);

    std::vector<Node> TopologicalSort(Network ntk);

    std::vector<Node> GetPrimaryInputs(Network ntk);

    std::vector<Node> GetPrimaryOutputs(Network ntk);

    std::vector<Node> GetInternalNodes(Network ntk);

    Node GetNodebyName(Network ntk, std::string name);

    std::string GetNodeName(Node node);

    std::vector<Node> GetFanins(Node node);

    std::vector<Node> GetFanouts(Node node);

    std::vector<Node> GetMFFC(Node node);

}

#endif
