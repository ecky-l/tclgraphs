
#include "graph.h"


/*
 * Graph API
 */

static const char* graphSubCommands[] = {
    "new",
    "create",
    "delete",
    "configure",
    "cget",
    "nodes",
    "subgraphs",
    "info",
    NULL
};

enum graphCommandIndex {
    GraphNewIx,
    GraphCreateIx,
    GraphDeleteIx,
    GraphConfigureIx,
    GraphCgetIx,
    GraphNodesIx,
    GraphSubgraphsIx,
    GraphInfoIx
};


static int
GraphNodesGetNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    static char* opts[] = { "-labels", "-notlabels", "-all", NULL };
    int optIdx = LABELS_ALL_IX;
    if (objc > 0) {
        if (Tcl_GetIndexFromObj(interp, objv[0], opts, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
    }

    return Graphs_GetNodes(graphPtr->nodes, optIdx, interp, objc-1, objv+1);
}

static int
GraphNodesAddNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    /* Validation first. No node is added unless all nodes are valid */
    for (int j = 0; j < objc; j++) {
        const char* nName = Tcl_GetString(objv[j]);
        Node* nodePtr = Graphs_ValidateNodeCommand(graphPtr->statePtr, interp, nName);
        if (nodePtr == NULL) {
            return TCL_ERROR;
        }
    }
    for (int j = 0; j < objc; j++) {
        const char* nName = Tcl_GetString(objv[j]);
        Node* nodePtr = Graphs_ValidateNodeCommand(graphPtr->statePtr, interp, nName);
        Graphs_AddNodeToGraph(graphPtr, nodePtr);
    }

    return TCL_OK;
}

static int
GraphNodesDeleteNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    return TCL_OK;
}

static int
GraphCmdNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;
    static const char* subCmds[] = { "add", "+", "delete", "-", "get", NULL };
    enum subCmdIdx {
        GraphAddNodes1Ix,
        GraphAddNodes2Ix,
        GraphDeleteNodes1Ix,
        GraphDeleteNodes2Ix,
        GraphGetNodesIx
    };

    if (objc < 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], subCmds, "option", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case GraphAddNodes1Ix:
    case GraphAddNodes2Ix: {
        return GraphNodesAddNodes(graphPtr, interp, objc-1, objv+1);
    }
    case GraphDeleteNodes1Ix:
    case GraphDeleteNodes2Ix: {
        return GraphNodesDeleteNodes(graphPtr, interp, objc-1, objv+1);
    }
    case GraphGetNodesIx:
    default: {
        break;
    }

    }

    return GraphNodesGetNodes(graphPtr, interp, objc-1, objv+1);
}

static int
GraphCmdConfigure(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int i, optIdx;
    static const char* opts[] = { "-name", NULL };
    enum OptsIx { NameIx };

    if (objc < 2 || objc > 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }

    for (i = 0; i < objc; i += 2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], opts, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        switch (optIdx) {
        case NameIx: {
            sprintf(graphPtr->name, "%s", Tcl_GetString(objv[i+1]));
            break;
        }
        default: {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong options in graph configure", -1));
            return TCL_ERROR;
        }
        }
    }

    return TCL_OK;
}

static int
GraphCmdCget(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx;
    char* opts[] = { "-name", NULL };
    enum OptsIx { NameIx };

    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], opts, "option", 1, &optIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (optIdx) {
    case NameIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(graphPtr->name, -1));
        return TCL_OK;
    }
    default: {
        break;
    }
    }
    return TCL_ERROR;
}

static int
GraphCmdDelete(Graph* graphPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    int optIdx;
    static const char *opts[] = { "-nodes", NULL };
    enum OptsIx { NodexIx };

    if (objc > 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "?-nodes?");
        return TCL_ERROR;
    }

    if (objc == 1) {
        if (Tcl_GetIndexFromObj(interp, objv[0], opts, "option", 1, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        Tcl_HashEntry* entry;
        Tcl_HashSearch search;
        entry = Tcl_FirstHashEntry(&graphPtr->nodes, &search);
        while (entry != NULL) {
            Node* nodePtr = Tcl_GetHashValue(entry);
            Graphs_DeleteNode(nodePtr, interp);
            entry = Tcl_FirstHashEntry(&graphPtr->nodes, &search);
        }
    }

    Tcl_DeleteCommandFromToken(interp, graphPtr->commandTkn);
    return TCL_OK;
}

static int
GraphInfoEdges(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    return TCL_OK;
}

static int
GraphInfoDelta(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    static const char* opts[] = { "+", "-", "+-", NULL };
    enum optsIdx { DeltaPlusIx, DeltaMinusIx, DeltaIx };
    int optIdx = DeltaIx;
    Tcl_HashSearch search;

    if (objc > 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "?+ | - | +-?");
        return TCL_ERROR;
    }
    if (objc == 1 && Tcl_GetIndexFromObj(interp, objv[0], opts, "option", TCL_EXACT, &optIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&graphPtr->nodes, &search);
    Tcl_Obj * resultObj = Tcl_NewObj();
    while (entry != NULL) {
        Node* nodePtr = Tcl_GetHashKey(&graphPtr->nodes, entry);
        switch (optIdx) {
        case DeltaPlusIx: {
            if (Graphs_AppendDeltaToObj(nodePtr->outgoing, graphPtr, EDGE_DIRECTED, interp, &resultObj) != TCL_OK) {
                return TCL_ERROR;
            }
            break;
        }
        case DeltaMinusIx: {
            if (Graphs_AppendDeltaToObj(nodePtr->incoming, graphPtr, EDGE_DIRECTED, interp, &resultObj) != TCL_OK) {
                return TCL_ERROR;
            }
            break;
        }
        case DeltaIx:
        default: {
            if (Graphs_AppendDeltaToObj(nodePtr->outgoing, graphPtr, EDGE_UNDIRECTED, interp, &resultObj) != TCL_OK) {
                return TCL_ERROR;
            }
            if (Graphs_AppendDeltaToObj(nodePtr->incoming, graphPtr, EDGE_UNDIRECTED, interp, &resultObj) != TCL_OK) {
                return TCL_ERROR;
            }
            break;
        }
        }
        entry = Tcl_NextHashEntry(&search);
    }

    Tcl_SetObjResult(interp, resultObj);
    return TCL_OK;
}

static int
GraphCmdInfo(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;
    static const char* subCmds[] = {
                                  "nodes",
                                  "edges",
                                  "delta",
                                  "subgraphs",
                                  NULL
    };
    enum cmdIndx {
        InfoNodesIx,
        InfoEdgesIx,
        InfoDeltaIx,
        InfoSubgraphsIx
    };

    if (objc < 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], subCmds, "option", 1, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case InfoNodesIx: {
        return GraphNodesGetNodes(graphPtr, interp, objc-1, objv+1);
    }
    case InfoEdgesIx: {
        return GraphInfoEdges(graphPtr, interp, objc-1, objv+1);
    }
    case InfoDeltaIx: {
        return GraphInfoDelta(graphPtr,interp,objc-1, objv+1);
    }

    case InfoSubgraphsIx: {

    }
    }


    return TCL_OK;
}

