#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <abc_api.h>
#include <ectl.h>

using namespace boost::filesystem;
using namespace ECTL;

int main() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir  = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C5315.blif";
    path mffc_path      = benchmark_dir / "mffc.blif";

    auto ntk = std::make_shared<Network>();
    ntk->ReadBlif(benchmark_path.string());
    auto ntk_copied = ntk->Duplicate();

//    for (auto &internal_node : ntk->GetInternalNodes()) {
//        std::cout << internal_node->GetName() << " ";
//    }
//
//    std::cout << std::endl;
//
//    for (auto &node : TopologicalSort(ntk)) {
//        std::cout << node->GetName() << " ";
//    }
//
//    std::cout << std::endl;
//
//    SimTest(ntk_copied);
    std::cout << SimErrorRate(ntk, ntk_copied, true);

    return 0;
}