#include <random>
#include <chrono>
#include <functional>
#include <bitset>
#include <boost/progress.hpp>
#include <ectl_simulator.h>

namespace ECTL {

    double _SimER(const NetworkPtr &origin_ntk, const NetworkPtr &approx_ntk, bool show_progress_bar, int sim_time) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);

        assert(abc::Abc_NtkIsSopLogic(origin_ntk->_Get_Abc_Ntk()));
        assert(abc::Abc_NtkIsSopLogic(approx_ntk->_Get_Abc_Ntk()));

        int err = 0;
        boost::progress_display *pd = nullptr;
        if (show_progress_bar)
            pd = new boost::progress_display((unsigned long) sim_time);

        for (int _ = 0; _ < sim_time; ++_) {
            if (show_progress_bar)
                ++(*pd);
            auto origin_pis = origin_ntk->GetPrimaryInputs();
            auto approx_pis = approx_ntk->GetPrimaryInputs();

            for (int i = 0; i < (int) origin_pis.size(); i++) {
                int rand_val = dice();
                origin_pis[i]->SetiTemp(rand_val);
                approx_pis[i]->SetiTemp(rand_val);
            }

            for (const auto &obj : TopologicalSort(origin_ntk))
                if (obj->IsNode())
                    obj->_SopSimulate();
            for (const auto &po : origin_ntk->GetPrimaryOutputs())
                po->SetiTemp(po->GetFanin0()->GetiTemp());

            for (const auto &obj : TopologicalSort(approx_ntk))
                if (obj->IsNode())
                    obj->_SopSimulate();
            for (const auto &po : approx_ntk->GetPrimaryOutputs())
                po->SetiTemp(po->GetFanin0()->GetiTemp());

            auto origin_pos = origin_ntk->GetPrimaryOutputs();
            auto approx_pos = approx_ntk->GetPrimaryOutputs();

            for (int i = 0; i < (int) origin_pos.size(); i++) {
                if (origin_pos[i]->GetiTemp() != approx_pos[i]->GetiTemp()) {
                    err++;
                    break;
                }
            }
        }
        return (double) err / (double) sim_time;

    }

    double SimER(const NetworkPtr &origin_ntk, const NetworkPtr &approx_ntk, bool show_progress_bar, int sim_time) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<uint64_t> distribution(0, UINT64_MAX);
        auto dice = std::bind(distribution, generator);

        assert(abc::Abc_NtkIsSopLogic(origin_ntk->_Get_Abc_Ntk()));
        assert(abc::Abc_NtkIsSopLogic(approx_ntk->_Get_Abc_Ntk()));

        int err = 0;
        boost::progress_display *pd = nullptr;
        if (show_progress_bar)
            pd = new boost::progress_display((unsigned long) sim_time / 64);

        for (int _ = 0; _ < sim_time / 64; ++_) {
            if (show_progress_bar)
                ++(*pd);
            auto origin_pis = origin_ntk->GetPrimaryInputs();
            auto approx_pis = approx_ntk->GetPrimaryInputs();

            for (int i = 0; i < (int) origin_pis.size(); i++) {
                uint64_t rand_val = dice();
                origin_pis[i]->SetVal(rand_val);
                approx_pis[i]->SetVal(rand_val);
            }

            for (const auto &obj : TopologicalSort(origin_ntk))
                if (obj->IsNode())
                    obj->Simulate();
            for (const auto &po : origin_ntk->GetPrimaryOutputs())
                po->SetVal(po->GetFanin0()->GetVal());

            for (const auto &obj : TopologicalSort(approx_ntk))
                if (obj->IsNode())
                    obj->Simulate();
            for (const auto &po : approx_ntk->GetPrimaryOutputs())
                po->SetVal(po->GetFanin0()->GetVal());

            auto origin_pos = origin_ntk->GetPrimaryOutputs();
            auto approx_pos = approx_ntk->GetPrimaryOutputs();

            uint64_t res = 0;
            for (int i = 0; i < (int) origin_pos.size(); i++) {
                res = res | (origin_pos[i]->GetVal() ^ approx_pos[i]->GetVal());
            }
            err += std::bitset<64>(res).count();
        }
        return (double) err / (double) sim_time;
    }

    std::unordered_map<ObjectPtr, std::vector<int>> SimTruthVec(const NetworkPtr &ntk, bool show_progress_bar, int sim_time) {
        std::unordered_map<ObjectPtr, std::vector<int>> truth_vec;

        truth_vec.reserve(ntk->GetPIsNodes().size());

        for (const auto &obj : ntk->GetPIsNodes()) {
            truth_vec.emplace(obj, std::vector<int>());
            truth_vec.at(obj).reserve(sim_time);
        }

        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        boost::progress_display *pd = nullptr;
        auto dice = std::bind(distribution, generator);
        assert(abc::Abc_NtkIsSopLogic(ntk->_Get_Abc_Ntk()));
        if (show_progress_bar)
            pd = new boost::progress_display((unsigned long) sim_time);

        for (int _ = 0; _ < sim_time; ++_) {
            if (show_progress_bar)
                ++(*pd);
            auto origin_pis = ntk->GetPrimaryInputs();

            for (const auto &obj : TopologicalSort(ntk)) {
                if (obj->IsPrimaryInput()) {
                    int rand_val = dice();
                    obj->SetiTemp(rand_val);
                    truth_vec[obj].push_back(obj->GetiTemp());
                } else if (obj->IsNode()) {
                    obj->_SopSimulate();
                    truth_vec[obj].push_back(obj->GetiTemp());
                }
            }
        }
        return truth_vec;
    }

    void SimTest(NetworkPtr ntk) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);

        assert(abc::Abc_NtkIsSopLogic(ntk->_Get_Abc_Ntk()));
        for (auto &pi : ntk->GetPrimaryInputs())
            pi->SetiTemp(dice());
        for (auto &obj : TopologicalSort(ntk))
            if (obj->IsNode())
                obj->_SopSimulate();
        for (auto &po : ntk->GetPrimaryOutputs())
            po->SetiTemp(po->GetFanin0()->GetiTemp());

        std::cout << "Performing simulation test (one round) for " << ntk->GetName() << ":\n";
        std::cout << "Primary inputs:\n";
        for (auto &pi : ntk->GetPrimaryInputs())
            std::cout << pi->GetName() << "=" << pi->GetiTemp() << " ";
        std::cout << "\nInternal nodes (including primary output nodes):\n";
        for (auto &node: ntk->GetNodes())
            std::cout << node->GetName() << "=" << node->GetiTemp() << " ";
        std::cout << "\nPrimary outputs:\n";
        for (auto &po : ntk->GetPrimaryOutputs())
            std::cout << po->GetName() << "=" << po->GetiTemp() << " ";
        std::cout << std::endl;
    }
}
