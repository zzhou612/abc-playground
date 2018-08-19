#include <iostream>
#include <abc_api.h>
#include <ectl_network.h>
#include "ectl_network.h"


namespace abc {
    int Abc_ObjSopSimulate(Abc_Obj_t *pObj);

    // copying the node's name as expected
    static Abc_Obj_t *ECTL_Abc_NtkDupObj(Abc_Ntk_t *pNtkNew, Abc_Obj_t *pObj, int fCopyName) {
        Abc_Obj_t *pObjNew;
        // create the new object
        pObjNew = Abc_NtkCreateObj(pNtkNew, (Abc_ObjType_t) pObj->Type);
        // transfer names of the terminal objects
        if (fCopyName) {
            if (Abc_ObjIsCi(pObj)) {
                if (!Abc_NtkIsNetlist(pNtkNew))
                    Abc_ObjAssignName(pObjNew, Abc_ObjName(Abc_ObjFanout0Ntk(pObj)), NULL);
            } else if (Abc_ObjIsCo(pObj)) {
                if (!Abc_NtkIsNetlist(pNtkNew)) {
                    if (Abc_ObjIsPo(pObj))
                        Abc_ObjAssignName(pObjNew, Abc_ObjName(Abc_ObjFanin0Ntk(pObj)), NULL);
                    else {
                        assert(Abc_ObjIsLatch(Abc_ObjFanout0(pObj)));
                        Abc_ObjAssignName(pObjNew, Abc_ObjName(pObj), NULL);
                    }
                }
            } else if (Abc_ObjIsBox(pObj) || Abc_ObjIsNet(pObj))
                Abc_ObjAssignName(pObjNew, Abc_ObjName(pObj), NULL);
        }
        // copy functionality/names
        if (Abc_ObjIsNode(pObj)) // copy the function if functionality is compatible
        {
            Abc_ObjAssignName(pObjNew, Abc_ObjName(pObj), NULL);  //MODIFIED: copy node's name as expected
            if (pNtkNew->ntkFunc == pObj->pNtk->ntkFunc) {
                if (Abc_NtkIsStrash(pNtkNew)) {}
                else if (Abc_NtkHasSop(pNtkNew) || Abc_NtkHasBlifMv(pNtkNew))
                    pObjNew->pData = Abc_SopRegister((Mem_Flex_t *) pNtkNew->pManFunc, (char *) pObj->pData);
#ifdef ABC_USE_CUDD
                    else if ( Abc_NtkHasBdd(pNtkNew) )
                pObjNew->pData = Cudd_bddTransfer((DdManager *)pObj->pNtk->pManFunc, (DdManager *)pNtkNew->pManFunc, (DdNode *)pObj->pData), Cudd_Ref((DdNode *)pObjNew->pData);
#endif
                else if (Abc_NtkHasAig(pNtkNew))
                    pObjNew->pData = Hop_Transfer((Hop_Man_t *) pObj->pNtk->pManFunc, (Hop_Man_t *) pNtkNew->pManFunc,
                                                  (Hop_Obj_t *) pObj->pData, Abc_ObjFaninNum(pObj));
                else if (Abc_NtkHasMapping(pNtkNew))
                    pObjNew->pData = pObj->pData, pNtkNew->nBarBufs2 += !pObj->pData;
                else
                    assert(0);
            }
        } else if (Abc_ObjIsNet(pObj)) // copy the name
        {
        } else if (Abc_ObjIsLatch(pObj)) // copy the reset value
            pObjNew->pData = pObj->pData;
        // transfer HAIG
//    pObjNew->pEquiv = pObj->pEquiv;
        // remember the new node in the old node
        pObj->pCopy        = pObjNew;
        return pObjNew;
    }

