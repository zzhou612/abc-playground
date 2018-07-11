#include <iostream>
#include <string>
#include <abc_api.h>
#include <ectl_network.h>
#include <vector>

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

    // use ECTL_Abc_NtkDupObj for copying to keep the consistency of node names
    static Abc_Ntk_t *ECTL_Abc_NtkCreateMffc(Abc_Ntk_t *pNtk, Abc_Obj_t *pNode, char *pNodeName) {
        Abc_Ntk_t *pNtkNew;
        Abc_Obj_t *pObj, *pFanin, *pNodeCoNew;
        Vec_Ptr_t *vCone, *vSupp;
        char      Buffer[1000];
        int       i, k;

        assert(Abc_NtkIsLogic(pNtk) || Abc_NtkIsStrash(pNtk));
        assert(Abc_ObjIsNode(pNode));

        // start the network
        pNtkNew = Abc_NtkAlloc(pNtk->ntkType, pNtk->ntkFunc, 1);
        // set the name
        sprintf(Buffer, "%s_%s", pNtk->pName, pNodeName);
        pNtkNew->pName = Extra_UtilStrsav(Buffer);

        // establish connection between the constant nodes
        if (Abc_NtkIsStrash(pNtk))
            Abc_AigConst1(pNtk)->pCopy = Abc_AigConst1(pNtkNew);

        // collect the nodes in MFFC
        vCone = Vec_PtrAlloc(100);
        vSupp = Vec_PtrAlloc(100);
        Abc_NodeDeref_rec(pNode);
        Abc_NodeMffcConeSupp(pNode, vCone, vSupp);
        Abc_NodeRef_rec(pNode);
        // create the PIs
        Vec_PtrForEachEntry(Abc_Obj_t *, vSupp, pObj, i) {
            pObj->pCopy = Abc_NtkCreatePi(pNtkNew);
            Abc_ObjAssignName(pObj->pCopy, Abc_ObjName(pObj), NULL);
        }
        // create the PO
        pNodeCoNew = Abc_NtkCreatePo(pNtkNew);
        Abc_ObjAssignName(pNodeCoNew, pNodeName, NULL);
        // copy the nodes
        Vec_PtrForEachEntry(Abc_Obj_t *, vCone, pObj, i) {
            // if it is an AIG, add to the hash table
            if (Abc_NtkIsStrash(pNtk)) {
                pObj->pCopy = Abc_AigAnd((Abc_Aig_t *) pNtkNew->pManFunc, Abc_ObjChild0Copy(pObj),
                                         Abc_ObjChild1Copy(pObj));
            } else {
                ECTL_Abc_NtkDupObj(pNtkNew, pObj, 0);
                Abc_ObjForEachFanin(pObj, pFanin, k)Abc_ObjAddFanin(pObj->pCopy, pFanin->pCopy);
            }
        }
        // connect the topmost node
        Abc_ObjAddFanin(pNodeCoNew, pNode->pCopy);
        Vec_PtrFree(vCone);
        Vec_PtrFree(vSupp);

        if (!Abc_NtkCheck(pNtkNew))
            fprintf(stdout, "Abc_NtkCreateMffc(): Network check has failed.\n");
        return pNtkNew;
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
}

namespace ECTL {
    std::string Node::GetName() {
        return std::string(Abc_ObjName(abc_node_));
    }

    NodePtr Node::GetFanin0() {
        return std::make_shared<Node>(abc::Abc_ObjFanin0(abc_node_));
    }

    NodePtr Node::GetFanin1() {
        return std::make_shared<Node>(abc::Abc_ObjFanin1(abc_node_));
    }

    Node::Node(abc::Abc_Obj_t *abc_node) : abc_node_(abc_node) {}

//    Network Node::CreateMFFCNetwork() {
//        return Network(
//                abc::ECTL_Abc_NtkCreateMffc(abc::Abc_ObjNtk(abc_node_), abc_node_, abc::Abc_ObjName(abc_node_)));
//    }

    int Node::GetID() {
        return abc::Abc_ObjId(abc_node_);
    }

