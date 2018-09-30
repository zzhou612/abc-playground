#ifndef ECTL_NETWORK_H
#define ECTL_NETWORK_H

#include <abc_api.h>
#include <memory>
#include <vector>

namespace ECTL {
    class Object;

    class Network;

    using ObjectID = int;
    using ObjectPtr = std::shared_ptr<Object>;
    using NetworkPtr = std::shared_ptr<Network>;

    enum class GateType : int {
        CONST0, CONST1, AND, INV
    };

    enum class AndType : int {
        AND0, AND1, AND2, AND3 // 00 01 10 11
    };

    std::ostream &operator<<(std::ostream &os, GateType gt);

    std::ostream &operator<<(std::ostream &os, AndType at);

    class Object {
    public:
        std::string GetName();

        GateType GetGateType();

        int GetiTemp();

        void SetiTemp(int val);

        void _SopSimulate();

        int GetVal();

        void SetVal(int val);

        void Simulate();

        bool IsPrimaryInput();

        bool IsPrimaryOutput();

        bool IsPrimaryOutputNode();

        bool IsNode();

        bool IsInverter();

        bool IsConst();

        unsigned int GetID();

        NetworkPtr GetHostNetwork();

        ObjectPtr GetObjbyID(int id);

        ObjectPtr GetFanin0();

        ObjectPtr GetFanin1();

        std::vector<ObjectPtr> GetFanins();

        std::vector<ObjectPtr> GetFanouts();

        abc::Abc_Obj_t *_Get_Abc_Obj();

        void Renew();

        explicit Object(abc::Abc_Obj_t *abc_node);

        Object(abc::Abc_Obj_t *abc_node, NetworkPtr host_ntk);

    private:
        unsigned int id_;
        GateType gate_type_;
        AndType and_type_;
        bool is_on_set_;

        int val_;

        abc::Abc_Obj_t *abc_obj_;
        bool renewed_;
        NetworkPtr host_ntk_;
        std::vector<ObjectPtr> fan_ins_;
        std::vector<ObjectPtr> fan_outs_;
    };

    class Network : public std::enable_shared_from_this<Network> {
    public:
        void ReadBlifLogic(const std::string &ifile, bool renew = true);

        void WriteBlifLogic(const std::string &ofile);

        void ShowInfo();

        void SetName(const std::string &new_name);

        NetworkPtr Duplicate(bool renew = true);

        NetworkPtr DuplicateDFS(bool renew = true);

        std::string GetName();

        std::vector<ObjectPtr> GetObjs();

        std::vector<ObjectPtr> GetPrimaryInputs();

        std::vector<ObjectPtr> GetPrimaryOutputs();

        std::vector<ObjectPtr> GetNodes();

        std::vector<ObjectPtr> GetPIsNodes();

        ObjectPtr GetObjbyID(int id);

        ObjectPtr GetPrimaryInputbyName(std::string name);

        ObjectPtr GetNodebyName(std::string name);

        void ReplaceObj(ObjectPtr obj_old, ObjectPtr obj_new);

        void RecoverObjFrom(ObjectPtr obj_bak);

        void DeleteObj(ObjectPtr obj);

        ObjectPtr CreateInverter(ObjectPtr fan_in);

        abc::Abc_Ntk_t *_Get_Abc_Ntk();

        void Renew();

        Network();

        explicit Network(abc::Abc_Ntk_t *abc_ntk_);

        ~Network();

    private:
        ObjectPtr _AddAbcObject(abc::Abc_Obj_t *abc_obj, bool renew = true);

        abc::Abc_Ntk_t *abc_ntk_;
        bool renewed;
        std::vector<ObjectPtr> objs_;
        std::vector<ObjectPtr> nodes_;
        std::vector<ObjectPtr> pis_;
        std::vector<ObjectPtr> pos_;
    };
}

#endif
