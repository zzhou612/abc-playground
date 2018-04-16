#include <random>
#include <chrono>
#include <functional>
#include <boost/progress.hpp>

#include <ectl_simulator.h>

namespace abc {
    int Abc_ObjSopSimulate(Abc_Obj_t *pObj);
}

namespace ECTL {

    double SimErrorRate(Network origin_ntk, Network approx_ntk, bool show_progress_bar, int simu_time) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);
        assert(abc::Abc_NtkIsSopLogic(origin_ntk));
        assert(abc::Abc_NtkIsSopLogic(approx_ntk));

        int err = 0;
        boost::progress_display *pd = nullptr;
        if (show_progress_bar)
            pd = new boost::progress_display((unsigned long) simu_time);

        for (int _ = 0; _ < simu_time; ++_) {
            if (show_progress_bar)
                ++(*pd);
            std::vector<Node> origin_pis = GetPrimaryInputs(origin_ntk);
            std::vector<Node> approx_pis = GetPrimaryInputs(approx_ntk);

            for (int i = 0; i < (int) origin_pis.size(); i++)
                origin_pis[i]->iTemp = approx_pis[i]->iTemp = dice();

            for (auto node : TopologicalSort(origin_ntk))
                node->iTemp = SopSimulate(node);
            for (auto po : GetPrimaryOutputs(origin_ntk))
                po->iTemp = GetFanin0(po)->iTemp;

            for (auto node : TopologicalSort(approx_ntk))
                node->iTemp = SopSimulate(node);
            for (auto po : GetPrimaryOutputs(approx_ntk))
                po->iTemp = GetFanin0(po)->iTemp;

            std::vector<Node> origin_pos = GetPrimaryOutputs(origin_ntk);
            std::vector<Node> approx_pos = GetPrimaryOutputs(approx_ntk);
            for (int i = 0; i < (int) origin_pos.size(); i++) {
                if (origin_pos[i]->iTemp != approx_pos[i]->iTemp) {
                    err++;
                    break;
                }
            }
        }
        return (double) err / (double) simu_time;

    }

    double SimMeanRelativeErrorDistance(Network approx_ntk, bool show_progress_bar, int simu_time) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);
        assert(abc::Abc_NtkIsSopLogic(approx_ntk));

        double err = 0;
        boost::progress_display *pd = nullptr;
        if (show_progress_bar)
            pd = new boost::progress_display((unsigned long) simu_time);

        for (int _ = 0; _ < simu_time; ++_) {
            if (show_progress_bar)
                ++(*pd);

            for (auto pi : GetPrimaryInputs(approx_ntk))
                pi->iTemp = dice();
            for (auto node : TopologicalSort(approx_ntk))
                node->iTemp = SopSimulate(node);
            for (auto po : GetPrimaryOutputs(approx_ntk))
                po->iTemp = GetFanin0(po)->iTemp;

            std::vector<int> inputs, outputs;
            for (auto pi : GetPrimaryInputs(approx_ntk))
                inputs.push_back(pi->iTemp);
            for (auto po : GetPrimaryOutputs(approx_ntk))
                outputs.push_back(po->iTemp);

            int in1 = 0, in2 = 0, out = 0;
            for (int i = 0; i < inputs.size() / 2; i++)
                in1 += inputs[i] * pow(2, i);
            for (int i = (int) inputs.size() / 2; i < inputs.size(); i++)
                in2 += inputs[i] * pow(2, i - inputs.size() / 2);
            for (int i = 0; i < outputs.size(); i++)
                out += outputs[i] * pow(2, i);
            if(in1 + in2 != 0)
                err += (double) abs(out - in1 - in2) /  (double) (in1 + in2) / (double) simu_time;
        }
        return err;
    }

    static std::vector<Node> GetAllNodes(Network ntk) {
        std::vector<Node> internal_nodes = GetInternalNodes(ntk);
        std::vector<Node> pi_nodes = GetPrimaryInputs(ntk);
        pi_nodes.insert(pi_nodes.end(),
                        std::make_move_iterator(internal_nodes.begin()),
                        std::make_move_iterator(internal_nodes.end()));
        return pi_nodes;
    }

    std::map<Node, double> SimZeroProb(Network ntk, int simu_time) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);
        assert(abc::Abc_NtkIsSopLogic(ntk));

        std::map<Node, double> zero_prob;
        std::vector<Node> all_nodes = GetAllNodes(ntk);

        for (auto node : GetAllNodes(ntk))
            zero_prob[node] = -1;

        for (int _ = 0; _ < simu_time; ++_) {
            std::vector<Node> pis = GetPrimaryInputs(ntk);

            for (auto pi:pis) {
                pi->iTemp = dice();
                if (pi->iTemp == 0) {
                    if (zero_prob[pi] == -1)
                        zero_prob[pi] = 0;
                    zero_prob[pi]++;
                }
            }

            for (auto node : TopologicalSort(ntk)) {
                node->iTemp = SopSimulate(node);
                if (node->iTemp == 0) {
                    if (zero_prob[node] == -1)
                        zero_prob[node] = 0;
                    zero_prob[node]++;
                }
            }
        }

        for (auto node : GetAllNodes(ntk))
            if (zero_prob[node] != -1)
                zero_prob[node] /= simu_time;

        return zero_prob;
    }

    void SimTest(Network ntk) {
        std::default_random_engine generator((unsigned) std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> distribution(0, 1);
        auto dice = std::bind(distribution, generator);

        assert(abc::Abc_NtkIsSopLogic(ntk));
        for (auto pi : GetPrimaryInputs(ntk))
            pi->iTemp = dice();
        for (auto node : TopologicalSort(ntk))
            node->iTemp = SopSimulate(node);
        for (auto po : GetPrimaryOutputs(ntk))
            po->iTemp = GetFanin0(po)->iTemp;

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
