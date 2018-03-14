#include <iostream>
#include <boost/filesystem.hpp>
#include <ectl.h>

using namespace boost::filesystem;

int main(int argc, char *argv[]) {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path iblif_origin = benchmark_dir / "C17.blif";
    path iblif_approx = benchmark_dir / "C17.blif.approx";

    Frame abc = ECTL::StartAbc();
    Network ntk_origin = ECTL::ReadBlif(iblif_origin.string());
    Network ntk_approx = ECTL::ReadBlif(iblif_approx.string());

    ECTL::ShowNetworkInfo(ntk_origin);
    ECTL::SimTest(ntk_origin);

    ECTL::ShowNetworkInfo(ntk_approx);
    ECTL::SimTest(ntk_approx);

    std::cout << ECTL::SimError(ntk_origin, ntk_approx) << std::endl;

    ECTL::DeleteNetwork(ntk_origin);
    ECTL::StopABC();
    return 0;
}
