#include "graphs.h"

/*
 * Graph API
 */
#define GRAPHS_MARKS_SET_HIDDEN(flags, newFlag) newFlag ? (flags | GRAPHS_MARK_HIDDEN) : (flags & ~GRAPHS_MARK_HIDDEN); 


static const char* GraphSubCommands[] = {
        "new",
        "create",
        "destroy",
        "configure",
        "cget",
        "nodes",
        "subgraphs",
        "info",
        "mark",
        NULL
};
enum GraphSubCommandIndex
{
    GraphNewIx,
    GraphCreateIx,
    GraphDestroyIx,
    GraphConfigureIx,
    GraphCgetIx,
    GraphNodesIx,
    GraphSubgraphsIx,
    GraphInfoIx,
    GraphMarkIx
};

static const char* GraphInfoOptions[] = {
        "nodes",
        "edges",
        "delta+",
        "deltaplus",
        "delta-",
        "deltaminus",
        "delta",
        "subgraphs",
        "order",
        NULL
};

enum GraphInfoOptionIndex
{
    GraphInfoNodesIx,
    GraphInfoEdgesIx,
    GraphInfoDeltaPlus1Ix,
    GraphInfoDeltaPlus2Ix,
    GraphInfoDeltaMinus1Ix,
    GraphInfoDeltaMinus2Ix,
    GraphInfoDeltaIx,
    GraphInfoSubgraphsIx,
    GraphInfoOrderIx
};

static const char* GraphMarks[] = { "hidden", NULL };
enum GraphMarksIndex
{
    GraphMarkHiddenIx
};

static const char* GraphInfoEdgesOptions[] = {
    "-marks",
    NULL
};
enum GraphInfoEdgesOptionIndex
{
    GraphInfoEdgesOptionMarksIx
};

static const char* LabelFilterOptions[] = { "-name", "-labels", "-notlabels", "-all", NULL };


static int GraphNodesGetNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx = LABELS_ALL_IDX;
    if (objc > 0) {
        if (Tcl_GetIndexFromObj(interp, objv[0], LabelFilterOptions, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        if (Graphs_CheckLabelsOptions(optIdx, interp, objc, objv) != TCL_OK) {
            return TCL_ERROR;
        }
    }

    return Graphs_GetNodes(graphPtr->nodes, optIdx, interp, objc - 1, objv + 1);
}

static int GraphNodesAddNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    /* Validation first. No node is added unless all nodes are valid */
    for (int j = 0; j < objc; j++) {
        const char* nName = Tcl_GetString(objv[j]);
        if (Graphs_NodeGetByCommand(graphPtr->statePtr, nName) == NULL) {
            Tcl_Obj* res = Tcl_NewStringObj("No such node: ", -1);
            Tcl_AppendObjToObj(res, objv[j]);
            Tcl_SetObjResult(interp, res);
            return TCL_ERROR;
        }
    }
    for (int j = 0; j < objc; j++) {
        const char* nName = Tcl_GetString(objv[j]);
        Node* nodePtr = Graphs_NodeGetByCommand(graphPtr->statePtr, nName);
        Graphs_AddNodeToGraph(graphPtr, nodePtr);
    }

    return TCL_OK;
}

static int GraphNodesDeleteNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    /* Validation first. No node is added unless all nodes are valid */
    for (int j = 0; j < objc; j++) {
        const char* nName = Tcl_GetString(objv[j]);
        if (Graphs_NodeGetByCommand(graphPtr->statePtr, nName) == NULL) {
            Tcl_Obj* res = Tcl_NewStringObj("No such node: ", -1);
            Tcl_AppendObjToObj(res, objv[j]);
            Tcl_SetObjResult(interp, res);
            return TCL_ERROR;
        }
    }
    for (int j = 0; j < objc; j++) {
        const char* nName = Tcl_GetString(objv[j]);
        Node* nodePtr = Graphs_NodeGetByCommand(graphPtr->statePtr, nName);
        Graphs_DeleteNodeFromGraph(graphPtr, nodePtr);
    }
    return TCL_OK;
}

static int GraphCmdNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;
    static const char* subCmds[] = { "add", "+", "delete", "-", "get",
    NULL };
    enum subCmdIdx
    {
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
        return GraphNodesAddNodes(graphPtr, interp, objc - 1, objv + 1);
    }
    case GraphDeleteNodes1Ix:
    case GraphDeleteNodes2Ix: {
        return GraphNodesDeleteNodes(graphPtr, interp, objc - 1, objv + 1);
    }
    case GraphGetNodesIx:
    default: {
        break;
    }

    }

    return GraphNodesGetNodes(graphPtr, interp, objc - 1, objv + 1);
}

