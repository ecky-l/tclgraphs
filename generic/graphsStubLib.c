#include "graphs.h"

#ifdef USE_TCL_STUBS
//const GraphsStubs *graphsStubsPtr;

const GraphsStubs *graphsStubsPtr = NULL;

DLLEXPORT CONST
char* Graphs_InitStubs(Tcl_Interp* interp, const char* version, int exact)
{
    const char* packageName = "graphs";
    const char* errMsg = NULL;
    GraphsStubs* stubsPtr = NULL;
    const char* actualVersion = tclStubsPtr->tcl_PkgRequireEx(interp, packageName, version, exact, &stubsPtr);
    if (actualVersion == NULL) {
        return NULL;
    }
    if (stubsPtr == NULL) {
        errMsg = "missing stub table pointer";
        tclStubsPtr->tcl_ResetResult(interp);
        tclStubsPtr->tcl_AppendResult(interp, "Error loading ", packageName,
            " (requested version ", version, ", actual version ",
            actualVersion, "): ", errMsg, NULL);
        return NULL;
    }
    graphsStubsPtr = stubsPtr;
    return actualVersion;
}
#endif