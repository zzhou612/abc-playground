#ifndef STA_H
#define STA_H

#include <string>
#include <map>
#include <ectl_types.h>

struct Path {
    std::vector<Node> nodes;
    explicit Path(std::vector<Node> nodes = std::vector<Node>()) {
        this->nodes = std::move(nodes);
    }
    void Print() const {
        for(auto node : nodes)
            std::cout << ECTL::GetNodeName(node) << " ";
        std::cout << std::endl;
    }
};

std::map<Node, int> CalculateSlack(Network ntk);

void KMostCriticalPaths(Network ntk, int k = 10, bool show_slack = false);

std::vector<Path> GetCriticalPaths(Network ntk);

std::vector<Node> MinCut(const Network ntk, std::map<Node, double> error);

#endif
