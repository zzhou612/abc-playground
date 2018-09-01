#ifndef DALS_SASIMI_H
#define DALS_SASIMI_H

#include <unordered_set>
#include <unordered_map>
#include <ectl_network.h>
#include <ectl_utils.h>

using namespace ECTL;

class Candidate {
public:
    void GenTargetBak(NetworkPtr ntk_bak);

    void SetTarget(ObjectPtr t);

    ObjectPtr GetTarget() const;

    void SetSubstitute(ObjectPtr sub);

    ObjectPtr GetSubstitute() const;

    void SetError(double err);

    double GetError() const;

    void SetComplemented(bool complemented);

    bool IsComplemented() const;

    bool CanBeComplemented() const;

    void Do();

    void Recover();

    Candidate &operator=(const Candidate &other);

    Candidate();

    Candidate(bool can_be_complemented, ObjectPtr t, ObjectPtr s);

private:
    double error;
    bool can_be_complemented;
    bool is_complemented;
    ObjectPtr target, substitute;
    ObjectPtr inv;
    ObjectPtr target_bak;
};

class SASIMI {
public:
    void LoadNetwork(const NetworkPtr &ntk);

    void GenerateTruthVector();

    std::vector<Candidate>
    GetBestCands(const std::vector<ObjectPtr> &target_nodes, bool real_error = false, bool show_progress_bar = false);

    double EstSubPairError(const ObjectPtr &target, const ObjectPtr &substitute);

private:
    void InitFaninCones(bool print_result = false);

    void GenerateLegalCands(const std::vector<ObjectPtr> &target_nodes);

    std::vector<Candidate> GetLegalCands(const ObjectPtr &target_node);


    using FaninCone = std::unordered_set<ObjectID>;

    NetworkPtr ntk_;
    NetworkPtr ntk_bak_;
    std::unordered_map<ObjectID, FaninCone> fan_in_cones_;
    std::unordered_map<ObjectPtr, std::vector<Candidate>> legal_cands_;
    std::unordered_map<ObjectPtr, std::vector<int>> truth_vec_;

};

#endif
