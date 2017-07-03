/*
 * Common procedures for entities
 */
#include "graph.h"

int
Graphs_CheckCommandExists(Tcl_Interp* interp, const char* cmdName)
{
    Tcl_CmdInfo cmdInfo;
    if (Tcl_GetCommandInfo(interp, cmdName, &cmdInfo) != 0) {
        Tcl_Obj* errRes = Tcl_NewStringObj("", -1);
        Tcl_AppendStringsToObj(errRes, "Command ", cmdName, " already exists!", NULL);
        Tcl_SetObjResult(interp, errRes);
        return TCL_ERROR;
    }

    return TCL_OK;

}

Graph*
Graphs_ValidateGraphCommand(GraphState* statePtr, Tcl_Interp* interp, const char* gName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&statePtr->graphs, gName);
    if (entry == NULL) {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "No such graph: ", gName, NULL);
        Tcl_SetObjResult(interp, res);
        return NULL;
    }
    return ((Graph*)Tcl_GetHashValue(entry));
}

Node*
Graphs_ValidateNodeCommand(GraphState* statePtr, Tcl_Interp* interp, const char* nName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&statePtr->nodes, nName);
    if (entry == NULL) {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "No such node: ", nName, NULL);
        Tcl_SetObjResult(interp, res);
        return NULL;
    }
    return ((Node*)Tcl_GetHashValue(entry));
}

Edge*
Graphs_ValidateEdgeCommand(GraphState* statePtr, Tcl_Interp* interp, const char* eName)
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

void
Graphs_AddNodeToGraph(Graph* graphPtr, Node* nodePtr)
{
    int new;
    Tcl_HashEntry* entry;

    entry = Tcl_CreateHashEntry(&graphPtr->nodes, nodePtr, &new);
    if (new) {
        Tcl_SetHashValue(entry, nodePtr);
    }

    entry = Tcl_CreateHashEntry(&nodePtr->graphs, graphPtr->cmdName, &new);
    if (new) {
        Tcl_SetHashValue(entry, graphPtr);
    }
}

void
Graphs_DeleteEdge(Edge* edgePtr, Tcl_Interp* interp)
{
    Tcl_DeleteCommandFromToken(interp, edgePtr->commandTkn);
}

void
Graphs_DeleteNode(Node* nodePtr, Tcl_Interp* interp)
{
    Tcl_DeleteCommandFromToken(interp, nodePtr->commandTkn);
}

int
Graphs_GetNodes(Tcl_HashTable fromTbl, enum ELabelFilterOption optIdx, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_HashSearch search;
    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&fromTbl, &search);
    Tcl_Obj* result = Tcl_NewObj();

    while (entry != NULL) {
        Node* neighborPtr = (Node*)Tcl_GetHashKey(&fromTbl, entry);
        int appendRes;


        switch (optIdx) {
        case LABELS_IDX: {
            int found = 0;
            for (int i = 0; i < objc; i++) {
                Tcl_HashEntry* lblEntry = Tcl_FindHashEntry(&neighborPtr->labels, Tcl_GetString(objv[i]));
                if (lblEntry != NULL) {
                    found++;
                }
            }
            appendRes = (found == objc);
            break;
        }
        case LABELS_NOT_IDX: {
            appendRes = 1;
            for (int i = 0; i < objc; i++) {
                Tcl_HashEntry* lblEntry = Tcl_FindHashEntry(&neighborPtr->labels, Tcl_GetString(objv[i]));
                if (lblEntry != NULL) {
                    appendRes = 0;
                    break;
                }
            }
            break;
        }
        case LABELS_ALL_IX:
        default: {
            appendRes = 1;
            break;
        }
        }

        if (appendRes) {
            Tcl_ListObjAppendElement(interp, result, Tcl_NewStringObj(neighborPtr->cmdName, -1));
        }

        entry = Tcl_NextHashEntry(&search);
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;

}
