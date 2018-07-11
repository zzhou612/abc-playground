#include <random>
#include <chrono>
#include <functional>
#include <boost/progress.hpp>

#include <ectl_simulator.h>

namespace abc {
    int Abc_ObjSopSimulate(Abc_Obj_t *pObj);
}

namespace ECTL {

//    double SimErrorRate(Network_t origin_ntk, Network_t approx_ntk, bool show_progress_bar, int simu_time) {
//        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
//        std::uniform_int_distribution<int> distribution(0, 1);
//        auto dice = std::bind(distribution, generator);
//        assert(abc::Abc_NtkIsSopLogic(origin_ntk));
//        assert(abc::Abc_NtkIsSopLogic(approx_ntk));
//
//        int err = 0;
//        boost::progress_display *pd = nullptr;
//        if (show_progress_bar)
//            pd = new boost::progress_display((unsigned long) simu_time);
//
//        for (int _ = 0; _ < simu_time; ++_) {
//            if (show_progress_bar)
//                ++(*pd);
//            std::vector<Node_t> origin_pis = GetPrimaryInputs(origin_ntk);
//            std::vector<Node_t> approx_pis = GetPrimaryInputs(approx_ntk);
//
//            for (int i = 0; i < (int) origin_pis.size(); i++)
//                origin_pis[i]->iTemp = approx_pis[i]->iTemp = dice();
//
//            for (auto node : TopologicalSort(origin_ntk))
//                node->iTemp = SopSimulate(node);
//            for (auto po : GetPrimaryOutputs(origin_ntk))
//                po->iTemp = GetFanin0(po)->iTemp;
//
//            for (auto node : TopologicalSort(approx_ntk))
//                node->iTemp = SopSimulate(node);
//            for (auto po : GetPrimaryOutputs(approx_ntk))
//                po->iTemp = GetFanin0(po)->iTemp;
//
//            std::vector<Node_t> origin_pos = GetPrimaryOutputs(origin_ntk);
//            std::vector<Node_t> approx_pos = GetPrimaryOutputs(approx_ntk);
//            for (int i = 0; i < (int) origin_pos.size(); i++) {
//                if (origin_pos[i]->iTemp != approx_pos[i]->iTemp) {
//                    err++;
//                    break;
//                }
//            }
//        }
//        return (double) err / (double) simu_time;
//
//    }

    void SimTest(NetworkPtr ntk) {
        std::default_random_engine         generator(
                (unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto                               dice = std::bind(distribution, generator);

        assert(abc::Abc_NtkIsSopLogic(ntk->_Get_Abc_Ntk()));
        for (auto &pi : ntk->GetPrimaryInputs())
            pi->_Get_Abc_Node()->iTemp   = dice();
        for (auto &node : TopologicalSort(ntk))
            node->_Get_Abc_Node()->iTemp = node->SopSimulate();
        for (auto &po : ntk->GetPrimaryOutputs())
            po->_Get_Abc_Node()->iTemp   = po->GetFanin0()->_Get_Abc_Node()->iTemp;

        std::cout << "Performing simulation test (one round) for " << ntk->GetName() << ":\n";
        std::cout << "Primary inputs:\n";
        for (auto &pi : ntk->GetPrimaryInputs())
            std::cout << pi->GetName() << "=" << pi->_Get_Abc_Node()->iTemp << " ";
        std::cout << "\nInternal nodes (including primary outputs):\n";
        for (auto &node: ntk->GetInternalNodes())
            std::cout << node->GetName() << "=" << node->_Get_Abc_Node()->iTemp << " ";
        std::cout << "\nPrimary outputs:\n";
        for (auto &po : ntk->GetPrimaryOutputs())
            std::cout << po->GetName() << "=" << po->_Get_Abc_Node()->iTemp << " ";
        std::cout << std::endl;
    }
}
