
#include "graph.h"
#include <string.h>

/*
 * Node API
 */

static int
NodeCmdDelFromGraphs(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    for (int i = 0; i < objc; i++) {
        Graph* graphPtr = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, Tcl_GetString(objv[i]));
        if (graphPtr == NULL) {
            return TCL_ERROR;
        }
    }
    for (int i = 0; i < objc; i++) {
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

static int
NodeCmdAddToGraphs(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    for (int i = 0; i < objc; i++) {
        Graph* graphPtr = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, Tcl_GetString(objv[i]));
        if (graphPtr == NULL) {
            return TCL_ERROR;
        }
    }
    for (int i = 0; i < objc; i++) {
        Graph* graphPtr = Graphs_ValidateGraphCommand(nodePtr->statePtr, interp, Tcl_GetString(objv[i]));
        Graphs_AddNodeToGraph(graphPtr, nodePtr);
    }
    return TCL_OK;
}

static int
NodeCmdTags(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
NodeCmdNeighbours(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
NodeCmdConfigure(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    int i, optIdx;
    char* opts[] = {
        "-name", NULL
    };
    enum OptsIx {
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
            sprintf(nodePtr->name, "%s", Tcl_GetString(objv[i+1]));
            return TCL_OK;
        }
        default: {
            break;
        }
        }
    }

    return TCL_ERROR;
}

static int
NodeCmdCget(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->name, -1));
    return TCL_OK;
}

/*
 * The delete subcommand, calls the command delete procedure.
 */
static int
NodeCmdDelete(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_DeleteCommandFromToken(interp, nodePtr->commandTkn);
    return TCL_OK;
}

static int
NodeCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    int cmdIdx;

    Node* nodePtr = (Node*) clientData;
    static char *subCmds[] = {
        "addtographs",
        "delfromgraphs",
        "->",
        "<-",
        "-",
        "neigbours",
        "tags",
        "configure",
        "cget",
        "delete",
        NULL
    };
    enum BlobIx {
        AddToGraphIx,
        DelFromGraphIx,
        AddOutNeighbourIx,
        AddInNeighbourIx,
        AddUndirNeighbourIx,
        NeighboursIx,
        TagsIx,
        ConfigureIx,
        CgetIx,
        DeleteIx
    };

    if (objc == 1 || objc > 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case AddToGraphIx: {
        return NodeCmdAddToGraphs(nodePtr, interp, objc-2, objv+2);
    }
    case DelFromGraphIx: {
        return NodeCmdDelFromGraphs(nodePtr, interp, objc-2, objv+2);
    }
    case AddOutNeighbourIx: {
        Tcl_SetStringObj(objv[1], "-out", -1);
        return NodeCmdNeighbours(nodePtr, interp, objc-1, objv+1);
    }
    case AddInNeighbourIx: {
        Tcl_SetStringObj(objv[1], "-in", -1);
        return NodeCmdNeighbours(nodePtr, interp, objc-1, objv+1);
    }
    case AddUndirNeighbourIx: {
        Tcl_SetStringObj(objv[1], "-u", -1);
        return NodeCmdNeighbours(nodePtr, interp, objc-1, objv+1);
    }
    case NeighboursIx: {
        return NodeCmdNeighbours(nodePtr, interp, objc-2, objv+2);
    }
    case TagsIx: {
        return NodeCmdTags(nodePtr, interp, objc-2, objv+2);
    }
    case ConfigureIx: {
        return NodeCmdConfigure(nodePtr, interp, objc-2, objv+2);
    }
    case CgetIx: {
        return NodeCmdCget(nodePtr, interp, objc-2, objv+2);
    }
    case DeleteIx: {
        return NodeCmdDelete(nodePtr, interp, objc-2, objv+2);
    }
    default: {
        break;
    }
    }

    return TCL_ERROR;
}

/*
 * Command Delete callback
 */
static void
NodeDeleteCmd(ClientData clientData)
{
    Node* nodePtr = (Node*)clientData;
    Tcl_HashEntry* entry;
    Tcl_HashSearch search;

    entry = Tcl_FirstHashEntry(&nodePtr->edges, &search);
    while (entry != NULL) {
        Edge* edgePtr = (Edge*)Tcl_GetHashValue(entry);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        entry = Tcl_FirstHashEntry(&nodePtr->edges, &search);
    }

    /* remove myself from the graphs where I am part of */
    entry = Tcl_FirstHashEntry(&nodePtr->graphs, &search);
    while (entry != NULL) {
        Tcl_HashEntry* gEntry;
        Graph* graphPtr = (Graph*)Tcl_GetHashValue(entry);
        gEntry = Tcl_FindHashEntry(&graphPtr->nodes, nodePtr->cmdName);
        if (gEntry != NULL) {
            Tcl_DeleteHashEntry(gEntry);
        }
        Tcl_DeleteHashEntry(entry);
        entry = Tcl_FirstHashEntry(&nodePtr->graphs, &search);
    }

    Tcl_DeleteHashTable(&nodePtr->edges);
    Tcl_DeleteHashTable(&nodePtr->graphs);

    entry = Tcl_FindHashEntry(&nodePtr->statePtr->nodes, nodePtr->cmdName);
    Tcl_DeleteHashEntry(entry);

    Tcl_Free((char*)nodePtr);
}

int
Node_CreateCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    GraphState* gState = (GraphState*)clientData;
    int new;
    Node* nodePtr;
    Tcl_HashEntry* entryPtr;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "[new|cmdname] ?arg ...?");
        return TCL_ERROR;
    }

    nodePtr = (Node*)Tcl_Alloc(sizeof(Node));
    nodePtr->statePtr = gState;
    sprintf(nodePtr->name, "%s", "");

    if (Tcl_StringMatch(Tcl_GetString(objv[1]), "new")) {
        sprintf(nodePtr->cmdName, "::tclgraphs::Node%d", gState->nodeUid);
        gState->nodeUid++;
    } else {
        sprintf(nodePtr->cmdName, Tcl_GetString(objv[1]));
    }

    if (Graphs_CheckCommandExists(interp, nodePtr->cmdName)) {
        Tcl_Free((char*)nodePtr);
        return TCL_ERROR;
    }

    if ((objc > 2) && NodeCmdConfigure(nodePtr, interp, objc-2, objv+2) != TCL_OK) {
        Tcl_Free((char*)nodePtr);
        return TCL_ERROR;
    }

    Tcl_InitHashTable(&nodePtr->edges, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&nodePtr->graphs, TCL_STRING_KEYS);

    nodePtr->commandTkn = Tcl_CreateObjCommand(interp, nodePtr->cmdName, NodeCmd, nodePtr, NodeDeleteCmd);
    entryPtr = Tcl_CreateHashEntry(&gState->nodes, nodePtr->cmdName, &new);
    Tcl_SetHashValue(entryPtr, nodePtr);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->cmdName, -1));

    return TCL_OK;
}

void
Node_CleanupCmd(ClientData data)
{
}
