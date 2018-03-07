#ifndef ABC_API_HPP
#define ABC_API_HPP

#define LIN64

#include <base/main/main.h>
#include <aig/gia/gia.h>
#include <misc/util/abc_global.h>
#include <opt/ret/retInt.h>

// procedures to get the ABC framework and execute commands in it
extern abc::Abc_Frame_t *abc::Abc_FrameGetGlobalFrame();

extern int abc::Cmd_CommandExecute(abc::Abc_Frame_t *pAbc, const char *sCommand);

// procedures to get the network
extern abc::Abc_Ntk_t *abc::Abc_FrameReadNtk(Abc_Frame_t *p);

// performs simulation
//extern void abc::Abc_NtkCycleInitStateSop(abc::Abc_Ntk_t *pNtk, int nFrames, int fVerbose);

//extern int abc::Abc_ObjSopSimulate(Abc_Obj_t *pObj);


#endif