/*
 * Common procedures for entities
 */
#include "graphsInt.h"

int GraphsInt_CheckCommandExists(Tcl_Interp* interp, const char* cmdName)
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

Graph* Graphs_GraphGetByCommand(const GraphState* statePtr, const char* gName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&((GraphState*)statePtr)->graphs, gName);
    return entry == NULL ? NULL : (Graph*)Tcl_GetHashValue(entry);
}

Node*
Graphs_NodeGetByCommand(const GraphState* statePtr, const char* nName)
{
    Tcl_HashEntry* entry = Tcl_FindHashEntry(&((GraphState*)statePtr)->nodes, nName);
    return (entry == NULL) ? NULL : (Node*)Tcl_GetHashValue(entry);
}

void Graphs_NodeAddToGraph(Graph* graphPtr, Node* nodePtr)
{
    int new;
    Tcl_HashEntry* entry;

    /* remove  old graph, if present */
    if (nodePtr->graph != NULL) {
        Tcl_HashEntry* nodeEntry = Tcl_FindHashEntry(&nodePtr->graph->nodes, nodePtr->cmdName);
        if (nodeEntry != NULL) {
            Tcl_DeleteHashEntry(nodeEntry);
        }
    }

    if (graphPtr != NULL) {
        entry = Tcl_CreateHashEntry(&graphPtr->nodes, nodePtr->cmdName, &new);
        if (new) {
            Tcl_SetHashValue(entry, nodePtr);
        }
        graphPtr->order++;
    }
    nodePtr->graph = graphPtr;
}

void Graphs_NodeDeleteFromGraph(Graph* graphPtr, Node* nodePtr)
{
    if (nodePtr->graph == graphPtr) {
        Tcl_HashEntry* entry = Tcl_FindHashEntry(&graphPtr->nodes, (ClientData) nodePtr);
        if (entry != NULL) {
            Tcl_DeleteHashEntry(entry);
        }
        nodePtr->graph = NULL;
        graphPtr->order--;
    }
}

void Graphs_EdgeDeleteEdge(Edge* edgePtr, Tcl_Interp* interp)
{
    Tcl_DeleteCommandFromToken(interp, edgePtr->commandTkn);
}

void Graphs_NodeDeleteNode(Node* nodePtr, Tcl_Interp* interp)
{
    Tcl_DeleteCommandFromToken(interp, nodePtr->commandTkn);
}

/*
 * \brief For a given node, this procedure checks whether the labels filter applies.
 *
 * \param labels Hashtable with the labels
 * \param name Name to match
 * \param labelFilt The labelfilter with type and arguments to match against
 * \param matchPtr int pointer for the result
 * 
 * \return TCL_OK or TCL_ERROR if the labels filter applies, 0 otherwise
 *
 */
int GraphsInt_MatchesLabels(const Tcl_HashTable* labels, const char* name, struct LabelFilter lblFilt, int* matchPtr)
{
    int result = 0;

    switch (lblFilt.filterType) {
    case LABELS_NAME_IDX: {
        if (lblFilt.objc != 1 || name == NULL) {
            return TCL_ERROR;
        }
        result = Tcl_StringMatch(name, Tcl_GetString((Tcl_Obj*)lblFilt.objv[0]));
        break;
    }
    case LABELS_IDX: {
        int found = 0;
        if (labels == NULL) {
            return TCL_ERROR;
        }
        for (int i = 0; i < lblFilt.objc; i++) {
            Tcl_HashEntry* lblEntry = Tcl_FindHashEntry((Tcl_HashTable*)labels, Tcl_GetString((Tcl_Obj*)lblFilt.objv[i]));
            if (lblEntry != NULL) {
                found++;
            }
        }
        result = (found == lblFilt.objc);
        break;
    }
    case LABELS_NOT_IDX: {
        result = 1;
        if (labels == NULL) {
            return TCL_ERROR;
        }
        for (int i = 0; i < lblFilt.objc; i++) {
            Tcl_HashEntry* lblEntry = Tcl_FindHashEntry((Tcl_HashTable*)labels, Tcl_GetString((Tcl_Obj*)lblFilt.objv[i]));
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

    *matchPtr = result;
    return TCL_OK;
}


int Graphs_GetNodes(Tcl_HashTable fromTbl, LabelFilterT optIdx, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_HashSearch search;
    Tcl_HashEntry* entry = Tcl_FirstHashEntry(&fromTbl, &search);
    Tcl_Obj* result = Tcl_NewObj();

    while (entry != NULL) {
        Node* neighborPtr = (Node*) Tcl_GetHashValue(entry);
        struct LabelFilter lblFilt;
        lblFilt.filterType = optIdx;
        lblFilt.objc = objc;
        lblFilt.objv = objv;
        int matches = 0;
        GraphsInt_MatchesLabels(&neighborPtr->labels, neighborPtr->name, lblFilt, &matches);
        if (matches) {
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
    const char* subcmds[] = {
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

/*
 * Appends to the objResult Ptr the commands of all nodes in the specified delta.
 *
 * If graphPtr == NULL, the procedure doesn't care about in which graph the delta is. This is the case, when
 * the proc was called from NodeInfoDelta or Node_GetDelta. If graphPtr is not NULL, the proc cares about the
 * graph, where the delta node is. The it is only appended, if the graph of the delta node is not the graph
 * of the current node.
 */
static int GraphsAppendDeltaToObj(const DeltaEntry* nodes, Graph* graphPtr, EdgeDirectionT directionType,
    struct LabelFilter labelFilter, Tcl_Interp* interp, Tcl_Obj** listObjPtr)
{
    const DeltaEntry* entry = nodes;
    while (entry != NULL) {
        const Node* nodePtr = entry->nodePtr;
        int matches = 0;
        GraphsInt_MatchesLabels(&nodePtr->labels, nodePtr->name, labelFilter, &matches);
        if (!matches) {
            entry = entry->next;
            continue;
        }

        if (graphPtr != NULL) {
            Tcl_HashEntry* lookupEntry = Tcl_FindHashEntry(&graphPtr->nodes, nodePtr->cmdName);
            if (lookupEntry == NULL) {
                /* otherNode is not in this graph. Append if it meets the direction*/
                const Edge* edgePtr = entry->edgePtr;
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

        entry = entry->next;
    }

    return TCL_OK;
}

/*
 * Get the delta of nodes or graphs
 *
 * Delta of nodes are all nodes connected via an edge. Delta+ are all outgoing nodes, delta- all incoming nodes, delta
 * without + or - are all connected nodes. The same definition applies for a graph, but with nodes that are in the graph 
 * and are connected to nodes that are *not* in the graph (i.e. in another graph).
 * If the graphPtr == NULL, we don't care about whether the delta is within the graph or not. In that case, all delta 
 * nodes are returned in the resultObj. Otherwise, if the graphPtr != NULL, only the delta from the graph specified by
 * graphPtr is returned.
 *
 * \param nodePtr       The node for which to get the delta
 * \param graphPtr      The graph of the node. Specifies if the function cares about graph delta (graphPtr != NULL) 
 *                      or node delta only (graphPtr == NULL)
 * \param deltaT        The delta type that is to be returned. DELTA_PLUS are the outgoing nodes, DELTA_MINUS the incoming 
 *                      nodes and DELTA_ALL all connected nodes
 * \param labelFilter   A label filter to apply. Only delta nodes that match the specified label filter are returned
 * \param interp        The Tcl interp for setting error messages etc.
 * \param resultObj     The Tcl list object for the result. Is filled with the command names of the delta.
 */
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
