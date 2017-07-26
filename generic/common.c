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
Graphs_ValidateGraphCommand(GraphState* statePtr, Tcl_Interp* interp, const char* gName)
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
Graphs_ValidateNodeCommand(GraphState* statePtr, Tcl_Interp* interp, const char* nName)
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
Graphs_ValidateEdgeCommand(GraphState* statePtr, Tcl_Interp* interp, const char* eName)
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

    /* remove  old graph, if present */
    if (nodePtr->graph != NULL) {
        Tcl_HashEntry* nodeEntry = Tcl_FindHashEntry(&nodePtr->graph->nodes, nodePtr);
        if (nodeEntry != NULL) {
            Tcl_DeleteHashEntry(nodeEntry);
        }
    }

    if (graphPtr == NULL) {
        goto setNodeGraph;
    }

    entry = Tcl_CreateHashEntry(&graphPtr->nodes, nodePtr, &new);
    if (new) {
        Tcl_SetHashValue(entry, nodePtr);
    }

setNodeGraph:
    nodePtr->graph = graphPtr;
}

void Graphs_DeleteNodeFromGraph(Graph* graphPtr, Node* nodePtr)
{
    if (nodePtr->graph == graphPtr) {
        Tcl_HashEntry* entry = Tcl_FindHashEntry(&graphPtr->nodes, nodePtr);
        if (entry != NULL) {
            Tcl_DeleteHashEntry(entry);
        }
        nodePtr->graph = NULL;
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

/*
 * \brief For a given node, this procedure checks whether the labels filter applies.
 *
 * \param nodePtr The node
 * \param optIdx
 *      The label type. One of LABELS_IDX (the node has all the desired labels assigned),
 *      LABELS_NOT_IDX (the node has none of the labels assigned) or LABELS_ALL_IX (labels are
 *      not filtered)
 * \param objc
 *      Number of labels (elements in objv
 * \param objv
 *      List of string labels
 *
 * \return 1 if the labels filter applies, 0 otherwise
 *
 */
static int GraphsFilterLabels(Node* nodePtr, LabelFilterT optIdx, int objc, Tcl_Obj* const objv[])
{
    int result = 0;

    switch (optIdx) {
    case LABELS_NAME_IDX: {
        result = Tcl_StringMatch(nodePtr->name, Tcl_GetString(objv[0]));
        break;
    }
    case LABELS_IDX: {
        int found = 0;
        for (int i = 0; i < objc; i++) {
            Tcl_HashEntry* lblEntry = Tcl_FindHashEntry(&nodePtr->labels, Tcl_GetString(objv[i]));
            if (lblEntry != NULL) {
                found++;
            }
        }
        result = (found == objc);
        break;
    }
    case LABELS_NOT_IDX: {
        result = 1;
        for (int i = 0; i < objc; i++) {
            Tcl_HashEntry* lblEntry = Tcl_FindHashEntry(&nodePtr->labels, Tcl_GetString(objv[i]));
            if (lblEntry != NULL) {
                result = 0;
                break;
            }
        }
        break;
    }
    case LABELS_ALL_IDX:
    default: {
        result = 1;
        break;
    }
    }

    return result;
}

int Graphs_GetNodes(Tcl_HashTable fromTbl, LabelFilterT optIdx, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_HashSearch search;
    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&fromTbl, &search);
    Tcl_Obj* result = Tcl_NewObj();

    while (entry != NULL) {
        Node* neighborPtr = (Node*) Tcl_GetHashKey(&fromTbl, entry);
        if (GraphsFilterLabels(neighborPtr, optIdx, objc, objv)) {
            Tcl_ListObjAppendElement(interp, result, Tcl_NewStringObj(neighborPtr->cmdName, -1));
        }
        entry = Tcl_NextHashEntry(&search);
    }

    Tcl_SetObjResult(interp, result);
    return TCL_OK;

}

int Graphs_CheckLabelsOptions(LabelFilterT optIdx, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    switch (optIdx) {
    case LABELS_NAME_IDX: {
        if (objc != 2) {
            Tcl_WrongNumArgs(interp, objc, objv, "name");
            return TCL_ERROR;
        }
        break;
    }
    case LABELS_ALL_IDX: {
        if (objc != 1) {
            Tcl_WrongNumArgs(interp, objc, objv, "");
            return TCL_ERROR;
        }
        break;
    }
    case LABELS_IDX:
    case LABELS_NOT_IDX: {
        if (objc < 2) {
            Tcl_WrongNumArgs(interp, objc, objv, "label ?label ...?");
            return TCL_ERROR;
        }
        break;
    }
    default: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Error in LabelsOption Check, wrong index!", -1));
        return TCL_ERROR;
    }
    }
    return TCL_OK;
}

