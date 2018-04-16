#include <iostream>
#include <cmath>
#include <chrono>
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
                                                                      int mode = 0, // 0: error rate, 1: mean relative error distance
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
            double error_0 = 0;
            if (mode == 0)
                error_0 = SimErrorRate(ntk_origin, t_ntk_approx, false, simu_time);
            else
                error_0 = SimMeanRelativeErrorDistance(t_ntk_approx, false, simu_time);
            DeleteNetwork(t_ntk_approx);

            t_ntk_approx = DuplicateNetwork(ntk_approx);
            node_approx = GetNodebyID(t_ntk_approx, GetNodeID(node));
            Node const_1 = CreateConstNode(t_ntk_approx, 1);
            ReplaceNode(node_approx, const_1);
            double error_1 = 0;
            if (mode == 0)
                error_1 = SimErrorRate(ntk_origin, t_ntk_approx, false, simu_time);
            else
                error_1 = SimMeanRelativeErrorDistance(t_ntk_approx, false, simu_time);

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

void ALS(string file_name, int type, double constraint) {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path result_dir = project_source_dir / "result";

    Frame abc = StartAbc();

    path origin_blif, approx_blif;
    if (type == 0) {
        origin_blif = benchmark_dir / "iscas-85" / file_name;
        approx_blif = result_dir / "iscas-85" / file_name;
    } else if (type == 1) {
        origin_blif = benchmark_dir / "approximate-adders" / file_name;
        approx_blif = result_dir / "approximate-adders" / file_name;
    } else
        return;

    Network ntk_origin = ReadBlif(origin_blif.string());
    Network ntk_approx = DuplicateNetwork(ntk_origin);

    cout << "Critical paths of original circuit: " << endl;
    KMostCriticalPaths(ntk_origin, 1);

    double error = 0;
    int round = 1;
    while (error < constraint) {
        map<Node, double> node_error;
        map<Node, int> approx_constant;
        tie(node_error, approx_constant) = GetNodeError(ntk_origin, ntk_approx, type, 1000, false);

        vector<Node> min_cut = MinCut_0(ntk_approx, node_error);
        cout << "Min Cut: ";
        for (auto node : min_cut)
            cout << GetNodeName(node) << "-" << node_error[node] << "-" << approx_constant[node] << " ";
        cout << endl << "MFFC:" << endl;
        for (auto node : min_cut)
            PrintMFFC(node);
        for (auto node : min_cut)
            ReplaceNode(node, CreateConstNode(ntk_approx, approx_constant[node]));

        double error_rate = SimErrorRate(ntk_origin, ntk_approx, true), mred = 0;
        if (type == 1)
            mred = SimMeanRelativeErrorDistance(ntk_approx);
        int delay = KMostCriticalPaths(ntk_approx, 1);
        cout << "Error rate: " << error_rate << endl;
        if (type == 1)
            cout << "Mean relative error distance: " << mred << endl;
        cout << endl << endl;

        if (type == 0)
            error = error_rate;
        else
            error = mred;
        path o_blif = approx_blif.parent_path() /
                      path(approx_blif.stem().string()
                           + "_" + to_string(round)
                           + "_" + to_string(delay)
                           + "_" + to_string(error)
                           + approx_blif.extension().string());
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
    path blif = benchmark_dir / "approximate-adders" / "RCA_N8.blif";
    path blif_approx = benchmark_dir / "approximate-adders" / "GeAr_N8_R2_P2.blif";

    Network ntk = ReadBlif(blif.string());
    Network ntk_approx = ReadBlif(blif_approx.string());

    cout << SimMeanRelativeErrorDistance(ntk_approx) << " " << SimErrorRate(ntk, ntk_approx);

    DeleteNetwork(ntk);
    DeleteNetwork(ntk_approx);
    StopABC();
}

int main(int argc, char *argv[]) {
//    ALS("RCA_N8.blif", 1, 0.1);
    Playground();
    return 0;
}
