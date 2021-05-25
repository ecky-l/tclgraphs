#include "graphs.h"

MODULE_SCOPE GraphsStubs graphsStubs;

/* Kept as a reference to be returned from depending C library packages */
static GraphState* graphState;

GraphState* Graphs_GetState(Tcl_Interp* interp)
{
    /*
    Tcl_CmdInfo cmdInfo;
    if (Tcl_GetCommandInfo(interp, "::graphs::graph", &cmdInfo) != TCL_OK) {
        return NULL;
    }
    return (GraphState*) cmdInfo.clientData;
    */
    return graphState;
}

/*
 *----------------------------------------------------------------------
 *
 * Combopt_Init --
 *
 *	Initialize the new package.  The string "Lpsolve" in the
 *	function name must match the PACKAGE declaration at the top of
 *	configure.ac.
 *
 * Results:
 *	A standard Tcl result
 *
 * Side effects:
 *	The Combopt package is created.
 *----------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
int Graphs_Init(Tcl_Interp *interp)
{
    Tcl_Namespace *graphsNS = NULL;

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
        return TCL_ERROR;
    }
#endif //USE_TCL_STUBS

    graphState = (GraphState*) Tcl_Alloc(sizeof(GraphState));
    graphState->interp = interp;
    Tcl_InitHashTable(&graphState->graphs, TCL_STRING_KEYS);
    Tcl_InitHashTable(&graphState->nodes, TCL_STRING_KEYS);
    Tcl_InitHashTable(&graphState->edges, TCL_STRING_KEYS);
    graphState->graphUid = 0;
    graphState->nodeUid = 0;
    graphState->edgeUid = 0;

    if ((graphsNS = Tcl_CreateNamespace(interp, "::graphs", NULL, NULL)) == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Cannot create ::graphs namespace", -1));
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "::graphs::graph", (Tcl_ObjCmdProc *) Graph_GraphCmd, (ClientData)graphState,
            (Tcl_CmdDeleteProc *) Graph_CleanupCmd);
    Tcl_CreateObjCommand(interp, "::graphs::node", (Tcl_ObjCmdProc *) Node_NodeCmd, (ClientData)graphState,
            (Tcl_CmdDeleteProc *) Node_CleanupCmd);
    Tcl_CreateObjCommand(interp, "::graphs::edge", (Tcl_ObjCmdProc *) Edge_EdgeCmd, (ClientData)graphState,
            (Tcl_CmdDeleteProc *) Edge_CleanupCmd);
    Tcl_Export(interp, graphsNS, "graph", 0);
    Tcl_Export(interp, graphsNS, "node", 0);
    Tcl_Export(interp, graphsNS, "edge", 0);

    if (Tcl_PkgProvideEx(interp, PACKAGE_NAME, PACKAGE_VERSION, &graphsStubs) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}
#ifdef __cplusplus
}
#endif  /* __cplusplus */