static int GraphCmdConfigure(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int i, optIdx;
    static const char* opts[] = { "-name", NULL };
    enum OptsIx
    {
        NameIx
    };

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
            sprintf(graphPtr->name, "%s", Tcl_GetString(objv[i + 1]));
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

static int GraphCmdCget(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx;
    char* opts[] = { "-name",
    NULL };
    enum OptsIx
    {
        NameIx
    };

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

static int GraphCmdDestroy(Graph* graphPtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int optIdx;
    static const char *opts[] = { "-nodes",
    NULL };
    enum OptsIx
    {
        NodexIx
    };

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

static int GraphParseMarks(const char* marksSpec, int* marksMaskOut)
{
    unsigned int lclMarksMask = 0;
    for (int i = 0; i < strlen(marksSpec); i++) {
        char c = marksSpec[i];
        if (c == 'c' || c == 'h') {
            lclMarksMask |= GRAPHS_MARK_HIDDEN;
        }
        else if (c == 'C' || c == 'H') {
            lclMarksMask |= ~GRAPHS_MARK_HIDDEN;
        }
        else {
            return TCL_ERROR;
        }
    }
    *marksMaskOut = lclMarksMask;
    return TCL_OK;
}

static int GraphInfoEdges(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;
    unsigned edgeMarksMask = 0;
    Tcl_HashSearch search;
    Tcl_HashEntry* entry = NULL;
    Tcl_Obj* result = Tcl_NewObj();


    if (objc == 1 || objc > 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "?-marks <marks>?");
        return TCL_ERROR;
    }

    if (objc == 2) {
        if (Tcl_GetIndexFromObj(interp, objv[0], GraphInfoEdgesOptions, "-marks", 0, &cmdIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        if (GraphParseMarks(Tcl_GetString(objv[1]), &edgeMarksMask) != TCL_OK) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(
                "Wrong marks specifier. Should be a combination of c/h (cut/hidden) C/H (not cut/hidden).",
                -1));
            return TCL_ERROR;
        }
    }


    /* collect edges */
    entry = Tcl_FirstHashEntry(&graphPtr->edges, &search);
    while (entry != NULL) {
        Edge* edgePtr = (Edge*)Tcl_GetHashKey(&graphPtr->edges, entry);
        if (Edge_HasMarks(edgePtr, edgeMarksMask)) {
            Tcl_ListObjAppendElement(interp, result, Tcl_NewStringObj(edgePtr->cmdName, -1));
        }
        entry = Tcl_NextHashEntry(&search);
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int GraphInfoDelta(Graph* graphPtr, DeltaT deltaType, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx = LABELS_ALL_IDX;
    if (objc > 0) {
        if (Tcl_GetIndexFromObj(interp, objv[0], LabelFilterOptions, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
    }

    struct LabelFilter lblFilt;
    lblFilt.filterType = optIdx;
    lblFilt.objc = objc - 1;
    lblFilt.objv = (Tcl_Obj**) objv + 1;

    Tcl_HashSearch search;
    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&graphPtr->nodes, &search);
    Tcl_Obj * resultObj = Tcl_NewObj();
    while (entry != NULL) {
        Node* nodePtr = Tcl_GetHashKey(&graphPtr->nodes, entry);
        Graphs_GetDelta(nodePtr, graphPtr, deltaType, lblFilt, interp, &resultObj);
        entry = Tcl_NextHashEntry(&search);
    }

    Tcl_SetObjResult(interp, resultObj);
    return TCL_OK;
}

static int GraphCmdInfo(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;
    if (objc < 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], GraphInfoOptions, "option", TCL_EXACT, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case GraphInfoNodesIx: {
        return GraphNodesGetNodes(graphPtr, interp, objc - 1, objv + 1);
    }
    case GraphInfoEdgesIx: {
        return GraphInfoEdges(graphPtr, interp, objc - 1, objv + 1);
    }
    case GraphInfoDeltaPlus1Ix:
    case GraphInfoDeltaPlus2Ix: {
        return GraphInfoDelta(graphPtr, DELTA_PLUS, interp, objc - 1, objv + 1);
    }
    case GraphInfoDeltaMinus1Ix:
    case GraphInfoDeltaMinus2Ix: {
        return GraphInfoDelta(graphPtr, DELTA_MINUS, interp, objc - 1, objv + 1);
    }
    case GraphInfoDeltaIx: {
        return GraphInfoDelta(graphPtr, DELTA_ALL, interp, objc - 1, objv + 1);
    }
    case GraphInfoSubgraphsIx: {
        break;
    }
    case GraphInfoOrderIx: {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(graphPtr->order));
        return TCL_OK;
    }
    }

    return TCL_OK;
}

static int GraphCmdMark(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int markIndx;

    if (objc < 1 || objc > 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "mark <mark> ?true|false?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], GraphMarks, "mark", 0, &markIndx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (markIndx) {
    case GraphMarkHiddenIx: {
        unsigned hasMark;

        if (objc == 2) {
            unsigned int newMark = 0;
            if (Tcl_GetBooleanFromObj(interp, objv[1], &newMark) != TCL_OK) {
                return TCL_ERROR;
            }
            graphPtr->marks = GRAPHS_MARKS_SET_HIDDEN(graphPtr->marks, newMark);
        }

        hasMark = (graphPtr->marks & GRAPHS_MARK_HIDDEN);
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(hasMark));
        break;
    }
    default: {
        Tcl_Obj* result = Tcl_NewStringObj("Wrong mark: ", -1);
        Tcl_AppendObjToObj(result, objv[0]);
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }
    }

    return TCL_OK;
}

static int GraphSubCmd(Graph* graphPtr, Tcl_Obj* cmd, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;

    if (Tcl_GetIndexFromObj(interp, cmd, GraphSubCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case GraphNewIx:
    case GraphCreateIx:
        return TCL_ERROR;
    case GraphDestroyIx:
        return GraphCmdDestroy(graphPtr, interp, objc, objv);
    case GraphConfigureIx:
        return GraphCmdConfigure(graphPtr, interp, objc, objv);
    case GraphCgetIx:
        return GraphCmdCget(graphPtr, interp, objc, objv);
    case GraphNodesIx:
        return GraphCmdNodes(graphPtr, interp, objc, objv);
    case GraphSubgraphsIx:
        break;
    case GraphInfoIx:
        return GraphCmdInfo(graphPtr, interp, objc, objv);
    case GraphMarkIx:
        return GraphCmdMark(graphPtr, interp, objc, objv);
    default:
        break;
    }

    return TCL_ERROR;
}

static int Graph_GraphSubCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Graph* graphPtr = (Graph*) clientData;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    return GraphSubCmd(graphPtr, objv[1], interp, objc - 2, objv + 2);
}

static void GraphDestroyCmd(ClientData clientData)
{
    Graph* g = (Graph*) clientData;
    Tcl_HashEntry* entry1;
    Tcl_HashSearch search;

    /* Remove all nodes from the graph */
    entry1 = Tcl_FirstHashEntry(&g->nodes, &search);
    while (entry1 != NULL) {
        Node* nodePtr = Tcl_GetHashValue(entry1);
        nodePtr->graph = NULL;
        entry1 = Tcl_NextHashEntry(&search);
    }

    /* Delete the graph from global state */
    entry1 = Tcl_FindHashEntry(&g->statePtr->graphs, g->cmdName);
    Tcl_DeleteHashEntry(entry1);

    /* free the nodes and edges */
    Tcl_DeleteHashTable(&g->nodes);
    Tcl_Free((char*) g);
}

int Graph_GraphCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    GraphState* gState = (GraphState*) clientData;
    Graph* graphPtr;
    Tcl_HashEntry* entryPtr;
    int new;
    int cmdIdx;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], GraphSubCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case GraphNewIx:
    case GraphCreateIx: {
        int paramOffset;

        graphPtr = (Graph*) Tcl_Alloc(sizeof(Graph));
        graphPtr->statePtr = gState;
        sprintf(graphPtr->name, "%s", "");
        graphPtr->order = 0;
        graphPtr->marks = 0;
        graphPtr->data = NULL;

        if (Tcl_StringMatch(Tcl_GetString(objv[1]), "new")) {
            sprintf(graphPtr->cmdName, "::graphs::Graph%d", gState->graphUid);
            gState->graphUid++;
            paramOffset = 2;
        }
        else {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 0, objv, "<name>");
                return TCL_ERROR;
            }
            const char* cmdName = Tcl_GetString(objv[2]);
            if (Graphs_CheckCommandExists(interp, cmdName)) {
                Tcl_Free((char*) graphPtr);
                return TCL_ERROR;
            }
            sprintf(graphPtr->cmdName, "%s", cmdName);
            paramOffset = 3;
        }

        Tcl_InitHashTable(&graphPtr->nodes, TCL_ONE_WORD_KEYS);
        Tcl_InitHashTable(&graphPtr->edges, TCL_ONE_WORD_KEYS);
        entryPtr = Tcl_CreateHashEntry(&gState->graphs, graphPtr->cmdName, &new);
        Tcl_SetHashValue(entryPtr, (ClientData )graphPtr);
        if (objc > paramOffset) {
            if (GraphCmdConfigure(graphPtr, interp, objc - paramOffset, objv + paramOffset) != TCL_OK) {
                GraphDestroyCmd(graphPtr);
                return TCL_ERROR;
            }
        }

        graphPtr->commandTkn = Tcl_CreateObjCommand(interp, graphPtr->cmdName, Graph_GraphSubCmd, graphPtr,
                GraphDestroyCmd);
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
    graphPtr = Graphs_GraphGetByCommand(gState, Tcl_GetString(objv[2]));
    if (graphPtr == NULL) {
        Tcl_Obj* res = Tcl_NewStringObj("No such graph: ", -1);
        Tcl_AppendObjToObj(res, objv[2]);
        Tcl_SetObjResult(interp, res);
        return TCL_ERROR;
    }
    return GraphSubCmd(graphPtr, objv[1], interp, objc - 3, objv + 3);
}

void Graph_CleanupCmd(ClientData data)
{

}
