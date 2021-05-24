#include "graph.h"

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
DLLEXPORT int Graphs_Init(Tcl_Interp *interp)
{
    Tcl_Namespace *graphsNS = NULL;
    GraphState* gState;

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
        return TCL_ERROR;
    }
#endif //USE_TCL_STUBS

    gState = (GraphState*) Tcl_Alloc(sizeof(GraphState));
    gState->interp = interp;
    Tcl_InitHashTable(&gState->graphs, TCL_STRING_KEYS);
    Tcl_InitHashTable(&gState->nodes, TCL_STRING_KEYS);
    Tcl_InitHashTable(&gState->edges, TCL_STRING_KEYS);
    gState->graphUid = 0;
    gState->nodeUid = 0;
    gState->edgeUid = 0;

    if ((graphsNS = Tcl_CreateNamespace(interp, "::graphs", NULL, NULL)) == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Cannot create ::graphs namespace", -1));
        return TCL_ERROR;
    }

    Tcl_CreateObjCommand(interp, "::graphs::graph", (Tcl_ObjCmdProc *) Graph_GraphCmd, (ClientData) gState,
            (Tcl_CmdDeleteProc *) Graph_CleanupCmd);
    Tcl_CreateObjCommand(interp, "::graphs::node", (Tcl_ObjCmdProc *) Node_NodeCmd, (ClientData) gState,
            (Tcl_CmdDeleteProc *) Node_CleanupCmd);
    Tcl_CreateObjCommand(interp, "::graphs::edge", (Tcl_ObjCmdProc *) Edge_EdgeCmd, (ClientData) gState,
            (Tcl_CmdDeleteProc *) Edge_CleanupCmd);
    Tcl_Export(interp, graphsNS, "graph", 0);
    Tcl_Export(interp, graphsNS, "node", 0);
    Tcl_Export(interp, graphsNS, "edge", 0);

    if (Tcl_PkgProvide(interp, PACKAGE_NAME, PACKAGE_VERSION) != TCL_OK) {
        return TCL_ERROR;
    }

    return TCL_OK;
}