    static Abc_Ntk_t *ECTL_Abc_NtkDup(Abc_Ntk_t *pNtk) {
        Abc_Ntk_t *pNtkNew;
        Abc_Obj_t *pObj, *pFanin;
        int       i, k;
        if (pNtk == NULL)
            return NULL;
        // start the network
        pNtkNew = Abc_NtkStartFrom(pNtk, pNtk->ntkType, pNtk->ntkFunc);
        // copy the internal nodes
        if (Abc_NtkIsStrash(pNtk)) {
            // copy the AND gates
            Abc_AigForEachAnd(pNtk, pObj, i)
                    pObj->pCopy = Abc_AigAnd((Abc_Aig_t *) pNtkNew->pManFunc,
                                             Abc_ObjChild0Copy(pObj), Abc_ObjChild1Copy(pObj));
            // relink the choice nodes
            Abc_AigForEachAnd(pNtk, pObj, i) if (pObj->pData)
                    pObj->pCopy->pData = ((Abc_Obj_t *) pObj->pData)->pCopy;
            // relink the CO nodes
            Abc_NtkForEachCo(pNtk, pObj, i)Abc_ObjAddFanin(pObj->pCopy, Abc_ObjChild0Copy(pObj));
            // get the number of nodes before and after
            if (Abc_NtkNodeNum(pNtk) != Abc_NtkNodeNum(pNtkNew))
                printf("Warning: Structural hashing during duplication reduced %d nodes (this is a minor bug).\n",
                       Abc_NtkNodeNum(pNtk) - Abc_NtkNodeNum(pNtkNew));
        } else {
            // duplicate the nets and nodes (CIs/COs/latches already dupped)
            Abc_NtkForEachObj(pNtk, pObj, i) if (pObj->pCopy == NULL)
                    ECTL_Abc_NtkDupObj(pNtkNew, pObj, Abc_NtkHasBlackbox(pNtk) && Abc_ObjIsNet(pObj));
            // reconnect all objects (no need to transfer attributes on edges)
            Abc_NtkForEachObj(pNtk, pObj, i) if (!Abc_ObjIsBox(pObj) && !Abc_ObjIsBo(pObj))
                    Abc_ObjForEachFanin(pObj, pFanin, k)Abc_ObjAddFanin(pObj->pCopy, pFanin->pCopy);
        }
        // duplicate the EXDC Ntk
        if (pNtk->pExdc)
            pNtkNew->pExdc   = Abc_NtkDup(pNtk->pExdc);
        if (pNtk->pExcare)
            pNtkNew->pExcare = Abc_NtkDup((Abc_Ntk_t *) pNtk->pExcare);
        // duplicate timing manager
        if (pNtk->pManTime)
            Abc_NtkTimeInitialize(pNtkNew, pNtk);
        if (pNtk->vPhases)
            Abc_NtkTransferPhases(pNtkNew, pNtk);
        if (pNtk->pWLoadUsed)
            pNtkNew->pWLoadUsed = Abc_UtilStrsav(pNtk->pWLoadUsed);
        // check correctness
        if (!Abc_NtkCheck(pNtkNew))
            fprintf(stdout, "Abc_NtkDup(): Network check has failed.\n");
        pNtk->pCopy = pNtkNew;
        return pNtkNew;
    }

    static inline int Abc_ObjFanoutId(Abc_Obj_t *pObj, int i) { return pObj->vFanouts.pArray[i]; }
}

namespace ECTL {
    std::string Object::GetName() { return std::string(Abc_ObjName(abc_obj_)); }

    ObjectPtr Object::GetObjbyID(int id) { return host_ntk_->GetObjbyID(id); }

    ObjectPtr Object::GetFanin0() {
        assert(renewed);
        return fan_ins_[0];
    }

    ObjectPtr Object::GetFanin1() {
        assert(renewed);
        return fan_ins_[1];
    }

    int Object::GetID() { return abc::Abc_ObjId(abc_obj_); }

    NetworkPtr Object::GetHostNetwork() { return host_ntk_; }

    std::vector<ObjectPtr> Object::GetFanins() {
        assert(renewed);
        return fan_ins_;
    }

    std::vector<ObjectPtr> Object::GetFanouts() {
        assert(renewed);
        return fan_outs_;
    }

    bool Object::IsPrimaryInput() { return (bool) abc::Abc_ObjIsPi(abc_obj_); }

    bool Object::IsPrimaryOutput() { return (bool) abc::Abc_ObjIsPo(abc_obj_); }

    abc::Abc_Obj_t *Object::_Get_Abc_Obj() { return abc_obj_; }

