#include <random>
#include <chrono>
#include <functional>

#include <ectl_simulator.h>

namespace abc {
    int Abc_ObjSopSimulate(Abc_Obj_t *pObj);
}

namespace ECTL {

    double SimError(Network origin_ntk, Network approx_ntk, int simu_time) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);
        assert(Abc_NtkIsSopLogic(origin_ntk));
        assert(Abc_NtkIsSopLogic(approx_ntk));

        int err = 0;
        for (int _ = 0; _ < simu_time; ++_) {
            std::vector<Object> origin_pis = GetPrimaryInputs(origin_ntk);
            std::vector<Object> approx_pis = GetPrimaryInputs(approx_ntk);

            for (int i = 0; i < (int) origin_pis.size(); i++)
                origin_pis[i]->iTemp = approx_pis[i]->iTemp = dice();

            for (auto node : TopologicalSort(origin_ntk))
                node->iTemp = abc::Abc_ObjSopSimulate(node);
            for (auto po : GetPrimaryOutputs(origin_ntk))
                po->iTemp = Abc_ObjFanin0(po)->iTemp;

            for (auto node : TopologicalSort(approx_ntk))
                node->iTemp = abc::Abc_ObjSopSimulate(node);
            for (auto po : GetPrimaryOutputs(approx_ntk))
                po->iTemp = Abc_ObjFanin0(po)->iTemp;

            std::vector<Object> origin_pos = GetPrimaryOutputs(origin_ntk);
            std::vector<Object> approx_pos = GetPrimaryOutputs(approx_ntk);
            for (int i = 0; i < (int) origin_pos.size(); i++) {
                if (origin_pos[i]->iTemp != approx_pos[i]->iTemp) {
                    err++;
                    break;
                }
            }
        }
        return (double) err / (double) simu_time;

    }

    void SimTest(Network ntk) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);

        assert(Abc_NtkIsSopLogic(ntk));
        for (auto pi : GetPrimaryInputs(ntk))
            pi->iTemp = dice();
        for (auto node : TopologicalSort(ntk))
            node->iTemp = abc::Abc_ObjSopSimulate(node);
        for (auto po : GetPrimaryOutputs(ntk))
            po->iTemp = Abc_ObjFanin0(po)->iTemp;

        std::cout << "Performing simulation test (one round) for " << GetNetworkName(ntk) << ":\n";
        std::cout << "Primary inputs:\n";
        for (auto pi : GetPrimaryInputs(ntk))
            std::cout << GetNodeName(pi) << "=" << pi->iTemp << " ";
        std::cout << "\nInternal nodes (including primary outputs):\n";
        for (auto node: GetInternalNodes(ntk))
            std::cout << GetNodeName(node) << "=" << node->iTemp << " ";
        std::cout << "\nPrimary outputs:\n";
        for (auto po : GetPrimaryOutputs(ntk))
            std::cout << GetNodeName(po) << "=" << po->iTemp << " ";
        std::cout << std::endl;
    }
}
