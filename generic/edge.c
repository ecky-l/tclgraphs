
#include "graph.h"
#include <string.h>

/*
 * Edge API
 */

int
EdgeCmdConfigure(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int i, optIdx;
    char* opts[] = {
        "-name", "-from", "-to", "-weight", NULL
    };
    enum OptsIx {
        NameIx, FromIx, ToIx, WeightIx
    };

    if (objc < 1 || objc > 8) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }

    for (i = 0; i < objc; i += 2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], opts, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        switch (optIdx) {
        case NameIx: {
            sprintf(edgePtr->name, "%s", Tcl_GetString(objv[i+1]));
            break;
        }
        case FromIx:
        case ToIx: {
            char* nName = Tcl_GetString(objv[i+1]);
            Tcl_HashEntry* entryPtr = Tcl_FindHashEntry(&edgePtr->statePtr->nodes, nName);
            if (entryPtr == NULL) {
                Tcl_Obj* res = Tcl_NewObj();
                Tcl_AppendStringsToObj(res, "No such node: ", nName, NULL);
                Tcl_SetObjResult(interp, res);
                return TCL_ERROR;
            }
            if (optIdx == FromIx) {
                edgePtr->fromNode = Tcl_GetHashValue(entryPtr);
            } else {
                edgePtr->toNode = Tcl_GetHashValue(entryPtr);
            }
            break;
        }
        case WeightIx: {
            if (Tcl_GetDoubleFromObj(interp, objv[i+1], &edgePtr->weight) != TCL_OK) {
                return TCL_ERROR;
            }
            break;
        }
        default: {
            break;
        }

        }
    }


    return TCL_OK;
}

int
EdgeCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;

    Edge* edgePtr = (Edge*) clientData;
    char *subCmds[] = {
        "configure", "cget", "delete", NULL
    };
    enum BlobIx {
        ConfigureIx, CgetIx, DeleteIx
    };

    if (objc == 1 || objc > 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }
    return TCL_OK;
}

static void
EdgeDeleteCmd(ClientData clientData)
{
    Edge* edgePtr = (Edge*)clientData;
    Tcl_HashEntry* entry;

    entry = Tcl_FindHashEntry(&edgePtr->fromNode->edges, edgePtr);
    if (entry != NULL) {
        Tcl_DeleteHashEntry(entry);
    }
    entry = Tcl_FindHashEntry(&edgePtr->toNode->edges, edgePtr);
    if (entry != NULL) {
        Tcl_DeleteHashEntry(entry);
    }

    ckfree(edgePtr->name);
    Tcl_Free((char*)edgePtr);

}

int
Edge_CreateCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    GraphState* gState = (GraphState*)clientData;
    Tcl_HashEntry* entryPtr;
    int new;
    Edge* edgePtr;

    if (objc < 6) {
        Tcl_WrongNumArgs(interp, 0, objv, "[new|cmdname] ?arg ...?");
        return TCL_ERROR;
    }

    edgePtr = (Edge*)Tcl_Alloc(sizeof(Edge));
    edgePtr->statePtr = gState;

    if (Tcl_StringMatch(Tcl_GetString(objv[1]), "new")) {
        sprintf(edgePtr->cmdName, "::tclgraphs::Edge%d", gState->edgeUid);
    } else {
        sprintf(edgePtr->cmdName, Tcl_GetString(objv[1]));
    }

    if (Graphs_CheckCommandExists(interp, edgePtr->cmdName)) {
        Tcl_Free((char*)edgePtr);
        return TCL_ERROR;
    }

    sprintf(edgePtr->name, "%s", "");
    edgePtr->fromNode = edgePtr->toNode = NULL;
    if (EdgeCmdConfigure(edgePtr, interp, objc-2, objv+2) != TCL_OK) {
        Tcl_Free((void*)edgePtr);
        return TCL_ERROR;
    }

    gState->edgeUid++;
    edgePtr->commandTkn = Tcl_CreateObjCommand(interp, edgePtr->cmdName, EdgeCmd, edgePtr, EdgeDeleteCmd);
    entryPtr = Tcl_CreateHashEntry(&gState->edges, edgePtr->cmdName, &new);
    Tcl_SetHashValue(entryPtr, edgePtr);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->cmdName, -1));

    return TCL_OK;
}

void
Edge_CleanupCmd(ClientData data)
{

}