static int
GraphSubCmd(Graph* graphPtr, Tcl_Obj* cmd, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;

    if (Tcl_GetIndexFromObj(interp, cmd, graphSubCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case GraphNewIx:
    case GraphCreateIx: {
        return TCL_ERROR;
    }
    case GraphDeleteIx: {
        return GraphCmdDelete(graphPtr, interp, objc, objv);
    }
    case GraphConfigureIx: {
        return GraphCmdConfigure(graphPtr, interp, objc, objv);
    }
    case GraphCgetIx: {
        return GraphCmdCget(graphPtr, interp, objc, objv);
    }
    case GraphNodesIx: {
        return GraphCmdNodes(graphPtr, interp, objc, objv);
    }
    case GraphSubgraphsIx: {
        break;
    }
    case GraphInfoIx: {
        return GraphCmdInfo(graphPtr, interp, objc, objv);
    }
    default: {
        break;
    }
    }

    return TCL_ERROR;
}

static int
Graph_GraphSubCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Graph* graphPtr = (Graph*)clientData;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    return GraphSubCmd(graphPtr, objv[1], interp, objc-2, objv+2);
}

static void
GraphDeleteCmd(ClientData clientData)
{
    Graph* g = (Graph*)clientData;

    Tcl_HashEntry* entryPtr = Tcl_FindHashEntry(&g->statePtr->graphs, g->cmdName);
    Tcl_DeleteHashEntry(entryPtr);

    /* free the nodes and edges */
    Tcl_DeleteHashTable(&g->nodes);
    Tcl_Free((char*)g);
}

int
Graph_GraphCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    GraphState* gState = (GraphState*)clientData;
    Graph* graphPtr;
    Tcl_HashEntry* entryPtr;
    int new;
    int cmdIdx;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], graphSubCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case GraphNewIx:
    case GraphCreateIx: {
        int paramOffset;

        graphPtr = (Graph*) Tcl_Alloc(sizeof(Graph));
        graphPtr->statePtr = gState;
        sprintf(graphPtr->name, "%s", "");

        if (Tcl_StringMatch(Tcl_GetString(objv[1]), "new")) {
            sprintf(graphPtr->cmdName, "::tclgraphs::Graph%d", gState->graphUid);
            gState->graphUid++;
            paramOffset = 2;
        } else {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 0, objv, "<name>");
                return TCL_ERROR;
            }
            const char* cmdName = Tcl_GetString(objv[2]);
            if (Graphs_CheckCommandExists(interp, cmdName)) {
                Tcl_Free((char*) graphPtr);
                return TCL_ERROR;
            }
            sprintf(graphPtr->cmdName, cmdName);
            paramOffset = 3;
        }

        Tcl_InitHashTable(&graphPtr->nodes, TCL_ONE_WORD_KEYS);
        entryPtr = Tcl_CreateHashEntry(&gState->graphs, graphPtr->cmdName, &new);
        Tcl_SetHashValue(entryPtr, (ClientData)graphPtr);
        if (objc > paramOffset) {
            if (GraphCmdConfigure(graphPtr, interp, objc - paramOffset, objv + paramOffset) != TCL_OK) {
                GraphDeleteCmd(graphPtr);
                return TCL_ERROR;
            }
        }

        graphPtr->commandTkn = Tcl_CreateObjCommand(interp, graphPtr->cmdName, Graph_GraphSubCmd, graphPtr, GraphDeleteCmd);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(graphPtr->cmdName, -1));
        return TCL_OK;
    }
    default: {
        break;
    }
    }

    if (objc < 3) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }
    graphPtr = Graphs_ValidateGraphCommand(gState, interp, Tcl_GetString(objv[2]));
    if (graphPtr == NULL) {
        return TCL_ERROR;
    }
    return GraphSubCmd(graphPtr, objv[1], interp, objc-3, objv+3);
}

void
Graph_CleanupCmd(ClientData data)
{

}
