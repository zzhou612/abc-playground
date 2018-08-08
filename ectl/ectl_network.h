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

    class Object {
    public:
        std::string GetName();

        int GetiTemp();

        void SetiTemp(int val);

        void SopSimulate();

        bool IsPrimaryInput();

        bool IsPrimaryOutput();

        bool IsPrimaryOutputNode();

        bool IsNode();

        bool IsInverter();

        int GetID();

        ObjectPtr GetObjbyID(int id);

        ObjectPtr GetFanin0();

        ObjectPtr GetFanin1();

        std::vector<ObjectPtr> GetFanins();

        std::vector<ObjectPtr> GetFanouts();

        abc::Abc_Obj_t *_Get_Abc_Node();

        void Renew();

        explicit Object(abc::Abc_Obj_t *abc_node);

        Object(abc::Abc_Obj_t *abc_node, NetworkPtr host_ntk);

    private:
        abc::Abc_Obj_t *abc_obj_;
        bool renewed;

        NetworkPtr             host_ntk_;
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

        std::string GetName();

        std::vector<ObjectPtr> GetPrimaryInputs();

        std::vector<ObjectPtr> GetPrimaryOutputs();

        std::vector<ObjectPtr> GetNodes();

        ObjectPtr GetObjbyID(int id);

        ObjectPtr GetPrimaryInputbyName(std::string name);

        ObjectPtr GetNodebyName(std::string name);

        abc::Abc_Ntk_t *_Get_Abc_Ntk();

        void Renew();

        Network();

        explicit Network(abc::Abc_Ntk_t *abc_ntk_);

        ~Network();

    private:
        abc::Abc_Ntk_t *abc_ntk_;
        bool renewed;

        std::vector<ObjectPtr> objs_;
        std::vector<ObjectPtr> nodes_;
        std::vector<ObjectPtr> pis_;
        std::vector<ObjectPtr> pos_;
    };
}


#endif //DALS_ECTL_NETWORK_H
