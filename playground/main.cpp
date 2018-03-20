#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <ectl.h>
#include "sta.h"

using namespace boost::filesystem;
using namespace ECTL;
using namespace std;

static const double INF_PROB = 100000;

static std::vector<Node> GetSortedNodes(Network ntk) {
    std::vector<Node> sorted_internal_nodes = TopologicalSort(ntk);
    std::vector<Node> sorted_nodes = GetPrimaryInputs(ntk);
    sorted_nodes.insert(sorted_nodes.end(),
                        std::make_move_iterator(sorted_internal_nodes.begin()),
                        std::make_move_iterator(sorted_internal_nodes.end()));
    return sorted_nodes;
}

std::tuple<std::map<Node, double>, std::map<Node, int> > GetNodeError(Network ntk) {
    std::map<Node, double> node_error;
    std::map<Node, int> approx_constant;

    for (auto node : GetSortedNodes(ntk))
        if (IsPrimaryInput(node) || IsPrimaryOutput(GetFanout0(node))) {
            node_error[node] = INF_PROB;
            approx_constant[node] = 0;
        } else {
            Network ntk_approx = DuplicateNetwork(ntk);
            Node node_approx = GetNodebyID(ntk_approx, GetNodeID(node));
            Node const_0 = CreateConstNode(ntk_approx, 0);
            Node const_1 = CreateConstNode(ntk_approx, 1);
            ReplaceNode(node_approx, const_0);
            double error_0 = SimError(ntk, ntk_approx, 10000);
            ReplaceNode(const_0, const_1);
            double error_1 = SimError(ntk, ntk_approx, 10000);
            DeleteNetwork(ntk_approx);

            node_error[node] = std::min(error_0, error_1);
            if (error_0 < error_1) {
                std::cout << GetNodeName(node) << " ---> 0 \t:\t" << error_0 << std::endl;
                approx_constant[node] = 0;
            } else {
                std::cout << GetNodeName(node) << " ---> 1 \t:\t" << error_1 << std::endl;
                approx_constant[node] = 1;
            }
        }

    return std::make_tuple(node_error, approx_constant);
}

int main(int argc, char *argv[]) {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path c17 = benchmark_dir / "C17.blif";
    path c880 = benchmark_dir / "C880.blif";
    path c1908 = benchmark_dir / "C1908.blif";

    Frame abc = StartAbc();

    Network ntk = ReadBlif(c1908.string());

    Network ntk_origin = DuplicateNetwork(ntk);

    Network ntk_approx = DuplicateNetwork(ntk);

    vector<Node> sorted_nodes = GetSortedNodes(ntk);
    map<Node, double> node_error;
    map<Node, int> approx_constant;

    tie(node_error, approx_constant) = GetNodeError(ntk);
    vector<Node> min_cut = MinCut(ntk, node_error);

    cout << "Min Cut: ";
    for (auto node : min_cut) {
        cout << GetNodeName(node) << "-" << node_error[node] << "-" << approx_constant[node] << " ";
        Node node_approx = GetNodebyID(ntk_approx, GetNodeID(node));
        ReplaceNode(node_approx, CreateConstNode(ntk_approx, approx_constant[node]));
    }
    cout << endl << "Error: " << SimError(ntk, ntk_approx) << endl;
    cout << "MFFC:" << endl;
    for( auto node : min_cut)
        PrintMFFC(node);

    // C1908
//    KMostCriticalPaths(ntk);
//    std::vector<std::string> const_0_nodes{"n312", "n346", "n377", "n385", "n390", "n398"};
//    for (const auto &node_name : const_0_nodes) {
//        Node node_approx = GetNodebyName(ntk_approx, node_name);
//        ReplaceNode(node_approx, CreateConstNode(ntk_approx, 0));
//    }
//    cout << SimError(ntk, ntk_approx) << endl;
//    KMostCriticalPaths(ntk_approx);


    // C880
//    KMostCriticalPaths(ntk);
//    std::vector<std::string> const_0_nodes{"n140"};
//    for (const auto &node_name : const_0_nodes) {
//        Node node_approx = GetNodebyName(ntk_approx, node_name);
//        ReplaceNode(node_approx, CreateConstNode(ntk_approx, 0));
//    }
//    cout << SimError(ntk, ntk_approx) << endl;
//    KMostCriticalPaths(ntk_approx);

    DeleteNetwork(ntk);
    DeleteNetwork(ntk_origin);
    DeleteNetwork(ntk_approx);

    StopABC();
    return 0;
}

//    Network ntk_strashed = abc::Abc_NtkStrash (ntk_origin, 1, 1, 0);
//    ECTL::ShowNetworkInfo(ntk_strashed);
//    ECTL::WriteBlif(oblif.string(), ntk_strashed);
//            Abc_NtkToLogic
//            Abc_NtkAigToLogicSop
