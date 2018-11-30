#include <boost/progress.hpp>
#include <ectl_simulator.h>
#include "sta.h"
#include "sasimi.h"

void SASIMI::LoadNetwork(const NtkPtr &ntk) {
    ntk_ = ntk;
    ntk_bak_ = ntk->Duplicate();
}

void SASIMI::InitFaninCones(bool print_result) {
    for (const auto &obj : ntk_->GetObjs())
        if (obj && obj->IsNode()) {
            fan_in_cones_.emplace(obj->GetID(), FaninCone());
            fan_in_cones_.at(obj->GetID()).insert(obj->GetID());
        }

    for (const auto &node : TopologicalSort(ntk_)) {
        for (const auto &fan_in : node->GetFanins()) {
            if (fan_in->IsPrimaryInput())
                fan_in_cones_[node->GetID()].insert(fan_in->GetID());
            else
                fan_in_cones_[node->GetID()].merge(FaninCone(fan_in_cones_[fan_in->GetID()]));
        }
    }

    if (print_result) {
        for (const auto &node : TopologicalSort(ntk_)) {
            std::cout << node->GetName() << ": ";
            for (auto id : fan_in_cones_[node->GetID()])
                std::cout << node->GetObjbyID(id)->GetName() << " ";
            std::cout << std::endl;
        }
    }

}

std::vector<Candidate> SASIMI::GetLegalCands(const ObjPtr &target_node) {
    return legal_cands_.at(target_node);
}

std::vector<Candidate> SASIMI::GetBestCands(const std::vector<ObjPtr> &target_nodes, bool real_error, bool show_progress_bar) {
    GenerateLegalCands(target_nodes);
    std::vector<Candidate> best_cands;
    if (!real_error) GenerateTruthVector();

    boost::progress_display *pd = nullptr;
    if (show_progress_bar)
        pd = new boost::progress_display(target_nodes.size());

    for (const auto &t_node: target_nodes) {
        if (show_progress_bar)
            ++(*pd);
        Candidate best_cand;
        best_cand.SetError(100.0);

        for (auto &t_cand : GetLegalCands(t_node)) {
            t_cand.GenTargetBak(ntk_bak_);
            double error_0 = 100.0, error_1 = 100.0;

            t_cand.SetComplemented(false);
            if (real_error) {
                t_cand.Do();
                error_0 = SimER(ntk_bak_, ntk_, false, 1000);
                t_cand.Recover();
            } else
                error_0 = EstSubPairError(t_cand.GetTarget(), t_cand.GetSubstitute());

            if (t_cand.CanBeComplemented()) {
                t_cand.SetComplemented(true);
                if (real_error) {
                    t_cand.Do();
                    error_1 = SimER(ntk_bak_, ntk_, false, 1000);
                    t_cand.Recover();
                } else
                    error_1 = 1 - EstSubPairError(t_cand.GetTarget(), t_cand.GetSubstitute());
            }

            if (std::min(error_0, error_1) < best_cand.GetError()) {
                best_cand = t_cand;
                if (error_0 < error_1) {
                    best_cand.SetComplemented(false);
                    best_cand.SetError(error_0);
                } else {
                    best_cand.SetComplemented(true);
                    best_cand.SetError(error_1);
                }
            }

        }
//        std::cout << best_cand.GetTarget()->GetName() << " " << std::flush;
        best_cands.push_back(best_cand);
    }
    return best_cands;
}

void SASIMI::GenerateTruthVector() {
    truth_vec_ = SimTruthVec(ntk_);
}

void SASIMI::GenerateLegalCands(const std::vector<ObjPtr> &target_nodes) {
    legal_cands_.clear();
    for (const auto &t_node : target_nodes)
        legal_cands_.emplace(t_node, std::vector<Candidate>());

    auto time_man = CalculateSlack(ntk_);

    for (const auto &t_node : target_nodes)
        for (const auto &s_node : TopologicalSort(ntk_))
            if (t_node != s_node && time_man.at(s_node).arrival_time < time_man.at(t_node).arrival_time) {
                bool can_be_complemented = time_man.at(s_node).arrival_time < time_man.at(t_node).arrival_time - 1;
                legal_cands_[t_node].emplace_back(can_be_complemented, t_node, s_node);
            }
}

double SASIMI::EstSubPairError(const ObjPtr &target, const ObjPtr &substitute) {
    assert(truth_vec_.at(target).size() == truth_vec_.at(substitute).size());
    auto vec_size = truth_vec_.at(target).size();
    double err = 0;

    for (unsigned long i = 0; i < vec_size; i++) {
        if (truth_vec_.at(target)[i] != truth_vec_.at(substitute)[i])
            err += 1;
    }
    err /= vec_size;
    return err;
}

void Candidate::GenTargetBak(NtkPtr ntk_bak) {
    assert(target != nullptr);
    target_bak = ntk_bak->GetObjbyID(target->GetID());
}

void Candidate::SetTarget(ObjPtr t) { target = std::move(t); }

ObjPtr Candidate::GetTarget() const { return target; }

void Candidate::SetSubstitute(ObjPtr sub) { substitute = std::move(sub); }

ObjPtr Candidate::GetSubstitute() const { return substitute; }

void Candidate::SetError(double err) { error = err; }

double Candidate::GetError() const { return error; }

void Candidate::SetComplemented(bool complemented) { is_complemented = complemented; }

bool Candidate::IsComplemented() const { return is_complemented; }

bool Candidate::CanBeComplemented() const { return can_be_complemented; }

void Candidate::Do() {
    auto ntk = target->GetHostNetwork();
    if (is_complemented) {
        inv = ntk->CreateInverter(substitute);
        ntk->ReplaceObj(target, inv);
    } else {
        ntk->ReplaceObj(target, substitute);
    }
}

void Candidate::Recover() {
    auto ntk = target->GetHostNetwork();
    if (is_complemented) {
        ntk->DeleteObj(inv);
        ntk->RecoverObjFrom(target_bak);
    } else {
        ntk->RecoverObjFrom(target_bak);
    }
}

Candidate &Candidate::operator=(const Candidate &other) {
    target = other.target;
    substitute = other.substitute;
    return *this;
}


Candidate::Candidate() : error(100.0), can_be_complemented(false), is_complemented(false),
                         target(nullptr), substitute(nullptr), inv(nullptr), target_bak(nullptr) {}

Candidate::Candidate(bool can_be_complemented, ObjPtr t, ObjPtr s) : can_be_complemented(can_be_complemented),
                                                                           target(std::move(t)),
                                                                           substitute(std::move(s)) {}