int Graphs_LabelsCommand(Tcl_HashTable labelsTbl, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int new;
    Tcl_HashEntry* entry;
    int cmdIdx;
    static char* subcmds[] = {
            "add",
            "+",
            "delete",
            "-",
            "get",
            NULL
    };
    enum subCmdIdx
    {
        LabelsAdd1,
        LabelsAdd2,
        LabelsDel1,
        LabelsDel2,
        LabelsGet
    };

    if (objc == 0) {
        /* List graphs */
        goto labelsCommandReturnLabels;
    }

    if (Tcl_GetIndexFromObj(interp, objv[0], subcmds, "option", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case LabelsAdd1:
    case LabelsAdd2: {
        for (int i = 1; i < objc; i++) {
            entry = Tcl_CreateHashEntry(&labelsTbl, Tcl_GetString(objv[i]), &new);
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

    labelsCommandReturnLabels: {
        Tcl_HashSearch search;
        Tcl_Obj* result = Tcl_NewObj();
        entry = Tcl_FirstHashEntry(&labelsTbl, &search);
        while (entry != NULL) {
            Tcl_ListObjAppendElement(interp, result, Tcl_NewStringObj(Tcl_GetHashKey(&labelsTbl, entry), -1));
            entry = Tcl_NextHashEntry(&search);
        }
        Tcl_SetObjResult(interp, result);

    }
    return TCL_OK;
}

static int GraphsAppendDeltaToObj(Tcl_HashTable nodesTbl, Graph* graphPtr, EdgeDirectionT directionType,
        struct LabelFilter labelFilter, Tcl_Interp* interp, Tcl_Obj** listObjPtr)
{
    Tcl_HashSearch search;
    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&nodesTbl, &search);
    while (entry != NULL) {
        Node* nodePtr = Tcl_GetHashKey(&nodesTbl, entry);
        if (!GraphsFilterLabels(nodePtr, labelFilter.filterType, labelFilter.objc, labelFilter.objv)) {
            goto nextEntry;
        }

        if (graphPtr != NULL) {
            Tcl_HashEntry* lookupEntry = Tcl_FindHashEntry(&graphPtr->nodes, nodePtr);
            if (lookupEntry == NULL) {
                /* otherNode is not in this graph. Append if it meets the direction */
                Edge* edgePtr = Tcl_GetHashValue(entry);
                if (edgePtr->directionType == directionType) {
                    if (Tcl_ListObjAppendElement(interp, *listObjPtr, Tcl_NewStringObj(nodePtr->cmdName, -1)) != TCL_OK) {
                        return TCL_ERROR;
                    }
                }
            }
        }
        else {
            if (Tcl_ListObjAppendElement(interp, *listObjPtr, Tcl_NewStringObj(nodePtr->cmdName, -1)) != TCL_OK) {
                return TCL_ERROR;
            }
        }

        nextEntry: entry = Tcl_NextHashEntry(&search);
    }

    return TCL_OK;
}

int Graphs_GetDelta(Node* nodePtr, Graph* graphPtr, DeltaT deltaT, struct LabelFilter labelFilter, Tcl_Interp* interp,
        Tcl_Obj** resultObj)
{
    Tcl_Obj* resObj = *resultObj;
    switch (deltaT) {
    case DELTA_PLUS: {
        if (GraphsAppendDeltaToObj(nodePtr->outgoing, graphPtr, EDGE_DIRECTED, labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        break;
    }
    case DELTA_MINUS: {
        if (GraphsAppendDeltaToObj(nodePtr->incoming, graphPtr, EDGE_DIRECTED, labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        break;
    }
    case DELTA_ALL:
    default: {
        if (GraphsAppendDeltaToObj(nodePtr->outgoing, graphPtr, EDGE_UNDIRECTED, labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        if (GraphsAppendDeltaToObj(nodePtr->incoming, graphPtr, EDGE_UNDIRECTED, labelFilter, interp, &resObj) != TCL_OK) {
            return TCL_ERROR;
        }
        break;
    }
    }

    return TCL_OK;
}
