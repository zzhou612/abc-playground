#include <iostream>
#include <vector>
#include <map>
#include <bitset>
#include <unordered_set>
#include <random>
#include <chrono>
#include <functional>
#include <boost/progress.hpp>
#include <boost/filesystem.hpp>
#include <abc_api.h>
#include <ectl.h>

using namespace boost::filesystem;
using namespace ECTL;

void Test_1();

void Test_2();

void Test_3();

void Test_4();

void Test_5();

int main() {
    Test_1();
//    Test_2();
//    Test_3();
//    Test_4();
//    Test_5();
    return 0;
}

void Test_1() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C17.blif";

    auto origin_ntk = std::make_shared<Network>();
    origin_ntk->ReadBlifLogic(benchmark_path.string());
    auto approx_ntk = origin_ntk->Duplicate();

    SimTest(approx_ntk);

    std::cout << std::endl;

    for (const auto &pi : approx_ntk->GetPrimaryInputs())
        std::cout << pi->GetID() << "-" << pi->GetName() << " ";
    std::cout << std::endl;

    for (const auto &po : approx_ntk->GetPrimaryOutputs())
        std::cout << po->GetID() << "-" << po->GetName() << " ";
    std::cout << std::endl;

    for (const auto &node : approx_ntk->GetNodes())
        std::cout << node->GetID() << "-" << node->GetName() << " ";
    std::cout << std::endl;

    for (const auto &node: TopologicalSort(approx_ntk))
        std::cout << node->GetID() << "-" << node->GetName() << " ";
    std::cout << std::endl;

    auto target_node = approx_ntk->GetNodebyName("G23gat");
    auto target_node_bak = origin_ntk->GetObjbyID(target_node->GetID());
    auto sub_node = approx_ntk->GetNodebyName("G19gat");
    auto sub_inv = approx_ntk->CreateInverter(sub_node);

    approx_ntk->ReplaceObj(target_node, sub_inv);

    std::cout << SimER(origin_ntk, approx_ntk, true);

    approx_ntk->DeleteObj(sub_inv);
    approx_ntk->RecoverObjFrom(target_node_bak);

    std::cout << SimER(origin_ntk, approx_ntk, true);
}

void Test_2() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C17.blif";
    path out_path = benchmark_dir / "out.blif";

    auto ntk = std::make_shared<Network>();
    ntk->ReadBlifLogic(benchmark_path.string());

    auto target_node = ntk->GetNodebyName("G23gat");
    auto target_node_bak = ntk->GetObjbyID(target_node->GetID());
    auto sub_node = ntk->GetNodebyName("G19gat");

    auto sub_inv = ntk->CreateInverter(sub_node);

    ntk->ReplaceObj(target_node, sub_inv);

    ntk->WriteBlifLogic(out_path.string());

    for (auto &obj : TopologicalSort(ntk)) {
        if (obj->IsNode()) {
            std::cout << obj->GetName() << ": " << obj->GetGateType() << " ";
        }
    }
}

void Test_3() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C432.blif";

    auto ntk = std::make_shared<Network>();
    ntk->ReadBlifLogic(benchmark_path.string());

    std::default_random_engine generator(
            (unsigned) std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> distribution(0, 1);
    auto dice = std::bind(distribution, generator);

    std::cout << "Performing simulation test (one round) for " << ntk->GetName() << ":\n";
    assert(abc::Abc_NtkIsSopLogic(ntk->_Get_Abc_Ntk()));
    for (auto &pi : ntk->GetPrimaryInputs()) {
        int zero_or_one = dice();
        pi->SetiTemp(zero_or_one);
        pi->SetVal((unsigned) zero_or_one);
    }
    for (auto &obj : TopologicalSort(ntk))
        if (obj->IsNode()) {
            obj->_SopSimulate();
            obj->Simulate();
        }
    for (auto &po : ntk->GetPrimaryOutputs()) {
        po->SetiTemp(po->GetFanin0()->GetiTemp());
        po->SetVal(po->GetFanin0()->GetVal());
    }

    std::cout << "Primary inputs:\n";
    for (auto &pi : ntk->GetPrimaryInputs())
        std::cout << pi->GetName() << "=" << pi->GetiTemp() << "=" << std::bitset<64>(pi->GetVal())[0] << " ";
    std::cout << "\nInternal nodes (including primary output nodes):\n";
    for (auto &node: ntk->GetNodes())
        std::cout << node->GetName() << "=" << node->GetiTemp() << "=" << std::bitset<64>(node->GetVal())[0] << " ";
    std::cout << "\nPrimary outputs:\n";
    for (auto &po : ntk->GetPrimaryOutputs())
        std::cout << po->GetName() << "=" << po->GetiTemp() << "=" << std::bitset<64>(po->GetVal())[0] << " ";
    std::cout << std::endl;
}

void Test_4() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C7552.blif";

    auto origin_ntk = std::make_shared<Network>();
    origin_ntk->ReadBlifLogic(benchmark_path.string());
    auto approx_ntk = origin_ntk->Duplicate();

    std::cout << SimER(origin_ntk, approx_ntk, true, 10000);

    std::cout << _SimER(origin_ntk, approx_ntk, true);
}

void Test_5() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C17.blif";

    auto ntk = std::make_shared<Network>();
    ntk->ReadBlifLogic(benchmark_path.string());

    auto node = ntk->GetNodebyName("G11gat");
//    ntk->Renew();
//    auto node_2 = ntk->GetNodebyName("G11gat");
//    std::cout << (node == node_2);

    std::vector<ObjectPtr> objs_;
    objs_.emplace_back(node);
    std::vector<ObjectPtr> nodes_;
    nodes_.emplace_back(node);
    std::cout << (objs_[0] == nodes_[0]);

}
