#ifndef ECTL_NETWORK_H
#define ECTL_NETWORK_H

#include <abc_api.h>
#include <cstdint>
#include <memory>
#include <vector>

namespace ECTL {
    class Object;

    class Network;

    using ObjID = int;
    using ObjPtr = std::shared_ptr<Object>;
    using NtkPtr = std::shared_ptr<Network>;

    enum class GateType : int {
        CONST0, CONST1, WIRE, AND, INV
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

        uint64_t GetVal();

        void SetVal(uint64_t val);

        void Simulate();

        bool IsPrimaryInput();

        bool IsPrimaryOutput();

        bool IsPrimaryOutputNode();

        bool IsNode();

        bool IsInverter();

        bool IsConst();

        unsigned int GetID();

        NtkPtr GetHostNetwork();

        ObjPtr GetObjbyID(int id);

        ObjPtr GetFanin0();

        ObjPtr GetFanin1();

        std::vector<ObjPtr> GetFanins();

        std::vector<ObjPtr> GetFanouts();

        abc::Abc_Obj_t *_Get_Abc_Obj();

        void Renew();

        explicit Object(abc::Abc_Obj_t *abc_node);

        Object(abc::Abc_Obj_t *abc_node, NtkPtr host_ntk);

    private:
        unsigned int id_;
        GateType gate_type_;
        AndType and_type_;
        bool is_complement;
        uint64_t val_;

        abc::Abc_Obj_t *abc_obj_;
        bool renewed_;
        NtkPtr host_ntk_;
        std::vector<ObjPtr> fan_ins_;
        std::vector<ObjPtr> fan_outs_;
    };

    class Network : public std::enable_shared_from_this<Network> {
    public:
        void ReadBlifLogic(const std::string &ifile, bool renew = true);

        void WriteBlifLogic(const std::string &ofile);

        void ShowInfo();

        void SetName(const std::string &new_name);

        NtkPtr Duplicate(bool renew = true);

        NtkPtr DuplicateDFS(bool renew = true);

        std::string GetName();

        std::vector<ObjPtr> GetObjs();

        std::vector<ObjPtr> GetPIs();

        std::vector<ObjPtr> GetPOs();

        ObjPtr GetObjbyID(int id);

        ObjPtr GetPrimaryInputbyName(std::string name);

        ObjPtr GetNodebyName(std::string name);

        void ReplaceObj(ObjPtr obj_old, ObjPtr obj_new);

        void RecoverObjFrom(ObjPtr obj_bak);

        void DeleteObj(ObjPtr obj);

        ObjPtr CreateInverter(ObjPtr fan_in);

        abc::Abc_Ntk_t *_Get_Abc_Ntk();

        void Renew();

        Network();

        explicit Network(abc::Abc_Ntk_t *abc_ntk_);

        ~Network();

    private:
        ObjPtr _AddAbcObject(abc::Abc_Obj_t *abc_obj, bool renew = true);

        abc::Abc_Ntk_t *abc_ntk_;
        bool renewed_;
        std::vector<ObjPtr> objs_;
        std::vector<ObjPtr> pis_;
        std::vector<ObjPtr> pos_;
    };
}

#endif
