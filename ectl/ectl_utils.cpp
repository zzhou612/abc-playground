#include <ectl_utils.h>

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
        pObj->pCopy = pObjNew;
        return pObjNew;
    }

    // use ECTL_Abc_NtkDupObj for copying to keep the consistency of node names
    static Abc_Ntk_t *ECTL_Abc_NtkCreateMffc(Abc_Ntk_t *pNtk, Abc_Obj_t *pNode, char *pNodeName) {
        Abc_Ntk_t *pNtkNew;
        Abc_Obj_t *pObj, *pFanin, *pNodeCoNew;
        Vec_Ptr_t *vCone, *vSupp;
        char Buffer[1000];
        int i, k;

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
            fprintf(stdout, "Abc_NtkCreateMffc(): Network_t check has failed.\n");
        return pNtkNew;
    }
}

namespace ECTL {
    std::vector<ObjectPtr> TopologicalSort(const NetworkPtr &ntk) {
        abc::Vec_Ptr_t *abc_objs = abc::Abc_NtkDfs(ntk->_Get_Abc_Ntk(), 0);
        std::vector<ObjectPtr> sorted_objs;

        for (int i = 0; i < abc_objs->nSize; ++i) {
            int id = abc::Abc_ObjId((abc::Abc_Obj_t *) abc_objs->pArray[i]);
            sorted_objs.push_back(ntk->GetObjbyID(id));
        }
        Vec_PtrFree(abc_objs);

        auto pis = ntk->GetPrimaryInputs();
        sorted_objs.insert(sorted_objs.begin(),
                           std::make_move_iterator(pis.begin()),
                           std::make_move_iterator(pis.end()));
        return sorted_objs;
    }

//    Node_t CreateConstNode(Network_t ntk, int constant) {
//        assert(constant == 0 || constant == 1);
//        if (constant)
//            return abc::Abc_NtkCreateNodeConst1(ntk);
//        else
//            return abc::Abc_NtkCreateNodeConst0(ntk);
//    }
//
//    void ReplaceNode(Node_t old_node, Node_t new_node) {
//        abc::Abc_ObjReplace(old_node, new_node);
//    }

    void PrintMFFC(const ObjectPtr &node) {
        //Abc_NodeMffcConeSuppPrint
        using namespace abc;
        Vec_Ptr_t *vCone, *vSupp;
        Abc_Obj_t *pObj;
        int i;
        vCone = Vec_PtrAlloc(100);
        vSupp = Vec_PtrAlloc(100);
        Abc_NodeDeref_rec(node->_Get_Abc_Obj());
        Abc_NodeMffcConeSupp(node->_Get_Abc_Obj(), vCone, vSupp);
        Abc_NodeRef_rec(node->_Get_Abc_Obj());
        printf("Node_t = %6s : Supp = %3d  Cone = %3d  (",
               Abc_ObjName(node->_Get_Abc_Obj()), Vec_PtrSize(vSupp), Vec_PtrSize(vCone));
        Vec_PtrForEachEntry(Abc_Obj_t *, vCone, pObj, i)printf(" %s", Abc_ObjName(pObj));
        printf(" )\n");
        Vec_PtrFree(vCone);
        Vec_PtrFree(vSupp);
    }

    std::vector<ObjectPtr> GetMFFCNodes(const ObjectPtr &node) {
        std::vector<ObjectPtr> mffc;
        abc::Vec_Ptr_t *vCone, *vSupp;
        abc::Abc_Obj_t *pObj;
        int i;
        vCone = abc::Vec_PtrAlloc(100);
        vSupp = abc::Vec_PtrAlloc(100);
        abc::Abc_NodeDeref_rec(node->_Get_Abc_Obj());
        Abc_NodeMffcConeSupp(node->_Get_Abc_Obj(), vCone, vSupp);
        abc::Abc_NodeRef_rec(node->_Get_Abc_Obj());
        Vec_PtrForEachEntry(abc::Abc_Obj_t *, vCone, pObj, i) {
            mffc.push_back(node->GetObjbyID(abc::Abc_ObjId(pObj)));
        }
        Vec_PtrFree(vCone);
        Vec_PtrFree(vSupp);
        return mffc;
    }

    std::vector<ObjectPtr> GetMFFCInputs(const ObjectPtr &node) {
        std::vector<ObjectPtr> inputs;
        abc::Vec_Ptr_t *vCone, *vSupp;
        abc::Abc_Obj_t *pObj;
        int i;
        vCone = abc::Vec_PtrAlloc(100);
        vSupp = abc::Vec_PtrAlloc(100);
        abc::Abc_NodeDeref_rec(node->_Get_Abc_Obj());
        Abc_NodeMffcConeSupp(node->_Get_Abc_Obj(), vCone, vSupp);
        abc::Abc_NodeRef_rec(node->_Get_Abc_Obj());
        Vec_PtrForEachEntry(abc::Abc_Obj_t *, vSupp, pObj, i) {
            inputs.push_back(node->GetObjbyID(abc::Abc_ObjId(pObj)));
        }
        Vec_PtrFree(vCone);
        Vec_PtrFree(vSupp);
        return inputs;
    }

    NetworkPtr CreateMFFCNetwork(const ObjectPtr &node) {
        auto mffc_ntk = std::make_shared<Network>(abc::ECTL_Abc_NtkCreateMffc(abc::Abc_ObjNtk(node->_Get_Abc_Obj()),
                                                                              node->_Get_Abc_Obj(),
                                                                              abc::Abc_ObjName(node->_Get_Abc_Obj())));
        mffc_ntk->Renew();
        return mffc_ntk;
    }
}
