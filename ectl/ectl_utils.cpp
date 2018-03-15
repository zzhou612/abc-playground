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

    void WriteBlif(Network ntk, std::string ofile) {
        abc::Io_WriteBlifLogic(ntk, (char *) ofile.c_str(), 0);
    }

    void ShowNetworkInfo(Network ntk) {
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

    std::vector<Node> TopologicalSort(Network ntk) {
        abc::Vec_Ptr_t *nodes = abc::Abc_NtkDfs(ntk, 0);
        std::vector<Node> sorted_nodes;
        for (int i = 0; i < nodes->nSize; ++i) {
            auto obj = (Node) nodes->pArray[i];
            sorted_nodes.emplace_back(obj);
        }
        Vec_PtrFree(nodes);
        return sorted_nodes;
    }

    std::vector<Node> GetPrimaryInputs(Network ntk) {
        std::vector<Node> pis;
        Node pi;
        int i;
        Abc_NtkForEachPi(ntk, pi, i) {
            pis.emplace_back(pi);
        }
        return pis;
    }

    std::vector<Node> GetPrimaryOutputs(Network ntk) {
        std::vector<Node> pos;
        Node po;
        int i;
        Abc_NtkForEachPo(ntk, po, i) {
            pos.emplace_back(po);
        }
        return pos;
    }

    Node GetNodebyName(Network ntk, std::string name) {
        return abc::Abc_NtkFindNode(ntk, (char *) name.c_str());
    }

    std::vector<Node> GetInternalNodes(Network ntk) {
        std::vector<Node> nodes;
        Node node;
        int i;
        Abc_NtkForEachNode(ntk, node, i) {
                nodes.emplace_back(node);
            }
        return nodes;
    }

    std::string GetNodeName(Node node) {
        return std::string(Abc_ObjName(node));
    }

    std::vector<Node> GetFanins(Node node) {
        std::vector<Node> fan_ins;
        Node fan_in;
        int i;
        Abc_ObjForEachFanin(node, fan_in, i) {
            fan_ins.emplace_back(fan_in);
        }
        return fan_ins;
    }

    std::vector<Node> GetFanouts(Node node) {
        std::vector<Node> fan_outs;
        Node fan_out;
        int i;
        Abc_ObjForEachFanout(node, fan_out, i) {
            fan_outs.emplace_back(fan_out);
        }
        return fan_outs;
    }

    std::vector<Node> GetMFFC(Node node) {
        abc::Vec_Ptr_t *vCone, *vSupp;
        vCone = abc::Vec_PtrAlloc(100);
        vSupp = abc::Vec_PtrAlloc(100);
        abc::Abc_NodeDeref_rec(node);
        Abc_NodeMffcConeSupp(node, vCone, vSupp);
        abc::Abc_NodeRef_rec(node);

        std::vector<Node> mffc;
        Node t;
        int i;
        Vec_PtrForEachEntry(Node, vCone, t, i) {
            mffc.emplace_back(t);
        }
        return mffc;
    }

}
