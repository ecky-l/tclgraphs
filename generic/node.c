#include "graph.h"
#include <string.h>

/*
 * Node API
 */

typedef enum _EdgeDirection
{
    EDGE_DIRECTION_NONE,
    EDGE_DIRECTION_OUT,
    EDGE_DIRECTION_IN
} EdgeDirection;

static const char* nodeSubCommands[] = {
                                      "new",
                                      "create",
                                      "delete",
                                      "configure",
                                      "cget",
                                      "graphs",
                                      "neighbors",
                                      "labels",
                                      NULL
};

enum nodeCommandIndex {
    NodeNewIx,
    NodeCreateIx,
    NodeDeleteIx,
    NodeConfigureIx,
    NodeCgetIx,
    NodeGraphsIx,
    NodeNeighborsIx,
    NodeLabelsIx
};


static int NodeCmdGraphs(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int cmdIdx;

    static char* subcmds[] = { "add", "+", "delete", "-", "get" };
    enum subCmdIdx { NodeToGraphAdd1, NodeToGraphAdd2, NodeToGraphDel1, NodeToGraphDel2, NodeToGraphGet };

    if (objc == 0) {
        /* List graphs */
        goto nodeCmdGraphsReturnGraphs;
    }

    if (Tcl_GetIndexFromObj(interp, objv[0], subcmds, "option", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case NodeToGraphAdd1:
    case NodeToGraphAdd2: {
        for (int i = 1; i < objc; i++) {
            Graph* graphPtr = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, Tcl_GetString(objv[i]));
            if (graphPtr == NULL) {
                return TCL_ERROR;
            }
        }
        for (int i = 1; i < objc; i++) {
            Graph* graphPtr = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, Tcl_GetString(objv[i]));
            Graphs_AddNodeToGraph(graphPtr, nodePtr);
        }
        return TCL_OK;
    }
    case NodeToGraphDel1:
    case NodeToGraphDel2: {
        for (int i = 1; i < objc; i++) {
            Graph* graphPtr = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, Tcl_GetString(objv[i]));
            if (graphPtr == NULL) {
                return TCL_ERROR;
            }
        }
        for (int i = 1; i < objc; i++) {
            const char* gName = Tcl_GetString(objv[i]);
            Graph* graphPtr = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, gName);
            Tcl_HashEntry* entry = Tcl_FindHashEntry(&graphPtr->nodes, nodePtr->cmdName);
            if (entry != NULL) {
                Tcl_DeleteHashEntry(entry);
            }
            entry = Tcl_FindHashEntry(&nodePtr->graphs, gName);
            if (entry != NULL) {
                Tcl_DeleteHashEntry(entry);
            }
        }
        return TCL_OK;
    }
    case NodeToGraphGet:
    default: {
        break;
    }
    }

nodeCmdGraphsReturnGraphs:

    return TCL_OK;

}

static int NodeCmdLabel(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    return TCL_OK;
}

static int NodeCmdGetNeighbours(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    return TCL_OK;
}

static int NodeCmdConfigure(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int i, optIdx;
    char* opts[] = { "-name", NULL };
    enum OptsIx
    {
        NameIx
    };

    if (objc < 2 || objc > 8) {
        Tcl_WrongNumArgs(interp, 0, objv, "option ?arg ...?");
        return TCL_ERROR;
    }

    for (i = 0; i < objc; i += 2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], opts, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        switch (optIdx) {
        case NameIx: {
            sprintf(nodePtr->name, "%s", Tcl_GetString(objv[i + 1]));
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
    Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->name, -1));
    return TCL_OK;
}

/*
 * The delete subcommand, calls the command delete procedure.
 */
static int NodeCmdDelete(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Tcl_DeleteCommandFromToken(interp, nodePtr->commandTkn);
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
        Tcl_SetObjResult(interp, Tcl_NewStringObj("cannot create node from within a node! Use the [node] command!", -1));
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
    case NodeGraphsIx: {
        return NodeCmdGraphs(nodePtr, interp, objc, objv);
    }
    case NodeNeighborsIx: {
        return NodeCmdGetNeighbours(nodePtr, interp, objc, objv);
    }
    case NodeLabelsIx: {
        return NodeCmdLabel(nodePtr, interp, objc, objv);
    }
    default: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong subcommand to node object!", -1));
        return TCL_ERROR;
    }
    }

}

static int Node_NodeSubCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Node* nodePtr = (Node*)clientData;
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    return NodeSubCmd(nodePtr, objv[1], interp, objc-2, objv+2);
}

/*
 * Command Delete callback
 */
static void NodeDeleteCmd(ClientData clientData)
{
    Node* nodePtr = (Node*) clientData;
    Tcl_HashEntry* entry;
    Tcl_HashSearch search;

    entry = Tcl_FirstHashEntry(&nodePtr->neighbors, &search);
    while (entry != NULL) {
        Edge* edgePtr = (Edge*) Tcl_GetHashValue(entry);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        entry = Tcl_FirstHashEntry(&nodePtr->neighbors, &search);
    }

    /* remove myself from the graphs where I am part of */
    entry = Tcl_FirstHashEntry(&nodePtr->graphs, &search);
    while (entry != NULL) {
        Tcl_HashEntry* gEntry;
        Graph* graphPtr = (Graph*) Tcl_GetHashValue(entry);
        gEntry = Tcl_FindHashEntry(&graphPtr->nodes, nodePtr->cmdName);
        if (gEntry != NULL) {
            Tcl_DeleteHashEntry(gEntry);
        }
        Tcl_DeleteHashEntry(entry);
        entry = Tcl_FirstHashEntry(&nodePtr->graphs, &search);
    }

    Tcl_DeleteHashTable(&nodePtr->neighbors);
    Tcl_DeleteHashTable(&nodePtr->graphs);

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
            sprintf(nodePtr->cmdName, cmdName);
            paramOffset = 3;
        }

        Tcl_InitHashTable(&nodePtr->neighbors, TCL_ONE_WORD_KEYS);
        Tcl_InitHashTable(&nodePtr->graphs, TCL_STRING_KEYS);

        entryPtr = Tcl_CreateHashEntry(&gState->nodes, nodePtr->cmdName, &new);
        Tcl_SetHashValue(entryPtr, nodePtr);

        if (objc > paramOffset) {
            if (NodeCmdConfigure(nodePtr, interp, objc - paramOffset, objv + paramOffset) != TCL_OK) {
                NodeDeleteCmd((ClientData)nodePtr);
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
    return NodeSubCmd(nodePtr, objv[1], interp, objc-3, objv+3);
}

void Node_CleanupCmd(ClientData data)
{
}
