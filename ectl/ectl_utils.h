#ifndef ECTL_UTILS_H
#define ECTL_UTILS_H

#include <iostream>
#include <vector>
#include <abc_api.h>
#include <ectl_network.h>

namespace ECTL {
    std::vector<ObjectPtr> TopologicalSort(NetworkPtr ntk);

//    Node_t CreateConstNode(Network_t ntk, int constant);
//
//    void ReplaceNode(Node_t old_node, Node_t new_node);

    void PrintMFFC(ObjectPtr node);

    std::vector<ObjectPtr> GetMFFCNodes(ObjectPtr node);

    std::vector<ObjectPtr> GetMFFCInputs(ObjectPtr node);

    NetworkPtr CreateMFFCNetwork(ObjectPtr node);
}

#endif
