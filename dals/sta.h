#ifndef DALS_STA_H
#define DALS_STA_H

#include <map>
#include <unordered_map>
#include <ectl_network.h>
#include <ectl_utils.h>

using namespace ECTL;

struct TimeObject {
    int arrival_time;
    int required_time;
    int slack;

    TimeObject(int arrival, int required, int slack) : arrival_time(arrival), required_time(required), slack(slack) {}
};

struct Path {
    std::vector<ObjectPtr> nodes;

    explicit Path(std::vector<ObjectPtr> nodes = std::vector<ObjectPtr>()) {
        this->nodes = std::move(nodes);
    }

    void Print() const {
        for (const auto &node : nodes)
            std::cout << node->GetName() << " ";
        std::cout << std::endl;
    }
};

std::unordered_map<ObjectPtr, TimeObject> CalculateSlack(const NetworkPtr &ntk, bool print_result = false);

int PrintKMostCriticalPaths(const NetworkPtr &ntk, int k = 10, bool show_slack = false);

std::vector<Path> GetKMostCriticalPaths(const NetworkPtr &ntk, int k = -1);

std::vector<ObjectPtr> MinCut(const NetworkPtr &ntk, const std::unordered_map<ObjectPtr, double> &node_error);


#endif
