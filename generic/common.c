/*
 * Common procedures for entities
 */
#include "graph.h"

int Graphs_CheckCommandExists(Tcl_Interp* interp, const char* cmdName)
{
    Tcl_CmdInfo cmdInfo;
    if (Tcl_GetCommandInfo(interp, cmdName, &cmdInfo) != 0) {
        Tcl_Obj* errRes = Tcl_NewStringObj("", -1);
        Tcl_AppendStringsToObj(errRes, "Command ", cmdName, " already exists!",
                NULL);
        Tcl_SetObjResult(interp, errRes);
        return TCL_ERROR;
    }

    return TCL_OK;

}

Graph*
Graphs_ValidateGraphCommand(GraphState* statePtr, Tcl_Interp* interp,
        const char* gName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&statePtr->graphs, gName);
    if (entry == NULL) {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "No such graph: ", gName, NULL);
        Tcl_SetObjResult(interp, res);
        return NULL;
    }
    return ((Graph*) Tcl_GetHashValue(entry));
}

Node*
Graphs_ValidateNodeCommand(GraphState* statePtr, Tcl_Interp* interp,
        const char* nName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&statePtr->nodes, nName);
    if (entry == NULL) {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "No such node: ", nName, NULL);
        Tcl_SetObjResult(interp, res);
        return NULL;
    }
    return ((Node*) Tcl_GetHashValue(entry));
}

Edge*
Graphs_ValidateEdgeCommand(GraphState* statePtr, Tcl_Interp* interp,
        const char* eName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&statePtr->edges, eName);
    if (entry == NULL) {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "No such edge: ", eName, NULL);
        Tcl_SetObjResult(interp, res);
        return NULL;
    }
    return ((Edge*) Tcl_GetHashValue(entry));
}

void Graphs_AddNodeToGraph(Graph* graphPtr, Node* nodePtr)
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

void Graphs_DeleteEdge(Edge* edgePtr, Tcl_Interp* interp)
{
    Tcl_DeleteCommandFromToken(interp, edgePtr->commandTkn);
}

void Graphs_DeleteNode(Node* nodePtr, Tcl_Interp* interp)
{
    Tcl_DeleteCommandFromToken(interp, nodePtr->commandTkn);
}

int Graphs_GetNodes(Tcl_HashTable fromTbl, LabelFilterT optIdx,
        Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_HashSearch search;
    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&fromTbl, &search);
    Tcl_Obj* result = Tcl_NewObj();

    while (entry != NULL) {
        Node* neighborPtr = (Node*) Tcl_GetHashKey(&fromTbl, entry);
        int appendRes;

        switch (optIdx) {
        case LABELS_IDX: {
            int found = 0;
            for (int i = 0; i < objc; i++) {
                Tcl_HashEntry* lblEntry = Tcl_FindHashEntry(
                        &neighborPtr->labels, Tcl_GetString(objv[i]));
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
                Tcl_HashEntry* lblEntry = Tcl_FindHashEntry(
                        &neighborPtr->labels, Tcl_GetString(objv[i]));
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
            Tcl_ListObjAppendElement(interp, result,
                    Tcl_NewStringObj(neighborPtr->cmdName, -1));
        }

        entry = Tcl_NextHashEntry(&search);
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;

}

int Graphs_LabelsCommand(Tcl_HashTable labelsTbl, Tcl_Interp* interp, int objc,
        Tcl_Obj* const objv[])
{
    int new;
    Tcl_HashEntry* entry;
    int cmdIdx;
    static char* subcmds[] =
    { "add", "+", "delete", "-", "get" };
    enum subCmdIdx
    {
        LabelsAdd1, LabelsAdd2, LabelsDel1, LabelsDel2, LabelsGet
    };

    if (objc == 0) {
        /* List graphs */
        goto labelsCommandReturnLabels;
    }

    if (Tcl_GetIndexFromObj(interp, objv[0], subcmds, "option", 0,
            &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case LabelsAdd1:
    case LabelsAdd2: {
        for (int i = 1; i < objc; i++) {
            entry = Tcl_CreateHashEntry(&labelsTbl, Tcl_GetString(objv[i]),
                    &new);
            if (new) {
                Tcl_SetHashValue(entry, NULL);
            }
        }
        return TCL_OK;
    }
    case LabelsDel1:
    case LabelsDel2: {
        for (int i = 1; i < objc; i++) {
            entry = Tcl_FindHashEntry(&labelsTbl, Tcl_GetString(objv[i]));
            if (entry != NULL) {
                Tcl_DeleteHashEntry(entry);
            }
        }
        return TCL_OK;
    }
    case LabelsGet:
    default: {
        break;
    }

    }

labelsCommandReturnLabels:
    {
        Tcl_HashSearch search;
        Tcl_Obj* result = Tcl_NewObj();
        entry = Tcl_FirstHashEntry(&labelsTbl, &search);
        while (entry != NULL) {
            Tcl_ListObjAppendElement(interp, result,
                    Tcl_NewStringObj(Tcl_GetHashKey(&labelsTbl, entry), -1));
            entry = Tcl_NextHashEntry(&search);
        }
        Tcl_SetObjResult(interp, result);

    }
    return TCL_OK;
}

static int GraphsAppendDeltaToObj(Tcl_HashTable nodesTbl, Graph* graphPtr,
        EdgeDirectionT directionType, struct LabelFilter labelFilter,
        Tcl_Interp* interp, Tcl_Obj** listObjPtr)
{
    Tcl_HashSearch search;
    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&nodesTbl, &search);
    while (entry != NULL) {
        Node* nodePtr = Tcl_GetHashKey(&nodesTbl, entry);
        if (graphPtr != NULL) {
            Tcl_HashEntry* lookupEntry = Tcl_FindHashEntry(&graphPtr->nodes,
                    nodePtr);
            if (lookupEntry == NULL) {
                /* otherNode is not in this graph. Append if it meets the direction */
                Edge* edgePtr = Tcl_GetHashValue(entry);
                if (edgePtr->directionType == directionType) {
                    if (Tcl_ListObjAppendElement(interp, *listObjPtr,
                            Tcl_NewStringObj(nodePtr->cmdName, -1)) != TCL_OK) {
                        return TCL_ERROR;
                    }
                }
            }
        }
        else {
            if (Tcl_ListObjAppendElement(interp, *listObjPtr,
                    Tcl_NewStringObj(nodePtr->cmdName, -1)) != TCL_OK) {
                return TCL_ERROR;
            }
        }

        entry = Tcl_NextHashEntry(&search);
    }

    return TCL_OK;
}

int Graphs_GetDelta(Node* nodePtr, Graph* graphPtr, DeltaT deltaT,
        struct LabelFilter labelFilter, Tcl_Interp* interp, Tcl_Obj** resultObj)
{
    Tcl_Obj* resObj = *resultObj;
    switch (deltaT) {
    case DELTA_PLUS: {
        if (GraphsAppendDeltaToObj(nodePtr->outgoing, graphPtr, EDGE_DIRECTED,
                labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        break;
    }
    case DELTA_MINUS: {
        if (GraphsAppendDeltaToObj(nodePtr->incoming, graphPtr, EDGE_DIRECTED,
                labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        break;
    }
    case DELTA_ALL:
    default: {
        if (GraphsAppendDeltaToObj(nodePtr->outgoing, graphPtr, EDGE_UNDIRECTED,
                labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        if (GraphsAppendDeltaToObj(nodePtr->incoming, graphPtr, EDGE_UNDIRECTED,
                labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        break;
    }
    }

    return TCL_OK;
}