    std::vector<NodePtr> Node::GetFanins() {
        std::vector<NodePtr> fan_ins;
        abc::Abc_Obj_t    *fan_in;
        int               i;
        Abc_ObjForEachFanin(abc_node_, fan_in, i) {
            fan_ins.push_back(std::make_shared<Node>(fan_in));
        }
        return fan_ins;
    }

    std::vector<NodePtr> Node::GetFanouts() {
        std::vector<NodePtr> fan_outs;
        abc::Abc_Obj_t    *fan_out;
        int               i;
        Abc_ObjForEachFanout(abc_node_, fan_out, i) {
            fan_outs.push_back(std::make_shared<Node>(fan_out));
        }
        return fan_outs;
    }

    bool Node::IsPrimaryInput() {
        return (bool) abc::Abc_ObjIsPi(abc_node_);
    }

    bool Node::IsPrimaryOutput() {
        return (bool) abc::Abc_ObjIsPo(abc_node_);

    }

    bool Node::IsPrimaryOutputNode() {
        for (auto &fan_out : GetFanouts())
            if (fan_out->IsPrimaryOutput())
                return true;
        return false;
    }

    bool Node::IsNode() {
        return (bool) abc::Abc_ObjIsNode(abc_node_);
    }

    abc::Abc_Obj_t *Node::_Get_Abc_Node() {
        return abc_node_;
    }

    int Node::SopSimulate() {
        return abc::Abc_ObjSopSimulate(abc_node_);
    }

    void Network::ReadBlif(const std::string &ifile) {
        abc_ntk_ = abc::Io_ReadBlif((char *) ifile.c_str(), 1);
        abc_ntk_ = abc::Abc_NtkToLogic(abc_ntk_); // Convert to Logic form.
    }

    NodePtr Network::GetNodebyID(int id) {
        return std::make_shared<Node>(abc::Abc_NtkObj(abc_ntk_, id));
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

    Network::Network() = default;

    Network::~Network() {
        abc::Abc_NtkDelete(abc_ntk_);
    }

    Network::Network(abc::Abc_Ntk_t *abc_ntk) : abc_ntk_(abc_ntk) {}

    void Network::WriteBlif(const std::string &ofile) {
        abc::Io_WriteBlifLogic(abc_ntk_, (char *) ofile.c_str(), 0);
    }

    NetworkPtr Network::Duplicate() {
        return std::make_shared<Network>(abc::ECTL_Abc_NtkDup(abc_ntk_));
    }

    std::string Network::GetName() {
        return std::string(abc_ntk_->pName);
    }

    void Network::SetName(const std::string &new_name) {
        abc_ntk_->pName = abc::Extra_UtilStrsav((char *) new_name.c_str());
    }

    std::vector<NodePtr> Network::GetPrimaryInputs() {
        std::vector<NodePtr> pis;
        abc::Abc_Obj_t    *pi;
        int               i;
        Abc_NtkForEachPi(abc_ntk_, pi, i) {
            pis.push_back(std::make_shared<Node>(pi));
        }
        return pis;
    }

    std::vector<NodePtr> Network::GetPrimaryOutputs() {
        std::vector<NodePtr> pos;
        abc::Abc_Obj_t    *po;
        int               i;
        Abc_NtkForEachPo(abc_ntk_, po, i) {
            pos.push_back(std::make_shared<Node>(po));
        }
        return pos;
    }

    std::vector<NodePtr> Network::GetInternalNodes() {
        std::vector<NodePtr> nodes;
        abc::Abc_Obj_t    *node;
        int               i;
        Abc_NtkForEachNode(abc_ntk_, node, i) {
                nodes.push_back(std::make_shared<Node>(node));
            }
        return nodes;
    }

    NodePtr Network::GetPrimaryInputbyName(std::string name) {
        return std::make_shared<Node>(abc::Abc_NtkFindCi(abc_ntk_, (char *) name.c_str()));
    }

    NodePtr Network::GetInternalNodebyName(std::string name) {
        return std::make_shared<Node>(abc::Abc_NtkFindNode(abc_ntk_, (char *) name.c_str()));
    }

    abc::Abc_Ntk_t *Network::_Get_Abc_Ntk() {
        return abc_ntk_;
    }
}

