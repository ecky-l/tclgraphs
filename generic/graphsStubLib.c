#ifndef USE_TCL_STUBS
#define USE_TCL_STUBS
#endif

#include "graphs.h"

#undef Graphs_InitStubs

const GraphsStubs *graphsStubsPtr;

DLLEXPORT CONST
char* Graphs_InitStubs(Tcl_Interp* interp, const char* version, int exact)
{
    const char* packageName = "graphs";
    const char* errMsg = NULL;
    const char* actualVersion = tclStubsPtr->tcl_PkgRequireEx(interp, packageName, version, exact, (ClientData*)&graphsStubsPtr);
    
    if (actualVersion == NULL) {
        return NULL;
    }
    if (graphsStubsPtr == NULL) {
        Tcl_SetResult(interp, "This implementation of the graphs extension does not support stubs", TCL_STATIC);
        return NULL;
    }

    return actualVersion;
}

