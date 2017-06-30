#include "graph.h"
#include <string.h>

/*
 * Edge API
 */

static char* edgeSubCmds[] = {
    "new",
    "create",
    "get",
    "configure",
    "cget",
    "delete",
    NULL
};
enum edgeSubCmdIndices
{
    NewIx,
    CreateIx,
    GetIx,
    ConfigureIx,
    CgetIx,
    DeleteIx
};


int EdgeCmdCget(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx;
    char* opts[] = { "-name", "-from", "-to", "-weight", NULL };
    enum OptsIx
    {
        NameIx,
        FromIx,
        ToIx,
        WeightIx
    };

    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 1, objv, "option arg");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], opts, "option", 0, &optIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (optIdx) {
    case NameIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->name, -1));
        return TCL_OK;
    }
    case FromIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->fromNode->cmdName, -1));
        return TCL_OK;
    }
    case ToIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->toNode->cmdName, -1));
        return TCL_OK;
    }
    case WeightIx: {
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(edgePtr->weight));
        return TCL_OK;
    }
    default: {
        break;
    }

    }
    return TCL_OK;
}

int EdgeCmdConfigure(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int i, optIdx;
    char* opts[] = { "-name", "-weight", NULL };
    enum OptsIx
    {
        NameIx,
        WeightIx
    };

    if (objc < 1 || objc > 10) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }

    for (i = 0; i < objc; i += 2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], opts, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        switch (optIdx) {
        case NameIx: {
            sprintf(edgePtr->name, "%s", Tcl_GetString(objv[i + 1]));
            break;
        }
        case WeightIx: {
            if (Tcl_GetDoubleFromObj(interp, objv[i + 1], &edgePtr->weight) != TCL_OK) {
                return TCL_ERROR;
            }
            break;
        }
        default: {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong options in edge configure", -1));
            return TCL_ERROR;
        }

        }
    }

    return TCL_OK;
}

int EdgeCmdDelete(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_DeleteCommandFromToken(interp, edgePtr->commandTkn);
    return TCL_OK;
}

static int
EdgeCmd(Edge* edgePtr, enum edgeSubCmdIndices cmdIdx, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    switch (cmdIdx) {
    case ConfigureIx: {
        return EdgeCmdConfigure(edgePtr, interp, objc, objv);
    }
    case CgetIx: {
        return EdgeCmdCget(edgePtr, interp, objc, objv);
    }
    case DeleteIx: {
        return EdgeCmdDelete(edgePtr, interp, objc, objv);
    }
    case NewIx:
    case CreateIx:
    case GetIx:
    default: {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "Wrong edge method ", edgeSubCmds[cmdIdx], " should be configure cget or delete", NULL);
        Tcl_SetObjResult(interp, res);
        return TCL_ERROR;
    }
    }
}

int
Edge_EdgeCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;

    Edge* edgePtr = (Edge*) clientData;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], edgeSubCmds, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    return EdgeCmd(edgePtr, cmdIdx, interp, objc-2, objv+2);
}

static void EdgeDeleteCmd(ClientData clientData)
{
    Edge* edgePtr = (Edge*) clientData;
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&edgePtr->fromNode->neighbors, edgePtr->toNode);
    if (entry != NULL) {
        Tcl_DeleteHashEntry(entry);
    }
    entry = Tcl_FindHashEntry(&edgePtr->toNode->neighbors, edgePtr->fromNode);
    if (entry != NULL) {
        Tcl_DeleteHashEntry(entry);
    }

    entry = Tcl_FindHashEntry(&edgePtr->statePtr->edges, edgePtr->cmdName);
    Tcl_DeleteHashEntry(entry);

    Tcl_Free((char*) edgePtr);
}

int Edge_CreateCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    GraphState* gState = (GraphState*) clientData;
    Edge* edgePtr = NULL;
    Node* fromNodePtr = NULL;
    Node* toNodePtr = NULL;
    int cmdIdx;
    int unDirected = 0;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "[new | create <name> | get] <node> [-> | -] <node> ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], edgeSubCmds, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    int pOffset = (cmdIdx == CreateIx) ? 1 : 0;
    if (objc < (4 + pOffset)) {
        Tcl_WrongNumArgs(interp, 1, objv, "[new | create <name> | get] <node> [-> | -] <node> ?arg...?");
        return TCL_ERROR;
    }

    const char * dir = Tcl_GetString(objv[3 + pOffset]);
    if (Tcl_StringMatch(dir, "->")) {
        fromNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[2 + pOffset]));
        toNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[4 + pOffset]));
    }
    else if (Tcl_StringMatch(dir, "<-")) {
        fromNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[4 + pOffset]));
        toNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[2 + pOffset]));
    }
    else if (Tcl_StringMatch(dir, "-")) {
        fromNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[2 + pOffset]));
        toNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[4 + pOffset]));
        unDirected = 1;
    }
    else {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "Not a valid direction: ", dir, "! Must be ->, <- or -", NULL);
        Tcl_SetObjResult(interp, res);
        return TCL_ERROR;
    }

    if ((fromNodePtr == NULL) || (toNodePtr == NULL)) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj("from and to parameters are required to be valid nodes", -1));
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case NewIx:
    case CreateIx: {
        edgePtr = Edge_CreateEdge(gState, fromNodePtr, toNodePtr, unDirected, interp,
            Tcl_GetString(objv[1 + pOffset]), objc - (5 + pOffset), objv + 5 + pOffset);
        if (edgePtr == NULL) {
            return TCL_ERROR;
        }
        break;
    }
    case GetIx: {
        if (objc != 4) {
            Tcl_WrongNumArgs(interp, 1, objv, "get <fromNode> <-> <toNode>");
            return TCL_ERROR;
        }
        edgePtr = Edge_GetEdge(gState, fromNodePtr, toNodePtr, unDirected, interp);
        if (edgePtr == NULL) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("", -1));
            return TCL_OK;
        }
        break;
    }

    case ConfigureIx:
    case CgetIx:
    case DeleteIx: {
        if (objc != 4) {
            Tcl_WrongNumArgs(interp, 2, objv, "<fromNode> <-> <toNode>");
            return TCL_ERROR;
        }
        edgePtr = Edge_GetEdge(gState, fromNodePtr, toNodePtr, unDirected, interp);
        if (edgePtr == NULL) {
            return TCL_ERROR;
        }
        return EdgeCmd(edgePtr, cmdIdx, interp, objc-5, objv+5);
    }

    default: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong subcommand for edge", -1));
        return TCL_ERROR;
    }
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->cmdName, -1));
    return TCL_OK;
}

