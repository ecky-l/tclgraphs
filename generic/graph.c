
#include "graph.h"


/*
 * Graph API
 */


static int
GraphCmdNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_HashSearch search;
    Tcl_HashEntry* entry;
    Tcl_Obj* result = Tcl_NewObj();
    int tagLen = 0;
    Tcl_Obj** tagsObj = NULL;

    if (objc > 0) {
        if (objc != 2 || !Tcl_StringMatch(Tcl_GetString(objv[0]), "-tags")) {
            Tcl_WrongNumArgs(interp, 0, objv, "?-tags ...?");
            return TCL_ERROR;
        }
        if (Tcl_ListObjGetElements(interp, objv[1], &tagLen, &tagsObj)) {
            return TCL_ERROR;
        }
    }

    entry = Tcl_FirstHashEntry(&graphPtr->nodes, &search);
    while (entry != NULL) {
        Node* np = Tcl_GetHashValue(entry);
        int appendRes = 1;

        if (tagLen > 0) {
            int i;
            appendRes = 0;
            for (i = 0; i < tagLen; i++) {
                Tcl_HashEntry* tagsEntry = Tcl_FindHashEntry(&np->tags, Tcl_GetString(tagsObj[i]));
                if (tagsEntry != NULL) {
                    appendRes = 1;
                    break;
                }
            }
        }

        if (appendRes == 1) {
            Tcl_ListObjAppendElement(interp, result, Tcl_NewStringObj(np->cmdName, -1));
        }

        entry = Tcl_NextHashEntry(&search);
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

static int
GraphCmdAddNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
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
GraphCmdRmNodes(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    return TCL_OK;
}

static int
GraphCmdConfigure(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int i, optIdx;
    char* opts[] = {
        "-name", "-nodes", NULL
    };
    enum OptsIx {
        NameIx, NodesIx
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
            sprintf(graphPtr->name, "%s", Tcl_GetString(objv[i+1]));
            break;
        }
        case NodesIx: {
            Tcl_HashEntry* entry;
            Tcl_HashSearch search;
            int j, loLen;
            Tcl_Obj** nodesList;

            /* check whether all given nodes are valid */
            if (Tcl_ListObjGetElements(interp, objv[i+1], &loLen, &nodesList) != TCL_OK) {
                return TCL_ERROR;
            }

            for (j = 0; j < loLen; j++) {
                char* nName = Tcl_GetString(nodesList[j]);
                if (Graphs_ValidateNodeCommand(graphPtr->statePtr, interp, nName) != TCL_OK) {
                    return TCL_ERROR;
                }

            }

            /* remove existing nodes, to set new ones. Do not delete them. */
            entry = Tcl_FirstHashEntry(&graphPtr->nodes, &search);
            while (entry != NULL) {
                Tcl_DeleteHashEntry(entry);
                entry = Tcl_FirstHashEntry(&graphPtr->nodes, &search);
            }

            /* assign new nodes to the graph */
            GraphCmdAddNodes(graphPtr, interp, loLen, nodesList);
            break;
        }
        default: {
            break;
        }
        }
    }

    return TCL_OK;
}

static int
GraphCmdCget(Graph* graphPtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx;
    char* opts[] = {
        "-name", "-nodes", NULL
    };
    enum OptsIx {
        NameIx, NodesIx
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
    case NodesIx: {
        return GraphCmdNodes(graphPtr, interp, objc-1, objv+1);
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
    const char *opts[] = {
        "-nodes", NULL
    };
    enum OptsIx {
        NodexIx
    };
    if (objc > 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "?-nodes?");
        return TCL_ERROR;
    } else if (objc == 1) {
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
GraphCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;

    Graph* graphPtr = (Graph*) clientData;
    const char *subCmds[] = {
        "addnodes", "delnodes", "nodes", "configure", "cget", "delete", NULL
    };
    enum BlobIx {
        AddNodesIx, RmNodesIx, NodesIx, ConfigureIx, CgetIx, DeleteIx
    };

    if (objc == 1) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case AddNodesIx: {
        return GraphCmdAddNodes(graphPtr, interp, objc-2, objv+2);
    }
    case RmNodesIx: {
        return GraphCmdRmNodes(graphPtr, interp, objc-2, objv+2);
    }
    case NodesIx: {
        return GraphCmdNodes(graphPtr, interp, objc-2, objv+2);
    }
    case ConfigureIx: {
        return GraphCmdConfigure(graphPtr, interp, objc-2, objv+2);
    }
    case CgetIx: {
        return GraphCmdCget(graphPtr, interp, objc-2, objv+2);
    }
    case DeleteIx: {
        return GraphCmdDelete(graphPtr, interp, objc-2, objv+2);
    }
    default: {
        break;
    }
    }

    return TCL_ERROR;
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
Graph_CreateCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    GraphState* gState = (GraphState*)clientData;
    Graph* graphPtr;
    Tcl_HashEntry* entryPtr;
    int new;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "[new|cmdname] ?arg ...?");
        return TCL_ERROR;
    }

    graphPtr = (Graph*)Tcl_Alloc(sizeof(Graph));
    graphPtr->statePtr = gState;
    sprintf(graphPtr->name,"%s", "");

    if (Tcl_StringMatch(Tcl_GetString(objv[1]), "new")) {
        sprintf(graphPtr->cmdName, "::tclgraphs::Graph%d", gState->graphUid);
    } else {
        sprintf(graphPtr->cmdName, Tcl_GetString(objv[1]));
    }

    if (Graphs_CheckCommandExists(interp, graphPtr->cmdName)) {
        Tcl_Free((char*)graphPtr);
        return TCL_ERROR;
    }

    if ((objc > 2) && GraphCmdConfigure(graphPtr, interp, objc-2, objv+2) != TCL_OK) {
        Tcl_Free((char*)graphPtr);
        return TCL_ERROR;
    }

    gState->graphUid++;

    Tcl_InitHashTable(&graphPtr->nodes, TCL_STRING_KEYS);

    graphPtr->commandTkn = Tcl_CreateObjCommand(interp, graphPtr->cmdName, GraphCmd, graphPtr, GraphDeleteCmd);

    entryPtr = Tcl_CreateHashEntry(&gState->graphs, graphPtr->cmdName, &new);
    Tcl_SetHashValue(entryPtr, (ClientData)graphPtr);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(graphPtr->cmdName, -1));

    return TCL_OK;
}

void
Graph_CleanupCmd(ClientData data)
{

}
