#include <iostream>
#include <boost/filesystem.hpp>
#include <ectl.h>

using namespace boost::filesystem;

int main(int argc, char *argv[]) {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path iblif = benchmark_dir / "C17.blif";

    Frame abc;
    Network ntk;

    abc = ECTL::StartAbc();
    ntk = ECTL::ReadBlif(iblif.string());

    ECTL::ShowNetworkInfo(ntk);

    ECTL::ComSimSopOnce(ntk);
    ECTL::PrintSimRes(ntk);

    ECTL::DeleteNetwork(ntk);
    ECTL::StopABC();
    return 0;
}

