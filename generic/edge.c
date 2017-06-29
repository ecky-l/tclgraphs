#include "graph.h"
#include <string.h>

/*
 * Edge API
 */

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

int
EdgeCmdConfigure(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int i, optIdx;
    char* opts[] = { "-name", "-from", "-to", "-weight", NULL };
    enum OptsIx
    {
        NameIx,
        FromIx,
        ToIx,
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
            return TCL_OK;
        }
        case FromIx: {
            Node* nodePtr = Graphs_ValidateNodeCommand(edgePtr->statePtr, interp, Tcl_GetString(objv[i + 1]));
            if (nodePtr == NULL) {
                return TCL_ERROR;
            }
            if (edgePtr->fromNode != nodePtr) {
                edgePtr->fromNode = nodePtr;
            }
            return TCL_OK;
        }
        case ToIx: {
            Node* nodePtr = Graphs_ValidateNodeCommand(edgePtr->statePtr, interp, Tcl_GetString(objv[i + 1]));
            if (nodePtr == NULL) {
                return TCL_ERROR;
            }
            if (edgePtr->toNode != nodePtr) {
                edgePtr->toNode = nodePtr;
            }
            return TCL_OK;
        }
        case WeightIx: {
            if (Tcl_GetDoubleFromObj(interp, objv[i + 1], &edgePtr->weight) != TCL_OK) {
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        default: {
            break;
        }

        }
    }

    return TCL_ERROR;
}

int
EdgeCmdDelete(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_DeleteCommandFromToken(interp, edgePtr->commandTkn);
    return TCL_OK;
}

int EdgeCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;

    Edge* edgePtr = (Edge*) clientData;
    char *subCmds[] = { "configure", "cget", "delete", NULL };
    enum BlobIx
    {
        ConfigureIx,
        CgetIx,
        DeleteIx
    };

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case ConfigureIx: {
        return EdgeCmdConfigure(edgePtr, interp, objc - 2, objv + 2);
    }
    case CgetIx: {
        return EdgeCmdCget(edgePtr, interp, objc - 2, objv + 2);
    }
    case DeleteIx: {
        return EdgeCmdDelete(edgePtr, interp, objc-2, objv+2);
    }
    default: {
        break;
    }
    }

    return TCL_ERROR;
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
    Node* fromNodePtr = NULL;
    Node* toNodePtr = NULL;
    Edge* edgePtr;
    Tcl_HashEntry* entryPtr;
    int new;

    if (objc < 6) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("-from and -to parameters are required to be valid nodes", -1));
        return TCL_ERROR;
    }

    for (int i = 2; i < objc; i++) {
        if (Tcl_StringMatch(Tcl_GetString(objv[i]), "-from")) {
            if ((i + 1) >= objc) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj("-from parameter not followed by a value", -1));
                return TCL_ERROR;
            }
            fromNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[i + 1]));
            if (fromNodePtr == NULL) {
                return TCL_ERROR;
            }
        }
        if (Tcl_StringMatch(Tcl_GetString(objv[i]), "-to")) {
            if ((i + 1) >= objc) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj("-to parameter not followed by a value", -1));
                return TCL_ERROR;
            }
            toNodePtr = Graphs_ValidateNodeCommand(gState, interp, Tcl_GetString(objv[i + 1]));
            if (toNodePtr == NULL) {
                return TCL_ERROR;
            }
        }
    }

    /* -from and -to must be given */
    if ((fromNodePtr == NULL) || (toNodePtr == NULL)) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj("-from and -to parameters are required to be valid nodes", -1));
        return TCL_ERROR;
    }

    entryPtr =  Tcl_CreateHashEntry(&fromNodePtr->neighbors, toNodePtr, &new);
    if (new) {
        edgePtr = Edge_CreateEdge(gState, fromNodePtr, toNodePtr, interp, Tcl_GetString(objv[1]), objc-2, objv+2);
        if (edgePtr == NULL) {
            Tcl_DeleteHashEntry(entryPtr);
            return TCL_ERROR;
        }
        Tcl_SetHashValue(entryPtr,edgePtr);
        return TCL_OK;
    }

    edgePtr = Tcl_GetHashValue(entryPtr);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->cmdName, -1));
    return TCL_OK;
}

Edge* Edge_CreateEdge(GraphState* gState, Node* fromNodePtr, Node* toNodePtr, Tcl_Interp* interp, const char* cmdName, int objc, Tcl_Obj* const objv[])
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
    if (objc > 0 && EdgeCmdConfigure(edgePtr, interp, objc, objv) != TCL_OK) {
        Tcl_Free((void*) edgePtr);
        return NULL;
    }
    edgePtr->fromNode = fromNodePtr;
    edgePtr->toNode = toNodePtr;

    edgePtr->commandTkn = Tcl_CreateObjCommand(interp, edgePtr->cmdName, EdgeCmd, edgePtr, EdgeDeleteCmd);
    entryPtr = Tcl_CreateHashEntry(&gState->edges, edgePtr->cmdName, &new);
    Tcl_SetHashValue(entryPtr, edgePtr);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->cmdName, -1));

    return edgePtr;
}

void Edge_CleanupCmd(ClientData data)
{

}

