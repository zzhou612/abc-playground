#ifndef ECTL_UTILS_H
#define ECTL_UTILS_H

#include <iostream>
#include <vector>
#include <ectl_types.h>

namespace ECTL {
    Frame StartAbc();

    void StopABC();

    Network DuplicateNetwork(Network ntk);

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

    Node GetNodebyID(Network ntk, int id);

    int GetNodeID(Node node);

    Network GetHostNetwork(Node node);

    std::string GetNodeName(Node node);

    std::vector<Node> GetFanins(Node node);

    Node GetFanin0(Node node);

    Node GetFanin1(Node node);

    std::vector<Node> GetFanouts(Node node);

    Node GetFanout0(Node node);

    bool IsPrimaryInput(Node node);

    bool IsPrimaryOutput(Node node);

    bool IsPrimaryOutputNode(Node node);

    bool IsNode(Node node);

    Node CreateConstNode(Network ntk, int constant);

    void ReplaceNode(Node old_node, Node new_node);

    int SopSimulate(Node node);

    void PrintMFFC(Node node);

    std::vector<Node> GetMFFCNodes(Node node);

    Network CreateMFFCNetwork(Node node);

}

#endif