Edge*
Edge_CreateEdge(GraphState* gState, Node* fromNodePtr, Node* toNodePtr, int unDirected, Tcl_Interp* interp,
    const char* cmdName, int objc, Tcl_Obj* const objv[])
{
    Tcl_HashEntry* entryPtr;
    int new;
    Edge* edgePtr;

    edgePtr = (Edge*) Tcl_Alloc(sizeof(Edge));
    edgePtr->statePtr = gState;

    if (Tcl_StringMatch(cmdName, "new")) {
        sprintf(edgePtr->cmdName, "::tclgraphs::Edge%d", gState->edgeUid);
        gState->edgeUid++;
    }
    else {
        sprintf(edgePtr->cmdName, cmdName);
    }

    if (Graphs_CheckCommandExists(interp, edgePtr->cmdName)) {
        Tcl_Free((char*) edgePtr);
        return NULL;
    }

    sprintf(edgePtr->name, "%s", "");
    edgePtr->weight = 0.;
    edgePtr->fromNode = fromNodePtr;
    edgePtr->toNode = toNodePtr;

    if (objc > 0 && EdgeCmdConfigure(edgePtr, interp, objc, objv) != TCL_OK) {
        Tcl_Free((char*) edgePtr);
        return NULL;
    }

    /* create neighbor and check if exists */
    {
        Tcl_HashEntry* entry1 = Tcl_CreateHashEntry(&fromNodePtr->neighbors, toNodePtr, &new);
        if (!new) {
            Tcl_Obj* result = Tcl_NewObj();
            Tcl_AppendStringsToObj(result, toNodePtr->cmdName, " is already neighbor of ",
                fromNodePtr->cmdName, NULL);
            Tcl_SetObjResult(interp, result);
            Tcl_Free((char*) edgePtr);
            return NULL;
        }
        Tcl_SetHashValue(entry1, edgePtr);

        if (unDirected) {
            Tcl_HashEntry* entry2 = Tcl_CreateHashEntry(&toNodePtr->neighbors, fromNodePtr, &new);
            if (!new) {
                Tcl_Obj* result = Tcl_NewObj();
                Tcl_AppendStringsToObj(result, fromNodePtr->cmdName, " is already neighbor of ",
                    toNodePtr->cmdName, NULL);
                Tcl_SetObjResult(interp, result);
                Tcl_DeleteHashEntry(entry1);
                Tcl_Free((char*) edgePtr);
                return NULL;
            }
            Tcl_SetHashValue(entry2, edgePtr);
        }
    }

    edgePtr->commandTkn = Tcl_CreateObjCommand(interp, edgePtr->cmdName, Edge_EdgeCmd, edgePtr, EdgeDeleteCmd);
    entryPtr = Tcl_CreateHashEntry(&gState->edges, edgePtr->cmdName, &new);
    Tcl_SetHashValue(entryPtr, edgePtr);
    return edgePtr;
}

Edge*
Edge_GetEdge(GraphState* gState, Node* fromNodePtr, Node* toNodePtr, int unDirected, Tcl_Interp* interp)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&fromNodePtr->neighbors, toNodePtr);
    if (entry == NULL) {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, fromNodePtr->cmdName, " is not a neighbor of ", toNodePtr->cmdName, NULL);
        Tcl_SetObjResult(interp, res);
        return NULL;
    }
    Edge *edgePtr1 = (Edge*) Tcl_GetHashValue(entry);

    if (unDirected) {
        entry = Tcl_FindHashEntry(&toNodePtr->neighbors, fromNodePtr);
        if (entry == NULL) {
            Tcl_Obj* res = Tcl_NewObj();
            Tcl_AppendStringsToObj(res, toNodePtr->cmdName, " is not a neighbor of ", fromNodePtr->cmdName,
                NULL);
            Tcl_SetObjResult(interp, res);
            return NULL;
        }

        Edge *edgePtr2 = (Edge*) Tcl_GetHashValue(entry);
        if (edgePtr1 != edgePtr2) {
            Tcl_Obj* res = Tcl_NewObj();
            Tcl_AppendStringsToObj(res, fromNodePtr->cmdName, " is not connected to ", toNodePtr->cmdName,
                " by an undirected edge", NULL);
            Tcl_SetObjResult(interp, res);
            return NULL;
        }
    }

    return edgePtr1;
}

void Edge_CleanupCmd(ClientData data)
{

}

