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

    auto ntk = std::make_shared<Network>();
    ntk->ReadBlifLogic(benchmark_path.string());
    auto ntk_copied = ntk->Duplicate();

    for (const auto &pi : ntk->GetPrimaryInputs())
        std::cout << pi->GetID() << "-" << pi->GetName() << " ";
    std::cout << std::endl;

    for (const auto &po : ntk->GetPrimaryOutputs())
        std::cout << po->GetID() << "-" << po->GetName() << " ";
    std::cout << std::endl;

    for (const auto &node : ntk->GetNodes())
        std::cout << node->GetID() << "-" << node->GetName() << " ";
    std::cout << std::endl;

    for (const auto &node: TopologicalSort(ntk))
        std::cout << node->GetID() << "-" << node->GetName() << " ";
    std::cout << std::endl;

    std::cout << SimErrorRate(ntk, ntk_copied, true);

    return 0;
}
