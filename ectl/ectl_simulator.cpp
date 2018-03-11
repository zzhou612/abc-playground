#include "ectl_simulator.h"

namespace ECTL {
    void ComSimSopOnce(Network pNtk) {
        ObjVector vNodes;
        Object pObj;
        int i;
        assert(Abc_NtkIsSopLogic(pNtk));
        srand((unsigned) time(nullptr));

        // initialize PI values
        Abc_NtkForEachPi(pNtk, pObj, i) {
            pObj->pCopy = (Object) (abc::ABC_PTRUINT_T) (rand() & 1);
        }

        // get the topological sequence of internal nodes
        vNodes = Abc_NtkDfs(pNtk, 0);

        // simulate internal nodes
        Vec_PtrForEachEntry(Object, vNodes, pObj, i) {
            pObj->pCopy = (Object) (abc::ABC_PTRUINT_T) abc::Abc_ObjSopSimulate(pObj);
        }

        // bring the results to the POs
        Abc_NtkForEachPo(pNtk, pObj, i) {
            pObj->pCopy = Abc_ObjFanin0(pObj)->pCopy;
        }

        Vec_PtrFree(vNodes);
    }

    void PrintSimRes(Network pNtk) {
        Object pObj;
        int i;
        // PI values
        abc::Abc_Print(1, "Primary Inputs:\n");
        Abc_NtkForEachPi(pNtk, pObj, i) {
            abc::Abc_Print(1, "%s=%d ", Abc_ObjName(pObj), (abc::ABC_PTRUINT_T) (pObj->pCopy));
        }
        abc::Abc_Print(1, "\n");

        // internal values
        abc::Abc_Print(1, "Internal Nodes (Including POs):\n");
        Abc_NtkForEachNode(pNtk, pObj, i) {
                abc::Abc_Print(1, "%s=%d ", Abc_ObjName(pObj), (abc::ABC_PTRUINT_T) (pObj->pCopy));
            }
        abc::Abc_Print(1, "\n");

        // PO values
        abc::Abc_Print(1, "Primary Outputs:\n");
        Abc_NtkForEachPo(pNtk, pObj, i) {
            abc::Abc_Print(1, "%s=%d ", Abc_ObjName(pObj), (abc::ABC_PTRUINT_T) (pObj->pCopy));
        }
        abc::Abc_Print(1, "\n");

    }
}
