#include <iostream>
#include "abc_api.h"

using namespace abc;

void ComSimSopOnce(Abc_Ntk_t *pNtk);

void PrintSimRes(Abc_Ntk_t *pNtk);

void ComSimSopOnce(Abc_Ntk_t *pNtk) {
    Vec_Ptr_t *vNodes;
    Abc_Obj_t *pObj;
    int i;
    assert(Abc_NtkIsSopLogic(pNtk));
    srand((unsigned) time(NULL));

    // initialize the values
    Abc_NtkForEachPi(pNtk, pObj, i)pObj->pCopy = (Abc_Obj_t *) (ABC_PTRUINT_T) (rand() & 1);

    // get the topological sequence using DFS
    vNodes = Abc_NtkDfs(pNtk, 0);

    // simulate internal nodes
    Vec_PtrForEachEntry(Abc_Obj_t *, vNodes, pObj, i)pObj->pCopy = (Abc_Obj_t *) (ABC_PTRUINT_T) Abc_ObjSopSimulate(
                pObj);

    Vec_PtrFree(vNodes);
}


void PrintSimRes(Abc_Ntk_t *pNtk) {
    Abc_Obj_t *pObj;
    int i;
    // PI values
    Abc_Print(1, "Primary inputs:\n");
    Abc_NtkForEachPi(pNtk, pObj, i)Abc_Print(1, "%s(%d) ", Abc_ObjName(pObj), (ABC_PTRUINT_T) (pObj->pCopy));
    Abc_Print(1, "\n");
    // internal values
    Abc_Print(1, "Internal nodes:\n");
    Abc_NtkForEachNode(pNtk, pObj, i)Abc_Print(1, "%s(%d) ", Abc_ObjName(pObj), (ABC_PTRUINT_T) (pObj->pCopy));
    Abc_Print(1, "\n");
    // PO values
    Abc_Print(1, "Primary outputs:\n");
    Abc_NtkForEachPo(pNtk, pObj, i)Abc_Print(1, "%s(%d) ", Abc_ObjName(pObj), (ABC_PTRUINT_T) (pObj->pCopy));
    Abc_Print(1, "\n");
    // latch values
    if (Abc_NtkLatchNum(pNtk)) {
        Abc_Print(1, "Latchs:\n");
        Abc_NtkForEachLatch(pNtk, pObj, i)Abc_Print(1, "%s(%d) ", Abc_ObjName(pObj), (ABC_PTRUINT_T) (pObj->pCopy));
        Abc_Print(1, "\n");
    }
}

int main(int argc, char *argv[]) {
    int i;
    abc::Abc_Frame_t *pAbc;                // defined in <base/main/mainInt.h>
    abc::Abc_Ntk_t *pNtk;                // defined in <base/abc/abc.h>
    abc::Abc_Obj_t *pObj;                // defined in <base/abc/abc.h>
    char Command[1000];
    clock_t clkRead, clk;

    std::string pFileName = PROJECT_SOURCE_DIR;
    pFileName += "/benchmark/C17.blif";

    abc::Abc_Start();
    pAbc = abc::Abc_FrameGetGlobalFrame();
    // read the file
    sprintf(Command, "read %s", pFileName.c_str());
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    // get the network from the ABC framework
    pNtk = Abc_FrameReadNtk(pAbc);
    // simulate the network
    ComSimSopOnce(pNtk);
    PrintSimRes(pNtk);
    // balance, convert to aig
    sprintf(Command, "balance");
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    // write the result in blif
    sprintf(Command, "write_blif %s_aig", pFileName.c_str());
    if (Cmd_CommandExecute(pAbc, Command)) {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    abc::Abc_Stop();
    return 0;
}

