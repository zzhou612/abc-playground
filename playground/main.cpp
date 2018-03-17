#include <iostream>
#include <boost/filesystem.hpp>
#include <ectl.h>

using namespace boost::filesystem;
using namespace ECTL;

int main(int argc, char *argv[]) {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path iblif_c880 = benchmark_dir / "C880.blif";
    path iblif_c17 = benchmark_dir / "C17.blif";
    path iblif_c17_approx = benchmark_dir / "C17_approx.blif";
    path oblif = benchmark_dir / "C17_strashed.blif";

    Frame abc = ECTL::StartAbc();

    Network ntk_c880 = ReadBlif(iblif_c880.string());
    for (auto node : GetInternalNodes(ntk_c880)) {
        PrintMFFC(node);
    }

    Node node_n307 = GetNodebyName(ntk_c880, "n307");
    PrintMFFC(node_n307);
    Network ntk_mffc = CreateMFFCNetwork(ntk_c880, node_n307);
    ShowNetworkInfo(ntk_mffc);

    std::cout<< "PIs: ";
    for(auto node:GetPrimaryInputs(ntk_mffc))
        std::cout << GetNodeName(node) << " ";
    std::cout << std::endl << "Internal nodes: ";
    for(auto node:GetInternalNodes(ntk_mffc))
        std::cout << GetNodeName(node) << " ";
    std::cout << std::endl << "POs: ";
    for(auto node:GetPrimaryOutputs(ntk_mffc))
        std::cout << GetNodeName(node) << " ";
    std::cout << std::endl;

    Network ntk_origin = ReadBlif(iblif_c17.string());
    Network ntk_approx = ReadBlif(iblif_c17_approx.string());

    ShowNetworkInfo(ntk_origin);
    SimTest(ntk_origin);

    ShowNetworkInfo(ntk_approx);
    SimTest(ntk_approx);

    std::cout << ECTL::SimError(ntk_origin, ntk_approx) << std::endl;

    DeleteNetwork(ntk_origin);
    DeleteNetwork(ntk_approx);
    DeleteNetwork(ntk_c880);
    StopABC();
    return 0;
}

//    Network ntk_strashed = abc::Abc_NtkStrash (ntk_origin, 1, 1, 0);
//    ECTL::ShowNetworkInfo(ntk_strashed);
//    ECTL::WriteBlif(oblif.string(), ntk_strashed);
//            Abc_NtkToLogic
//            Abc_NtkAigToLogicSop
