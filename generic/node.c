
#include "graph.h"

/*
 * Node API
 */

static Edge*
ValidateEdgeCmd(GraphState* statePtr, Tcl_Interp* interp, const char* eName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&statePtr->edges, eName);
    if (entry == NULL) {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "No such edge: ", eName, NULL);
        Tcl_SetObjResult(interp, res);
        return NULL;
    }
    return ((Edge*)Tcl_GetHashValue(entry));
}

#define VALIDATE_DELETE_ADD_EDGES(WHAT, FCN) \
            Tcl_HashEntry* entry;\
            Tcl_HashSearch search;\
            int j, loLen;\
            Tcl_Obj** edgesList;\
            /* check whether all given nodes are valid */\
            if (Tcl_ListObjGetElements(interp, objv[i+1], &loLen, &edgesList) != TCL_OK) {\
                return TCL_ERROR;\
            }\
            for (j = 0; j < loLen; j++) {\
                char* nName = Tcl_GetString(edgesList[j]);\
                if (ValidateEdgeCmd(nodePtr->statePtr, interp, nName) != TCL_OK) {\
                    return TCL_ERROR;\
                }\
            }\
            /* remove existing nodes, to set new ones. Do not delete them. */\
            entry = Tcl_FirstHashEntry(&nodePtr->WHAT, &search);\
            while (entry != NULL) {\
                Edge* e = Tcl_GetHashValue(entry);\
                e->toNode = NULL;\
                Tcl_DeleteHashEntry(entry);\
            }\
            return FCN(nodePtr, interp, loLen, edgesList);


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
            nodePtr->name = Tcl_GetString(objv[i+1]);
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
    Tcl_SetObjResult(interp, Tcl_NewStringObj((nodePtr->name == NULL ? "" : nodePtr->name), -1));
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
    char *subCmds[] = {
        "neigbours", "tags", "configure", "cget", "delete", NULL
    };
    enum BlobIx {
        NeighboursIx, TagsIx, ConfigureIx, CgetIx, DeleteIx
    };

    if (objc == 1 || objc > 4) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
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
    Tcl_DeleteHashTable(&nodePtr->edges);
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
    if (Tcl_StringMatch(Tcl_GetString(objv[1]), "new")) {
        sprintf(nodePtr->cmdName, "::tclgraphs::Node%d", gState->nodeUid);
    } else {
        sprintf(nodePtr->cmdName, Tcl_GetString(objv[1]));
    }

    if ((objc > 2) && NodeCmdConfigure(nodePtr, interp, objc-2, objv+2) != TCL_OK) {
        return TCL_ERROR;
    }
    gState->nodeUid++;

    Tcl_InitHashTable(&nodePtr->edges, TCL_ONE_WORD_KEYS);

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
