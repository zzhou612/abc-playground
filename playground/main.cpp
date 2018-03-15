#include <iostream>
#include <boost/filesystem.hpp>
#include <ectl.h>

using namespace boost::filesystem;
using namespace ECTL;

int main(int argc, char *argv[]) {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path iblif_origin = benchmark_dir / "C17.blif";
    path iblif_approx = benchmark_dir / "C17_approx.blif";
    path oblif = benchmark_dir / "C17_strashed.blif";

    Frame abc = ECTL::StartAbc();
    Network ntk_origin = ReadBlif(iblif_origin.string());
    Network ntk_approx = ReadBlif(iblif_approx.string());

    for (auto node : GetInternalNodes(ntk_origin)) {
        std::vector<Node> mffc = GetMFFC(node);
        if(mffc.size() > 1) {
            std::cout << "MFFC of " << GetNodeName(node) << ": ";
            for (auto t : mffc)
                std::cout << GetNodeName(t) << " ";
            std::cout << "\n";
        }
    }

//    Network ntk_strashed = abc::Abc_NtkStrash (ntk_origin, 1, 1, 0);
//    ECTL::ShowNetworkInfo(ntk_strashed);
//    ECTL::WriteBlif(oblif.string(), ntk_strashed);
//            Abc_NtkToLogic
//            Abc_NtkAigToLogicSop

    ShowNetworkInfo(ntk_origin);
    SimTest(ntk_origin);

    ShowNetworkInfo(ntk_approx);
    SimTest(ntk_approx);

    std::cout << ECTL::SimError(ntk_origin, ntk_approx) << std::endl;

    DeleteNetwork(ntk_origin);
    StopABC();
    return 0;
}
