#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>
#include <boost/filesystem.hpp>
#include <abc_api.h>
#include <ectl.h>

using namespace boost::filesystem;
using namespace ECTL;

int main() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir  = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C17.blif";
    path mffc_path      = benchmark_dir / "mffc.blif";

    auto origin_ntk = std::make_shared<Network>();
    origin_ntk->ReadBlifLogic(benchmark_path.string());
    auto approx_ntk = origin_ntk->Duplicate();

    for (const auto &pi : approx_ntk->GetPrimaryInputs())
        std::cout << pi->GetID() << "-" << pi->GetName() << " ";
    std::cout << std::endl;

    for (const auto &po : approx_ntk->GetPrimaryOutputs())
        std::cout << po->GetID() << "-" << po->GetName() << " ";
    std::cout << std::endl;

    for (const auto &node : approx_ntk->GetNodes())
        std::cout << node->GetID() << "-" << node->GetName() << " ";
    std::cout << std::endl;

    for (const auto &node: TopologicalSort(approx_ntk))
        std::cout << node->GetID() << "-" << node->GetName() << " ";
    std::cout << std::endl;

    auto target_node     = approx_ntk->GetNodebyName("23GAT(9)");
    auto target_node_bak = origin_ntk->GetObjbyID(target_node->GetID());
    auto sub_node        = approx_ntk->GetNodebyName("19GAT(7)");

    auto sub_inv = approx_ntk->CreateInverter(sub_node);

    approx_ntk->ReplaceObj(target_node, sub_inv);

    std::cout << SimErrorRate(origin_ntk, approx_ntk, true);

    approx_ntk->DeleteObj(sub_inv);
    approx_ntk->RecoverObjFrom(target_node_bak);

    std::cout << SimErrorRate(origin_ntk, approx_ntk, true);

    return 0;
}
