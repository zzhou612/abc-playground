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
            fprintf(stdout, "Abc_NtkCreateMffc(): Network check has failed.\n");
        return pNtkNew;
    }

    static Abc_Ntk_t *ECTL_Abc_NtkDup(Abc_Ntk_t *pNtk) {
        Abc_Ntk_t *pNtkNew;
        Abc_Obj_t *pObj, *pFanin;
        int i, k;
        if (pNtk == NULL)
            return NULL;
        // start the network
        pNtkNew = Abc_NtkStartFrom(pNtk, pNtk->ntkType, pNtk->ntkFunc);
        // copy the internal nodes
        if (Abc_NtkIsStrash(pNtk)) {
            // copy the AND gates
            Abc_AigForEachAnd(pNtk, pObj, i)pObj->pCopy = Abc_AigAnd((Abc_Aig_t *) pNtkNew->pManFunc,
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
            pNtkNew->pExdc = Abc_NtkDup(pNtk->pExdc);
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
    Frame StartAbc() {
        abc::Abc_Start();
        Frame frame = abc::Abc_FrameGetGlobalFrame();
        if (!frame) {
            abc::Abc_Stop();
            std::cout << "[e] Error: Not able to invoke the ABC frame" << std::endl;
            std::exit(-1);
        }
        return frame;
    }

    void StopABC() {
        abc::Abc_Stop();
    }

    Network DuplicateNetwork(Network ntk) {
        return abc::ECTL_Abc_NtkDup(ntk);
    }

    Network ReadBlif(std::string ifile) {
        Network ntk = abc::Io_ReadBlif((char *) ifile.c_str(), 1);
        ntk = abc::Abc_NtkToLogic(ntk); // Convert to Logic form.
        return ntk;
    }

    void WriteBlif(Network ntk, std::string ofile) {
        abc::Io_WriteBlifLogic(ntk, (char *) ofile.c_str(), 0);
    }

    void ShowNetworkInfo(Network ntk) {
        std::cout << "Network Name: " << GetNetworkName(ntk) << std::endl;

        std::cout << "Network Type: ";
        if (abc::Abc_NtkIsNetlist(ntk))
            std::cout << "Netlist\n";
        else if (abc::Abc_NtkIsLogic(ntk))
            std::cout << "Logic\n";
        else if (abc::Abc_NtkIsStrash(ntk))
            std::cout << "Strash\n";
        else
            std::cout << "Others\n";

        std::cout << "Network Function: ";
        if (abc::Abc_NtkHasSop(ntk))
            std::cout << "SOP\n";
        else if (abc::Abc_NtkHasBdd(ntk))
            std::cout << "BDD\n";
        else if (abc::Abc_NtkHasAig(ntk))
            std::cout << "AIG\n";
        else if (abc::Abc_NtkHasMapping(ntk))
            std::cout << "Map\n";
        else
            std::cout << "Others\n";
    }

    std::string GetNetworkName(Network ntk) {
        return std::string(ntk->pName);
    }

    void SetNetworkName(Network ntk, const std::string &new_name) {
        ntk->pName = abc::Extra_UtilStrsav((char *) new_name.c_str());
    }

    void DeleteNetwork(Network ntk) {
        abc::Abc_NtkDelete(ntk);
    }

    std::vector<Node> TopologicalSort(Network ntk) {
        abc::Vec_Ptr_t *nodes = abc::Abc_NtkDfs(ntk, 0);
        std::vector<Node> sorted_nodes;
        for (int i = 0; i < nodes->nSize; ++i) {
            auto obj = (Node) nodes->pArray[i];
            sorted_nodes.emplace_back(obj);
        }
        Vec_PtrFree(nodes);
        return sorted_nodes;
    }

    std::vector<Node> GetPrimaryInputs(Network ntk) {
        std::vector<Node> pis;
        Node pi;
        int i;
        Abc_NtkForEachPi(ntk, pi, i) {
            pis.emplace_back(pi);
        }
        return pis;
    }

    std::vector<Node> GetPrimaryOutputs(Network ntk) {
        std::vector<Node> pos;
        Node po;
        int i;
        Abc_NtkForEachPo(ntk, po, i) {
            pos.emplace_back(po);
        }
        return pos;
    }

    Node GetNodebyName(Network ntk, std::string name) {
        return abc::Abc_NtkFindNode(ntk, (char *) name.c_str());
    }

    Network GetHostNetwork(Node node) {
        return abc::Abc_ObjNtk(node);
    }

    std::vector<Node> GetInternalNodes(Network ntk) {
        std::vector<Node> nodes;
        Node node;
        int i;
        Abc_NtkForEachNode(ntk, node, i) {
                nodes.emplace_back(node);
            }
        return nodes;
    }

    std::string GetNodeName(Node node) {
        return std::string(Abc_ObjName(node));
    }

    std::vector<Node> GetFanins(Node node) {
        std::vector<Node> fan_ins;
        Node fan_in;
        int i;
        Abc_ObjForEachFanin(node, fan_in, i) {
            fan_ins.emplace_back(fan_in);
        }
        return fan_ins;
    }

    Node GetFanin0(Node node) {
        return abc::Abc_ObjFanin0(node);
    }

    Node GetFanin1(Node node) {
        return abc::Abc_ObjFanin1(node);
    }

    std::vector<Node> GetFanouts(Node node) {
        std::vector<Node> fan_outs;
        Node fan_out;
        int i;
        Abc_ObjForEachFanout(node, fan_out, i) {
            fan_outs.emplace_back(fan_out);
        }
        return fan_outs;
    }

    Node CreateConstNode(Network ntk, int constant) {
        assert(constant == 0 || constant == 1);
        if (constant)
            return abc::Abc_NtkCreateNodeConst1(ntk);
        else
            return abc::Abc_NtkCreateNodeConst0(ntk);
    }

    void ReplaceNode(Node old_node, Node new_node) {
        abc::Abc_ObjReplace(old_node, new_node);
    }

    int SopSimulate(Node node) {
        return abc::Abc_ObjSopSimulate(node);
    }

    void PrintMFFC(Node node) {
        //Abc_NodeMffcConeSuppPrint
        using namespace abc;
        Vec_Ptr_t *vCone, *vSupp;
        Abc_Obj_t *pObj;
        int i;
        vCone = Vec_PtrAlloc(100);
        vSupp = Vec_PtrAlloc(100);
        Abc_NodeDeref_rec(node);
        Abc_NodeMffcConeSupp(node, vCone, vSupp);
        Abc_NodeRef_rec(node);
        printf("Node = %6s : Supp = %3d  Cone = %3d  (",
               Abc_ObjName(node), Vec_PtrSize(vSupp), Vec_PtrSize(vCone));
        Vec_PtrForEachEntry(Abc_Obj_t *, vCone, pObj, i)printf(" %s", Abc_ObjName(pObj));
        printf(" )\n");
        Vec_PtrFree(vCone);
        Vec_PtrFree(vSupp);
    }

    std::vector<Node> GetMFFCNodes(Node node) {
        std::vector<Node> mffc;
        abc::Vec_Ptr_t *vCone, *vSupp;
        Node pObj;
        int i;
        vCone = abc::Vec_PtrAlloc(100);
        vSupp = abc::Vec_PtrAlloc(100);
        abc::Abc_NodeDeref_rec(node);
        Abc_NodeMffcConeSupp(node, vCone, vSupp);
        abc::Abc_NodeRef_rec(node);
        Vec_PtrForEachEntry(Node, vCone, pObj, i) {
            mffc.emplace_back(pObj);
        }
        printf(" )\n");
        Vec_PtrFree(vCone);
        Vec_PtrFree(vSupp);
        return mffc;
    }

    Network CreateMFFCNetwork(Node node) {
        return abc::ECTL_Abc_NtkCreateMffc(GetHostNetwork(node), node, abc::Abc_ObjName(node));
    }

}