    bool Object::IsPrimaryOutputNode() {
        for (auto &fan_out : GetFanouts())
            if (fan_out->IsPrimaryOutput())
                return true;
        return false;
    }

    bool Object::IsNode() { return (bool) abc::Abc_ObjIsNode(abc_obj_); }

    bool Object::IsInverter() { return (bool) abc::Abc_NodeIsInv(abc_obj_); }

    bool Object::IsConst() { return (bool) abc::Abc_NodeIsConst(abc_obj_); }

    void Object::SopSimulate() { SetiTemp(abc::Abc_ObjSopSimulate(abc_obj_)); }

    int Object::GetiTemp() { return abc_obj_->iTemp; }

    void Object::SetiTemp(int val) { abc_obj_->iTemp = val; }

    Object::Object(abc::Abc_Obj_t *abc_node) : abc_obj_(abc_node), renewed(false) {}

    Object::Object(abc::Abc_Obj_t *abc_node, NetworkPtr host_ntk) : abc_obj_(abc_node),
                                                                    renewed(false),
                                                                    host_ntk_(std::move(host_ntk)) {}


    void Object::Renew() {
        int fan_in_id, fan_out_id, i;
        fan_ins_.clear();
        fan_outs_.clear();

        Abc_ObjForEachFaninId(abc_obj_, fan_in_id, i) {
            fan_ins_.push_back(host_ntk_->GetObjbyID(fan_in_id));
        }

        Abc_ObjForEachFanoutId(abc_obj_, fan_out_id, i) {
            fan_outs_.push_back(host_ntk_->GetObjbyID(fan_out_id));
        }

        renewed = true;
    }

    void Network::ReadBlifLogic(const std::string &ifile, bool renew) {
        abc_ntk_ = abc::Io_ReadBlif((char *) ifile.c_str(), 1);
        abc_ntk_ = abc::Abc_NtkToLogic(abc_ntk_);
        if (renew) this->Renew();
    }

    void Network::WriteBlifLogic(const std::string &ofile) {
        abc::Io_WriteBlifLogic(abc::ECTL_Abc_NtkDup(abc_ntk_), (char *) ofile.c_str(), 0);
    }

    NetworkPtr Network::Duplicate(bool renew) {
        auto dup_ntk = std::make_shared<Network>(abc::ECTL_Abc_NtkDup(abc_ntk_));
        if (renew) dup_ntk->Renew();
        return dup_ntk;
    }

    void Network::ShowInfo() {
        std::cout << "Network Name: " << std::string(abc_ntk_->pName) << std::endl;
        std::cout << "Network Type: ";
        if (abc::Abc_NtkIsNetlist(abc_ntk_))
            std::cout << "Netlist\n";
        else if (abc::Abc_NtkIsLogic(abc_ntk_))
            std::cout << "Logic\n";
        else if (abc::Abc_NtkIsStrash(abc_ntk_))
            std::cout << "Strash\n";
        else
            std::cout << "Others\n";

        std::cout << "Network Function: ";
        if (abc::Abc_NtkHasSop(abc_ntk_))
            std::cout << "SOP\n";
        else if (abc::Abc_NtkHasBdd(abc_ntk_))
            std::cout << "BDD\n";
        else if (abc::Abc_NtkHasAig(abc_ntk_))
            std::cout << "AIG\n";
        else if (abc::Abc_NtkHasMapping(abc_ntk_))
            std::cout << "Map\n";
        else
            std::cout << "Others\n";
    }

    std::string Network::GetName() { return std::string(abc_ntk_->pName); }

    void Network::SetName(const std::string &new_name) {
        abc_ntk_->pName = abc::Extra_UtilStrsav((char *) new_name.c_str());
    }

    ObjectPtr Network::GetObjbyID(int id) { return objs_.at((unsigned) id); }

    std::vector<ObjectPtr> Network::GetObjs() {
        return objs_;
    }

    std::vector<ObjectPtr> Network::GetPrimaryInputs() {
        assert(renewed);
        return pis_;
    }

    std::vector<ObjectPtr> Network::GetPrimaryOutputs() {
        assert(renewed);
        return pos_;
    }

    std::vector<ObjectPtr> Network::GetNodes() {
        assert(renewed);
        return nodes_;
    }

