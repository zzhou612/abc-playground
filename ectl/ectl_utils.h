#ifndef ECTL_UTILS_H
#define ECTL_UTILS_H

#include <iostream>
#include <vector>
#include <abc_api.h>
#include <ectl_network.h>

namespace ECTL {
    std::vector<NodePtr> TopologicalSort(NetworkPtr ntk);

//    Node_t CreateConstNode(Network_t ntk, int constant);
//
//    void ReplaceNode(Node_t old_node, Node_t new_node);

    void PrintMFFC(NodePtr node);

    std::vector<NodePtr> GetMFFCNodes(NodePtr node);

    std::vector<NodePtr> GetMFFCInputs(NodePtr node);

    NetworkPtr CreateMFFCNetwork(NodePtr node);
}

#endif
