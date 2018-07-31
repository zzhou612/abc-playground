#ifndef ECTL_NETWORK_H
#define ECTL_NETWORK_H

#include <abc_api.h>
#include <memory>
#include <vector>

namespace ECTL {
    class Node;

    class Network;

    using NodePtr = std::shared_ptr<Node>;

    using NetworkPtr = std::shared_ptr<Network>;

    class Node {
    public:
////////////////////////////////////////////////////////////////////////
        std::string GetName();

        int GetiTemp();

        void SetiTemp(int val);

        void SopSimulate();

        bool IsPrimaryInput();

        bool IsPrimaryOutput();

        bool IsPrimaryOutputNode();

        bool IsNode();

        bool IsInverter();

////////////////////////////////////////////////////////////////////////
        int GetID();

        NodePtr GetFanin0();

        NodePtr GetFanin1();

        std::vector<NodePtr> GetFanIns();

        std::vector<NodePtr> GetFanOuts();

////////////////////////////////////////////////////////////////////////
        abc::Abc_Obj_t *_Get_Abc_Node();

        explicit Node(abc::Abc_Obj_t *abc_node);

    private:
        abc::Abc_Obj_t *abc_node_;
        int id_;
        std::vector<NodePtr> fan_ins_;
        std::vector<NodePtr> fan_outs_;
    };

    class Network {
    public:
        void ReadBlif(const std::string &ifile);

        void WriteBlif(const std::string &ofile);

        void ShowInfo();

        void SetName(const std::string &new_name);

        NetworkPtr Duplicate();

        std::string GetName();

        std::vector<NodePtr> GetPrimaryInputs();

        std::vector<NodePtr> GetPrimaryOutputs();

        std::vector<NodePtr> GetInternalNodes();

        NodePtr GetNodebyID(int id);

        NodePtr GetPrimaryInputbyName(std::string name);

        NodePtr GetInternalNodebyName(std::string name);

        abc::Abc_Ntk_t *_Get_Abc_Ntk();

        Network();

        explicit Network(abc::Abc_Ntk_t *abc_ntk_);

        ~Network();

    private:
        abc::Abc_Ntk_t *abc_ntk_;
        std::vector<NodePtr> nodes_;
    };
}


#endif //DALS_ECTL_NETWORK_H
