#include <iostream>
#include <vector>
#include <map>
#include <bitset>
#include <unordered_set>
#include <random>
#include <chrono>
#include <functional>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <abc_api.h>
#include <ectl.h>

using namespace boost::filesystem;
using namespace ECTL;

void ApproxSubTest();

void SimTest();

void SimSpdTest();

int main() {
    ApproxSubTest();
    SimTest();
    SimSpdTest();
    return 0;
}

void ApproxSubTest() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C17.blif";

    auto origin_ntk = std::make_shared<Network>();
    origin_ntk->ReadBlifLogic(benchmark_path.string());
    auto approx_ntk = origin_ntk->Duplicate();

    for (const auto &pi : approx_ntk->GetPIs())
        std::cout << pi->GetID() << "-" << pi->GetName() << " ";
    std::cout << std::endl;

    for (const auto &po : approx_ntk->GetPOs())
        std::cout << po->GetID() << "-" << po->GetName() << " ";
    std::cout << std::endl;

    for (const auto &obj : approx_ntk->GetObjs())
        if (obj->IsNode())
            std::cout << obj->GetID() << "-" << obj->GetName() << " ";
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

    std::cout << SimER(origin_ntk, approx_ntk, true) << std::endl;
}

void SimTest() {
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
    for (auto &pi : ntk->GetPIs()) {
        int zero_or_one = dice();
        pi->SetiTemp(zero_or_one);
        pi->SetVal((unsigned) zero_or_one);
    }
    for (auto &obj : TopologicalSort(ntk))
        if (obj->IsNode()) {
            obj->_SopSimulate();
            obj->Simulate();
        }
    for (auto &po : ntk->GetPOs()) {
        po->SetiTemp(po->GetFanin0()->GetiTemp());
        po->SetVal(po->GetFanin0()->GetVal());
    }

    std::cout << "Primary inputs:\n";
    for (auto &pi : ntk->GetPIs())
        std::cout << pi->GetName() << "=" << pi->GetiTemp() << "=" << std::bitset<64>(pi->GetVal())[0] << " ";
    std::cout << "\nInternal nodes (including primary output nodes):\n";
    for (auto &obj: ntk->GetObjs())
        if (obj->IsNode()) {
            std::cout << obj->GetName() << "=" << obj->GetiTemp() << "=" << std::bitset<64>(obj->GetVal())[0] << " ";
        }
    std::cout << "\nPrimary outputs:\n";
    for (auto &po : ntk->GetPOs())
        std::cout << po->GetName() << "=" << po->GetiTemp() << "=" << std::bitset<64>(po->GetVal())[0] << " ";
    std::cout << std::endl;
}

void SimSpdTest() {
    path project_source_dir(PROJECT_SOURCE_DIR);
    path benchmark_dir = project_source_dir / "benchmark";
    path benchmark_path = benchmark_dir / "C1908.blif";

    auto ntk = std::make_shared<Network>();
    ntk->ReadBlifLogic(benchmark_path.string());
    boost::timer::cpu_timer timer;
    timer.start();
    SimER(ntk, ntk, true);
    timer.stop();
    std::cout << timer.format() << std::endl;
    timer.start();
    _SimER(ntk, ntk, true);
    timer.stop();
    std::cout << timer.format() << std::endl;
}
