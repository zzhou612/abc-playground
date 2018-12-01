#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <unordered_set>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <abc_api.h>
#include <ectl.h>
#include "sasimi.h"
#include "sta.h"

using namespace boost::filesystem;
using namespace ECTL;

int main() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C880.blif";
    path test_path = benchmark_dir / "out.blif";

    boost::timer::cpu_timer timer;

    auto ntk_origin = std::make_shared<Network>();
    ntk_origin->ReadBlifLogic(benchmark_path.string());
    auto ntk_approx = ntk_origin->Duplicate();

    double error_rate = 0;

    while (error_rate < 0.15) {
        auto time_objs = CalculateSlack(ntk_approx);
        std::vector<ObjPtr> critical_nodes;
        for (const auto &node : TopologicalSort(ntk_approx))
            if (time_objs.at(node).slack == 0 && node->IsNode())
                critical_nodes.push_back(node);
        SASIMI sasimi;
        sasimi.LoadNetwork(ntk_approx);
//        sasimi.GenerateTruthVector();
        auto cands = sasimi.GetBestCands(critical_nodes, true, true);
        std::unordered_map<ObjPtr, double> node_error;
        for (const auto &cand : cands)
            node_error.emplace(cand.GetTarget(), cand.GetError());
        auto min_cut = MinCut(ntk_approx, node_error);
        PrintKMostCriticalPaths(ntk_approx, 1);
        std::cout << "Min Cut: ";
        for (const auto &node : min_cut) {
            std::cout << node->GetName() << " ";
            for (auto &cand : cands) {
                if (cand.GetTarget() == node)
                    cand.Do();
            }
        }
        std::cout << std::endl;
        PrintKMostCriticalPaths(ntk_approx, 1);
        error_rate = SimER(ntk_origin, ntk_approx);
        std::cout << "Error Rate: " << error_rate << std::endl;
        std::cout << std::endl << std::endl << std::endl;
    }

    std::cout << timer.format() << std::endl;

    return 0;
}
