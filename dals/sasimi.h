#ifndef DALS_SASIMI_H
#define DALS_SASIMI_H

#include <unordered_set>
#include <unordered_map>
#include <ectl_network.h>
#include <ectl_utils.h>

using namespace ECTL;

class Candidate {
public:
    void GenTargetBak(NtkPtr ntk_bak);

    void SetTarget(ObjPtr t);

    ObjPtr GetTarget() const;

    void SetSubstitute(ObjPtr sub);

    ObjPtr GetSubstitute() const;

    void SetError(double err);

    double GetError() const;

    void SetComplemented(bool complemented);

    bool IsComplemented() const;

    bool CanBeComplemented() const;

    void Do();

    void Recover();

    Candidate &operator=(const Candidate &other);

    Candidate();

    Candidate(bool can_be_complemented, ObjPtr t, ObjPtr s);

private:
    double error;
    bool can_be_complemented;
    bool is_complemented;
    ObjPtr target, substitute;
    ObjPtr inv;
    ObjPtr target_bak;
};

class SASIMI {
public:
    void LoadNetwork(const NtkPtr &ntk);

    void GenerateTruthVector();

    std::vector<Candidate>
    GetBestCands(const std::vector<ObjPtr> &target_nodes, bool real_error = false, bool show_progress_bar = false);

    double EstSubPairError(const ObjPtr &target, const ObjPtr &substitute);

private:
    void InitFaninCones(bool print_result = false);

    void GenerateLegalCands(const std::vector<ObjPtr> &target_nodes);

    std::vector<Candidate> GetLegalCands(const ObjPtr &target_node);

    using FaninCone = std::unordered_set<ObjID>;

    NtkPtr ntk_;
    NtkPtr ntk_bak_;
    std::unordered_map<ObjID, FaninCone> fan_in_cones_;
    std::unordered_map<ObjPtr, std::vector<Candidate>> legal_cands_;
    std::unordered_map<ObjPtr, std::vector<int>> truth_vec_;

};

#endif
