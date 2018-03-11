#include <ectl_utils.h>

namespace ECTL {
    Frame StartAbc() {
        abc::Abc_Start();
        Frame frame = abc::Abc_FrameGetGlobalFrame();
        if (!frame) {
            abc::Abc_Stop();
            std::cout << "[e] Error: Not able to invoke the ABC frame" << std::endl;
            std::exit(-1);
        }
        return frame;
    }

    void StopABC() {
        abc::Abc_Stop();
    }

    Network ReadBlif(std::string ifile) {
        Network ntk = abc::Io_ReadBlif((char *) ifile.c_str(), 1);
        ntk = abc::Abc_NtkToLogic(ntk); // Convert to Logic form.
        return ntk;
    }

    void ShowNetworkInfo(Network ntk) {
        std::cout << "***************************************\n";
        std::cout << "Network Name: " << GetNetworkName(ntk) << std::endl;

        std::cout << "Network Type: ";
        if (abc::Abc_NtkIsNetlist(ntk))
            std::cout << "Netlist\n";
        else if (abc::Abc_NtkIsLogic(ntk))
            std::cout << "Logic\n";
        else if (abc::Abc_NtkIsStrash(ntk))
            std::cout << "Strash\n";
        else
            std::cout << "Others\n";

        std::cout << "Network Function: ";
        if (abc::Abc_NtkHasSop(ntk))
            std::cout << "SOP\n";
        else if (abc::Abc_NtkHasBdd(ntk))
            std::cout << "BDD\n";
        else if (abc::Abc_NtkHasAig(ntk))
            std::cout << "AIG\n";
        else if (abc::Abc_NtkHasMapping(ntk))
            std::cout << "Map\n";
        else
            std::cout << "Others\n";
        std::cout << "***************************************\n";
    }

    std::string GetNetworkName(Network ntk) {
        return std::string(ntk->pName);
    }

    void SetNetworkName(Network ntk, const std::string &new_name) {
        ntk->pName = abc::Extra_UtilStrsav((char *) new_name.c_str());
    }

    void DeleteNetwork(Network ntk) {
        abc::Abc_NtkDelete(ntk);
    }
}


