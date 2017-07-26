#include "graph.h"
#include <string.h>

/*
 * Node API
 */

static const char* LabelFilterOptions[] = { "-name", "-labels", "-notlabels", "-all", NULL };

static const char* NodeOptions[] = { "-name", "-graph", NULL };
enum NodeOptsIdx { NameIx, GraphIx };

typedef enum _EdgeDirection
{
    EDGE_DIRECTION_NONE,
    EDGE_DIRECTION_OUT,
    EDGE_DIRECTION_IN
} EdgeDirection;

static const char* nodeSubCommands[] =
{
        "new",
        "create",
        "delete",
        "configure",
        "cget",
        "info",
        "labels",
        NULL
};

enum nodeCommandIndex
{
    NodeNewIx,
    NodeCreateIx,
    NodeDeleteIx,
    NodeConfigureIx,
    NodeCgetIx,
    NodeInfoIx,
    NodeLabelsIx
};


static int NodeCmdConfigure(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int i, optIdx;

    if (objc < 2 || objc > 4 || (objc % 2) != 0) {
        Tcl_WrongNumArgs(interp, 0, objv, "option arg ?option arg ...?");
        return TCL_ERROR;
    }

    for (i = 0; i < objc; i += 2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], NodeOptions, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        switch (optIdx) {
        case NameIx: {
            sprintf(nodePtr->name, "%s", Tcl_GetString(objv[i + 1]));
            break;
        }
        case GraphIx: {
            Graph* g = NULL;
            if (Tcl_StringMatch(Tcl_GetString(objv[i+1]), "")) {
                goto addNodeToGraph;
            }
            g = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, Tcl_GetString(objv[i+1]));
            if (g == NULL) {
                return TCL_ERROR;
            }
addNodeToGraph:
            Graphs_AddNodeToGraph(g, nodePtr);
            break;
        }
        default: {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong options in node configure", -1));
            return TCL_ERROR;
        }
        }
    }

    return TCL_OK;
}

static int NodeCmdCget(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int optIdx;

    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[0], NodeOptions, "option", 0, &optIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (optIdx) {
    case NameIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->name, -1));
        return TCL_OK;
    }
    case GraphIx: {
        if (nodePtr->graph == NULL) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("",-1));
        } else {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->graph->cmdName, -1));
        }
        return TCL_OK;
    }
    default: {
        break;
    }
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong options in node configure", -1));
    return TCL_ERROR;
}

/*
 * The delete subcommand, calls the command delete procedure.
 */
static int NodeCmdDelete(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Tcl_DeleteCommandFromToken(interp, nodePtr->commandTkn);
    return TCL_OK;
}

static int NodeInfoDelta(Node* nodePtr, DeltaT deltaType, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
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

    Tcl_Obj* deltaList = Tcl_NewObj();
    struct LabelFilter lblFilt;
    lblFilt.filterType = optIdx;
    lblFilt.objc = objc - 1;
    lblFilt.objv = (Tcl_Obj**) objv + 1;
    if (Graphs_GetDelta(nodePtr, NULL, deltaType, lblFilt, interp, &deltaList) != TCL_OK) {
        return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, deltaList);
    return TCL_OK;
}

static int NodeCmdInfo(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int cmdIdx;
    static const char* subCmds[] = { "delta+", "deltaplus", "delta-", "deltaminus", "delta", "graph", "labels", NULL };
    enum cmdIndx
    {
        NodeInfoDeltaPlus1Ix,
        NodeInfoDeltaPlus2Ix,
        NodeInfoDeltaMinus1Ix,
        NodeInfoDeltaMinus2Ix,
        NodeInfoDeltaIx,
        NodeInfoGraphIx,
        NodeInfoLabelsIx
    };

    if (objc < 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], subCmds, "option", TCL_EXACT, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case NodeInfoDeltaPlus1Ix:
    case NodeInfoDeltaPlus2Ix: {
        return NodeInfoDelta(nodePtr, DELTA_PLUS, interp, objc - 1, objv + 1);
    }
    case NodeInfoDeltaMinus1Ix:
    case NodeInfoDeltaMinus2Ix: {
        return NodeInfoDelta(nodePtr, DELTA_MINUS, interp, objc - 1, objv + 1);
    }
    case NodeInfoDeltaIx: {
        return NodeInfoDelta(nodePtr, DELTA_ALL, interp, objc - 1, objv + 1);
    }
    case NodeInfoGraphIx: {
        if (nodePtr->graph == NULL) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("",-1));
        } else {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->graph->cmdName, -1));
        }
        return TCL_OK;
    }
    case NodeInfoLabelsIx: {
        if (objc > 1) {
            Tcl_WrongNumArgs(interp, 0, objv, "option");
            return TCL_ERROR;
        }
        return Graphs_LabelsCommand(nodePtr->labels, interp, objc - 1, objv + 1);
    }

    }

    return TCL_OK;
}

