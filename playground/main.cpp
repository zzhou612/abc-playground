#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <ectl.h>
#include "sta.h"

using namespace boost::filesystem;
using namespace ECTL;
using namespace std;

static const double INF_PROB = 100000;
static const int INF = 1000000;

static std::vector<Node> GetSortedNodes(Network ntk) {
    std::vector<Node> sorted_internal_nodes = TopologicalSort(ntk);
    std::vector<Node> sorted_nodes = GetPrimaryInputs(ntk);
    sorted_nodes.insert(sorted_nodes.end(),
                        std::make_move_iterator(sorted_internal_nodes.begin()),
                        std::make_move_iterator(sorted_internal_nodes.end()));
    return sorted_nodes;
}

std::tuple<std::map<Node, double>, std::map<Node, int> > GetNodeError(Network ntk_origin,
                                                                      Network ntk_approx,
                                                                      int simu_time = 10000,
                                                                      bool print_info = false) {
    std::map<Node, int> slack = CalculateSlack(ntk_approx);
    std::map<Node, double> node_error;
    std::map<Node, int> approx_constant;
    auto sorted_nodes = GetSortedNodes(ntk_approx);
    boost::progress_display pd(sorted_nodes.size());

    for (auto node : sorted_nodes) {
        ++pd;
        if (slack[node] != 0)
            continue;
        if (abc::Abc_NodeIsConst(node) || IsPrimaryInput(node) || IsPrimaryOutputNode(node)) {
            node_error[node] = INF_PROB;
            approx_constant[node] = 0;
        } else {
            Network t_ntk_approx = DuplicateNetwork(ntk_approx);
            Node node_approx = GetNodebyID(t_ntk_approx, GetNodeID(node));
            Node const_0 = CreateConstNode(t_ntk_approx, 0);
            ReplaceNode(node_approx, const_0);
            double error_0 = SimError(ntk_origin, t_ntk_approx, false, simu_time);
            DeleteNetwork(t_ntk_approx);

            t_ntk_approx = DuplicateNetwork(ntk_approx);
            node_approx = GetNodebyID(t_ntk_approx, GetNodeID(node));
            Node const_1 = CreateConstNode(t_ntk_approx, 1);
            ReplaceNode(node_approx, const_1);
            double error_1 = SimError(ntk_origin, t_ntk_approx, false, simu_time);
            DeleteNetwork(t_ntk_approx);

            node_error[node] = std::max(std::min(error_0, error_1), 0.00000001);

            if (error_0 < error_1) {
                if (print_info) std::cout << GetNodeName(node) << " ---> 0 \t:\t" << node_error[node] << std::endl;
                approx_constant[node] = 0;
            } else {
                if (print_info) std::cout << GetNodeName(node) << " ---> 1 \t:\t" << node_error[node] << std::endl;
                approx_constant[node] = 1;
            }
        }
    }

    return std::make_tuple(node_error, approx_constant);
}

void ALS() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path result_dir = project_source_dir / "result";

    Frame abc = StartAbc();

    path origin_blif = benchmark_dir / "C1908.blif";
    path approx_blif = result_dir / "C1908.blif";

    Network ntk_origin = ReadBlif(origin_blif.string());
    Network ntk_approx = DuplicateNetwork(ntk_origin);

    cout << "Critical paths of original circuit: " << endl;
    KMostCriticalPaths(ntk_origin, 1);

    double error = 0;
    int round = 1;
    while (error < 0.2) {
        map<Node, double> node_error;
        map<Node, int> approx_constant;
        tie(node_error, approx_constant) = GetNodeError(ntk_origin, ntk_approx, 1000, false);

        vector<Node> min_cut = MinCut(ntk_approx, node_error);
        cout << "Min Cut: ";
        for (auto node : min_cut)
            cout << GetNodeName(node) << "-" << node_error[node] << "-" << approx_constant[node] << " ";
        cout << endl << "MFFC:" << endl;
        for (auto node : min_cut)
            PrintMFFC(node);
        for (auto node : min_cut)
            ReplaceNode(node, CreateConstNode(ntk_approx, approx_constant[node]));

        error = SimError(ntk_origin, ntk_approx, true);
        KMostCriticalPaths(ntk_approx, 1);
        cout << "Error: " << error << endl;
        cout << endl << endl;

        path o_blif = approx_blif.parent_path() /
                      path(approx_blif.stem().string() + "_" + to_string(round) + approx_blif.extension().string());
//        ntk_approx = abc::Abc_NtkStrash(ntk_approx, 1, 1, 0);
        WriteBlif(ntk_approx, o_blif.string());
        DeleteNetwork(ntk_approx);
        ntk_approx = ReadBlif(o_blif.string());

        round++;
    }

    DeleteNetwork(ntk_origin);
    DeleteNetwork(ntk_approx);

    StopABC();
}

void Playground() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path result_dir = project_source_dir / "result";

    Frame abc = StartAbc();
    path i_blif = benchmark_dir / "C6288.blif";
    path o_blif = benchmark_dir / "C6288.blif";

    Network ntk = ReadBlif(i_blif.string());
    Network ntk_approx = ReadBlif(o_blif.string());

    std::cout << SimError(ntk, ntk_approx, true);

    DeleteNetwork(ntk);
    DeleteNetwork(ntk_approx);

//    Node n149 = GetInternalNodebyName(ntk, "n149");
//    PrintMFFC(n149);
//    Network mffc = CreateMFFCNetwork(n149);
//    WriteBlif(mffc, o_blif.string());
//
//    std::map<Node, double> zero_prob = SimZeroProb(ntk);
//    for (auto node : GetMFFCNodes(n149))
//        std::cout << GetNodeName(node) << "-" << zero_prob[node] << " ";
//    std::cout << std::endl;
//    for (auto node : GetMFFCInputs(n149))
//        std::cout << GetNodeName(node) << "-" << zero_prob[node] << " ";
//    std::cout << std::endl;
//    DeleteNetwork(ntk);
//    DeleteNetwork(mffc);
    StopABC();
}

int main(int argc, char *argv[]) {
    ALS();
//    Playground();
    return 0;
}