    std::vector<ObjectPtr> Network::GetPIsNodes() {
        assert(renewed);
        std::vector<ObjectPtr> pis_nodes;
        pis_nodes.reserve(pis_.size() + nodes_.size());
        pis_nodes.insert(pis_nodes.end(), pis_.begin(), pis_.end());
        pis_nodes.insert(pis_nodes.end(), nodes_.begin(), nodes_.end());
        return pis_nodes;
    }

    ObjectPtr Network::GetPrimaryInputbyName(std::string name) {
        assert(renewed);
        return GetObjbyID(abc::Abc_ObjId(abc::Abc_NtkFindCi(abc_ntk_, (char *) name.c_str())));
    }

    ObjectPtr Network::GetNodebyName(std::string name) {
        assert(renewed);
        return GetObjbyID(abc::Abc_ObjId(abc::Abc_NtkFindNode(abc_ntk_, (char *) name.c_str())));
    }

    void Network::ReplaceObj(ObjectPtr obj_old, ObjectPtr obj_new) {
        abc::Abc_ObjTransferFanout(obj_old->_Get_Abc_Obj(), obj_new->_Get_Abc_Obj());
        renewed = false;

//        obj_new->Renew();
//        obj_old->Renew();
//        for (const auto &fan_out : obj_new->GetFanouts())
//            fan_out->Renew();
//        renewed = true;
        Renew();
    }

    void Network::RecoverObjFrom(ObjectPtr obj_bak) {
        renewed = false;

        for (const auto &fan_out_bak : obj_bak->GetFanouts()) {
            auto abc_fan_out = GetObjbyID(fan_out_bak->GetID())->_Get_Abc_Obj();
            abc::Abc_ObjRemoveFanins(abc_fan_out);

            for (const auto &fan_in_bak : fan_out_bak->GetFanins()) {
                auto abc_fan_in = GetObjbyID(fan_in_bak->GetID())->_Get_Abc_Obj();
                abc::Abc_ObjAddFanin(abc_fan_out, abc_fan_in);
            }
        }

        Renew();
    }

    void Network::DeleteObj(ObjectPtr obj) {
        abc::Abc_NtkDeleteObj(obj->_Get_Abc_Obj());
        renewed = false;
        Renew();
    }


    ObjectPtr Network::CreateInverter(ObjectPtr fan_in) {
        auto abc_inv = abc::Abc_NtkCreateNodeInv(abc_ntk_, fan_in->_Get_Abc_Obj());
        renewed = false;
        _AddAbcObject(abc_inv);
        renewed = true;
        return GetObjbyID(abc::Abc_ObjId(abc_inv));
    }


    abc::Abc_Ntk_t *Network::_Get_Abc_Ntk() { return abc_ntk_; }

    void Network::Renew() {
        abc::Abc_Obj_t *abc_obj;
        int            i;
        objs_.clear();
        nodes_.clear();
        pis_.clear();
        pos_.clear();
        for (int t = 0; t <= abc::Abc_NtkObjNumMax(abc_ntk_); t++) objs_.push_back(nullptr);
        Abc_NtkForEachObj(abc_ntk_, abc_obj, i) {
                _AddAbcObject(abc_obj, false);
            }
        for (const auto &obj : objs_)
            if (obj != nullptr) {
                obj->Renew();
            }
        renewed = true;
    }

    Network::Network() : renewed(false) {};

    Network::Network(abc::Abc_Ntk_t *abc_ntk) : abc_ntk_(abc_ntk), renewed(false) {}

    Network::~Network() { abc::Abc_NtkDelete(abc_ntk_); }

    ObjectPtr Network::_AddAbcObject(abc::Abc_Obj_t *abc_obj, bool renew) {
        auto obj = std::make_shared<Object>(abc_obj, shared_from_this());
        objs_.at(abc::Abc_ObjId(abc_obj)) = obj;
        if (abc::Abc_ObjIsNode(abc_obj))
            nodes_.push_back(obj);
        if (abc::Abc_ObjIsPi(abc_obj))
            pis_.push_back(obj);
        if (abc::Abc_ObjIsPo(abc_obj))
            pos_.push_back(obj);
        if (renew)
            obj->Renew();
        return obj;
    }
}