static int NodeSubCmd(Node* nodePtr, Tcl_Obj* cmd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int cmdIdx;

    if (Tcl_GetIndexFromObj(interp, cmd, nodeSubCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case NodeNewIx:
    case NodeCreateIx: {
        Tcl_SetObjResult(interp,
                Tcl_NewStringObj("cannot create node from within a node! Use the [node] command!", -1));
        return TCL_ERROR;
    }
    case NodeDeleteIx: {
        return NodeCmdDelete(nodePtr, interp, objc, objv);
    }
    case NodeConfigureIx: {
        return NodeCmdConfigure(nodePtr, interp, objc, objv);
    }
    case NodeCgetIx: {
        return NodeCmdCget(nodePtr, interp, objc, objv);
    }
    case NodeInfoIx: {
        return NodeCmdInfo(nodePtr, interp, objc, objv);
    }
    case NodeLabelsIx: {
        return Graphs_LabelsCommand(nodePtr->labels, interp, objc, objv);
    }
    default: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong subcommand to node object!", -1));
        return TCL_ERROR;
    }
    }

}

static int Node_NodeSubCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Node* nodePtr = (Node*) clientData;
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    return NodeSubCmd(nodePtr, objv[1], interp, objc - 2, objv + 2);
}

/*
 * Command Delete callback
 */
static void NodeDeleteCmd(ClientData clientData)
{
    Node* nodePtr = (Node*) clientData;
    Tcl_HashEntry* entry;
    Tcl_HashSearch search;

    /* delete outgoing edges (neighbors) */
    entry = Tcl_FirstHashEntry(&nodePtr->outgoing, &search);
    while (entry != NULL) {
        Edge* edgePtr = (Edge*) Tcl_GetHashValue(entry);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        entry = Tcl_FirstHashEntry(&nodePtr->outgoing, &search);
    }

    /* delete incoming edges */
    entry = Tcl_FirstHashEntry(&nodePtr->incoming, &search);
    while (entry != NULL) {
        Edge* edgePtr = (Edge*) Tcl_GetHashValue(entry);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        entry = Tcl_FirstHashEntry(&nodePtr->incoming, &search);
    }

    /* remove myself from the graphs where I am part of */
    if (nodePtr->graph != NULL) {
        Graphs_DeleteNodeFromGraph(nodePtr->graph, nodePtr);
    }

    Tcl_DeleteHashTable(&nodePtr->outgoing);
    Tcl_DeleteHashTable(&nodePtr->incoming);
    Tcl_DeleteHashTable(&nodePtr->labels);

    entry = Tcl_FindHashEntry(&nodePtr->statePtr->nodes, nodePtr->cmdName);
    Tcl_DeleteHashEntry(entry);

    Tcl_Free((char*) nodePtr);
}

int Node_NodeCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    GraphState* gState = (GraphState*) clientData;
    int new, cmdIdx;
    Node* nodePtr;
    Tcl_HashEntry* entryPtr;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], nodeSubCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case NodeNewIx:
    case NodeCreateIx: {
        int paramOffset;

        nodePtr = (Node*) Tcl_Alloc(sizeof(Node));
        nodePtr->statePtr = gState;
        nodePtr->graph = NULL;
        sprintf(nodePtr->name, "%s", "");

        if (Tcl_StringMatch(Tcl_GetString(objv[1]), "new")) {
            sprintf(nodePtr->cmdName, "::tclgraphs::Node%d", gState->nodeUid);
            gState->nodeUid++;
            paramOffset = 2;
        }
        else {
            if (objc < 3) {
                Tcl_WrongNumArgs(interp, 0, objv, "<name>");
                return TCL_ERROR;
            }
            const char* cmdName = Tcl_GetString(objv[2]);
            if (Graphs_CheckCommandExists(interp, cmdName)) {
                Tcl_Free((char*) nodePtr);
                return TCL_ERROR;
            }
            sprintf(nodePtr->cmdName, "%s", cmdName);
            paramOffset = 3;
        }

        Tcl_InitHashTable(&nodePtr->outgoing, TCL_ONE_WORD_KEYS);
        Tcl_InitHashTable(&nodePtr->incoming, TCL_ONE_WORD_KEYS);
        Tcl_InitHashTable(&nodePtr->labels, TCL_STRING_KEYS);

        entryPtr = Tcl_CreateHashEntry(&gState->nodes, nodePtr->cmdName, &new);
        Tcl_SetHashValue(entryPtr, nodePtr);

        if (objc > paramOffset) {
            if (NodeCmdConfigure(nodePtr, interp, objc - paramOffset, objv + paramOffset) != TCL_OK) {
                NodeDeleteCmd((ClientData) nodePtr);
                return TCL_ERROR;
            }
        }

        nodePtr->commandTkn = Tcl_CreateObjCommand(interp, nodePtr->cmdName, Node_NodeSubCmd, nodePtr, NodeDeleteCmd);
        Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->cmdName, -1));
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
    nodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[2]));
    if (nodePtr == NULL) {
        return TCL_ERROR;
    }
    return NodeSubCmd(nodePtr, objv[1], interp, objc - 3, objv + 3);
}

void Node_CleanupCmd(ClientData data)
{
}
