#ifndef ECTL_UTILS_H
#define ECTL_UTILS_H

#include <iostream>
#include <vector>
#include <abc_api.h>
#include <ectl_network.h>

namespace ECTL {
    std::vector<ObjPtr> TopologicalSort(const NtkPtr &ntk);

//    Node_t CreateConstNode(Network_t ntk, int constant);
//
//    void ReplaceNode(Node_t old_node, Node_t new_node);

    void PrintMFFC(ObjPtr node);

    std::vector<ObjPtr> GetMFFCNodes(const ObjPtr &node);

    std::vector<ObjPtr> GetMFFCInputs(const ObjPtr &node);

    NtkPtr CreateMFFCNetwork(const ObjPtr &node);
}

#endif
