#include "graphsInt.h"
#include <string.h>

/*
 * Edge API
 */
#define GRAPHS_MARKS_SET_HIDDEN(flags, newFlag) newFlag ? (flags | GRAPHS_MARK_HIDDEN) : (flags & ~GRAPHS_MARK_HIDDEN); 


static const char* EdgeSubCmommands[] = {
    "new",
    "create",
    "get",
    "configure",
    "cget",
    "destroy",
    "labels",
    "mark",
    "unmark",
    "ismarked",
    NULL
};
enum EdgeSubCommandIndex
{
    EdgeNewIx,
    EdgeCreateIx,
    EdgeGetIx,
    EdgeConfigureIx,
    EdgeCgetIx,
    EdgeDestroyIx,
    EdgeLabelsIx,
    EdgeMarkIx,
    EdgeUnmarkIx,
    EdgeIsmarkedIx
};

static const char* EdgeCGetOptions[] = {
    "-name",
    "-from",
    "-to",
    "-weight",
    "-data",
    "-directed",
    NULL
};

enum EdgeCGetOptionsIndex
{
    EdgeCGetOptionNameIx,
    EdgeCGetOptionFromIx,
    EdgeCGetOptionToIx,
    EdgeCGetOptionWeightIx,
    EdgeCGetOptionDataIx,
    EdgeCGetOptionDirectedIx
};

static const char* EdgeGetOptions[] = {
    "-marks",
    NULL
};
enum EdgeGetOptionIndex
{
    EdgeGetOptionMarkIx
};

static const char* EdgeConfigureOptions[] = { "-name", "-weight", "-data", NULL };
enum EdgeConfigureOptionsIndex
{
    ConfigureOptionNameIx,
    ConfigureOptionWeightIx,
    ConfigureOptionDataIx
};

static const char* EdgeMarks[] = { "hidden", "cut", NULL };
enum EdgeMarksIndex
{
    EdgeMarkHiddenIx,
    EdgeMarkCutIx
};

static const DeltaEntry* FindDeltaEntryByNode(const DeltaEntry* start, const Node* nodePtr)
{
    const DeltaEntry* entry = start;
    while (entry != NULL) {
        if (entry->nodePtr == nodePtr) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

static void FindAndDeleteDeltaEntryByNode(DeltaEntry** start, const Node* nodePtr)
{
    DeltaEntry* now = *start;
    DeltaEntry* prev = NULL;

    while (now != NULL) {
        if (now->nodePtr == nodePtr) {
            if (prev != NULL) {
                prev->next = now->next;
            }
            else {
                *start = now->next;
            }
            ckfree((ClientData)now);
            break;
        }
        prev = now;
        now = now->next;
    }
}


int EdgeCmdCget(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx;
    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option arg");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], EdgeCGetOptions, "option", 0, &optIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (optIdx) {
    case EdgeCGetOptionNameIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->name, -1));
        return TCL_OK;
    }
    case EdgeCGetOptionFromIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->fromNode->cmdName, -1));
        return TCL_OK;
    }
    case EdgeCGetOptionToIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->toNode->cmdName, -1));
        return TCL_OK;
    }
    case EdgeCGetOptionWeightIx: {
        Tcl_SetObjResult(interp, Tcl_NewDoubleObj(edgePtr->weight));
        return TCL_OK;
    }
    case EdgeCGetOptionDataIx: {
        Tcl_SetObjResult(interp, edgePtr->data);
        return TCL_OK;
    }
    case EdgeCGetOptionDirectedIx: {
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(edgePtr->directionType == EDGE_DIRECTED));
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

    if (objc < 1 || objc > 12 || (objc % 2) != 0) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    for (i = 0; i < objc; i += 2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], EdgeConfigureOptions, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        switch (optIdx) {
        case ConfigureOptionNameIx: {
            sprintf(edgePtr->name, "%s", Tcl_GetString(objv[i + 1]));
            break;
        }
        case ConfigureOptionWeightIx: {
            if (Tcl_GetDoubleFromObj(interp, objv[i + 1], &edgePtr->weight) != TCL_OK) {
                return TCL_ERROR;
            }
            break;
        }
        case ConfigureOptionDataIx: {
            Tcl_DecrRefCount(edgePtr->data);
            edgePtr->data = objv[i + 1];
            Tcl_IncrRefCount(edgePtr->data);
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

int EdgeCmdDestroy(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    Tcl_DeleteCommandFromToken(interp, edgePtr->commandTkn);
    return TCL_OK;
}

int EdgeCmdLabels(Edge* edgePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    return TCL_OK;
}

static int EdgeCmdMark(Edge* edgePtr, Tcl_Interp* interp, int cmdIdx, int objc, Tcl_Obj* const objv[])
{
    int markIndx;

    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "<mark>");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], EdgeMarks, "mark", 0, &markIndx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (markIndx) {
    case EdgeMarkHiddenIx: 
    case EdgeMarkCutIx: {
        unsigned hasMark;

        if (cmdIdx == EdgeMarkIx || cmdIdx == EdgeUnmarkIx) {
            int newMark = (cmdIdx == EdgeMarkIx);
            edgePtr->marks = GRAPHS_MARKS_SET_HIDDEN(edgePtr->marks, newMark);
        }
        hasMark = (edgePtr->marks & GRAPHS_MARK_HIDDEN);
        Tcl_SetObjResult(interp, Tcl_NewBooleanObj(hasMark));
        break;
    }
    default: {
        Tcl_Obj* result = Tcl_NewStringObj("Wrong mark: ", -1);
        Tcl_AppendObjToObj(result, objv[0]);
        Tcl_SetObjResult(interp, result);
        return TCL_ERROR;
    }
    }

    return TCL_OK;
}

static int EdgeCmd(Edge* edgePtr, enum EdgeSubCommandIndex cmdIdx, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    switch (cmdIdx) {
    case EdgeConfigureIx:
        return EdgeCmdConfigure(edgePtr, interp, objc, objv);
    case EdgeCgetIx:
        return EdgeCmdCget(edgePtr, interp, objc, objv);
    case EdgeDestroyIx:
        return EdgeCmdDestroy(edgePtr, interp, objc, objv);
    case EdgeLabelsIx:
        return GraphsInt_LabelsCommand(edgePtr->labels, interp, objc, objv);
    case EdgeMarkIx:
    case EdgeUnmarkIx:
    case EdgeIsmarkedIx:
        return EdgeCmdMark(edgePtr, interp, cmdIdx, objc, objv);
    case EdgeNewIx:
    case EdgeCreateIx:
    case EdgeGetIx:
    default: {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "Wrong edge method ", EdgeSubCmommands[cmdIdx], " should be configure cget or destroy",
        NULL);
        Tcl_SetObjResult(interp, res);
        return TCL_ERROR;
    }
    }
}

static int EdgeSubCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int cmdIdx;

    Edge* edgePtr = (Edge*) clientData;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], EdgeSubCmommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    return EdgeCmd(edgePtr, cmdIdx, interp, objc - 2, objv + 2);
}

static void EdgeDestroyCmd(ClientData clientData)
{
    Edge* edgePtr = (Edge*)clientData;

    if (edgePtr->fromNode != NULL && edgePtr->toNode != NULL) {
        FindAndDeleteDeltaEntryByNode(&edgePtr->fromNode->outgoing, edgePtr->toNode);
        FindAndDeleteDeltaEntryByNode(&edgePtr->toNode->incoming, edgePtr->fromNode);

        /*
         * Undirected edges are linked to the other side (from toNode to fromNode) as well.
         * Delete this link without any further intervention.
         */
        if (edgePtr->directionType == EDGE_UNDIRECTED) {
            FindAndDeleteDeltaEntryByNode(&edgePtr->toNode->outgoing, edgePtr->fromNode);
            FindAndDeleteDeltaEntryByNode(&edgePtr->fromNode->incoming, edgePtr->toNode);
            edgePtr->fromNode->degreeundir--;
            edgePtr->toNode->degreeundir--;
        } else {
            edgePtr->fromNode->degreeplus--;
            edgePtr->toNode->degreeminus--;
        }
    }

    if (edgePtr->data != NULL) {
        Tcl_DecrRefCount(edgePtr->data);
    }

    Tcl_DeleteHashTable(&edgePtr->labels);
    edgePtr->fromNode = edgePtr->toNode = NULL;

    Tcl_HashEntry* entry = Tcl_FindHashEntry(&edgePtr->statePtr->edges, edgePtr->cmdName);
    Tcl_DeleteHashEntry(entry);

    Tcl_Free((char*) edgePtr);
}


static int EdgeParseMarks(const char* marksSpec, unsigned int* marksMaskOut) {
    unsigned int lclMarksMask = 0;
    for (int i = 0; i < strlen(marksSpec); i++) {
        char c = marksSpec[i];
        if (c == 'c' || c == 'h') {
            lclMarksMask |= GRAPHS_MARK_HIDDEN;
        }
        else if (c == 'C' || c == 'H') {
            lclMarksMask |= ~GRAPHS_MARK_HIDDEN;
        }
        else {
            return TCL_ERROR;
        }
    }
    *marksMaskOut = lclMarksMask;
    return TCL_OK;
}

/*
 * Implements the edge command.
 *
 * Used to create edges ([edge create <name> ...], [edge new ...]), to get edges ([edge get ...])
 * and to call subcommands on edges. The edge can be given as its own command name, i.e. the
 * name specified/returned during create/new, or the by the nodes that the edges connects. In the
 * latter case, the call is i.e. [edge <cmd> n1 -> n2], where -> denotes the outgoing edge from
 * n1 to n2. The three possibilities are "n1 -> n2" for outgoing edges from n1 to n2, "n1 <- n2"
 * for incoming edges, resp. outgoing edge from n2 to n1 and n1 <-> n2 for undirected edges.
 *
 * Examples:
 *
 *  edge create e n1 -> n2
 *  - creates an outgoing directed edge from n1 to n2, named e (n2 is a neighbor of n1 but n1 is
 *    not a neighbor of n2)
 *
 *  edge new n1 <-> n2
 *  - creates an undirected edge between n1 and n2 (both are neighbors of each other) with auto
 *    choosen name. The name is returned
 *
 *  edge get n1 -> n2
 *  - resolves and returns the edge handle/command for the directed edge from n1 to n2
 *
 *  edge configure n1 -> n2 -weight 42
 *  - configures the directed edge from n1 to n2 with a weight of 42
 *
 *  edge destroy e
 *  - deletes the edge with token/command e
 */
int GraphsInt_EdgeCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    GraphState* gState = (GraphState*) clientData;
    Edge* edgePtr = NULL;
    Node* fromNodePtr = NULL;
    Node* toNodePtr = NULL;
    int cmdIdx, directionIdx;
    int unDirected = 0;

    const char* directChars[] = { "->", "<-", "<->", NULL };
    enum directIdx
    {
        OutIx,
        InIx,
        UndirIx
    };

    if (objc < 5) {
        Tcl_WrongNumArgs(interp, 1, objv, "[new | create <name> | get] <node> [-> | <->] <node> ?arg ...?");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], EdgeSubCmommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    int pOffset = 0;
    if (cmdIdx == EdgeCreateIx) {
        if (objc < 6) {
            Tcl_WrongNumArgs(interp, 1, objv, "[create <name>] <node> [-> | <->] <node> ?arg ...?");
            return TCL_ERROR;
        }
        objc--;
        objv++;
    }

    if (Tcl_GetIndexFromObj(interp, objv[3], directChars, "direction", TCL_EXACT, &directionIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    /*
    * Check the direction index and set fromNode and toNode pointers while validating them
    */
    switch (directionIdx) {
    case OutIx: {
        fromNodePtr = Graphs_NodeGetByCommand(gState, Tcl_GetString(objv[2 + pOffset]));
        toNodePtr = Graphs_NodeGetByCommand(gState, Tcl_GetString(objv[4 + pOffset]));
        break;
    }
    case InIx: {
        fromNodePtr = Graphs_NodeGetByCommand(gState, Tcl_GetString(objv[4 + pOffset]));
        toNodePtr = Graphs_NodeGetByCommand(gState, Tcl_GetString(objv[2 + pOffset]));
        break;
    }
    case UndirIx: {
        fromNodePtr = Graphs_NodeGetByCommand(gState, Tcl_GetString(objv[2 + pOffset]));
        toNodePtr = Graphs_NodeGetByCommand(gState, Tcl_GetString(objv[4 + pOffset]));
        unDirected = 1;
        break;
    }
    default: {
        Tcl_Obj* res = Tcl_NewObj();
        Tcl_AppendStringsToObj(res, "Not a valid edge direction: \"", Tcl_GetString(objv[3 + pOffset]),
            "\". Must be ->, <- or <->",
            NULL);
        Tcl_SetObjResult(interp, res);
        return TCL_ERROR;
    }
    }

    if ((fromNodePtr == NULL) || (toNodePtr == NULL)) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("from and to parameters are required to be valid nodes", -1));
        return TCL_ERROR;
    }

    switch (cmdIdx)
    {
    case EdgeGetIx: {
        unsigned int edgeMarksMask = 0;
        if (objc == 6 || objc > 7) {
            Tcl_WrongNumArgs(interp, 5, objv, "?-marks [cChH]?");
            return TCL_ERROR;
        }
        else if (objc == 7) {
            if (Tcl_GetIndexFromObj(interp, objv[5], EdgeGetOptions, "-marks", 0, &cmdIdx) != TCL_OK) {
                return TCL_ERROR;
            }
            if (EdgeParseMarks(Tcl_GetString(objv[6]), &edgeMarksMask) != TCL_OK) {
                Tcl_SetObjResult(interp, Tcl_NewStringObj(
                    "Wrong marks specifier. Should be a combination of c/h (cut/hidden) C/H (not cut/hidden).",
                    -1));
                return TCL_ERROR;
            }
        }
        edgePtr = Graphs_EdgeGetEdge(gState, fromNodePtr, toNodePtr, unDirected, edgeMarksMask);
        if (edgePtr == NULL) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("", -1));
            return TCL_OK;
        }
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->cmdName, -1));
        return TCL_OK;
    }
    case EdgeNewIx:
    case EdgeCreateIx:
        edgePtr = Graphs_EdgeCreateEdge(gState, fromNodePtr, toNodePtr, unDirected, interp,
            Tcl_GetString(objv[1 + pOffset]), objc - (5 + pOffset), objv + 5 + pOffset);
        if (edgePtr == NULL) {
            return TCL_ERROR;
        }
        Tcl_SetObjResult(interp, Tcl_NewStringObj(edgePtr->cmdName, -1));
        return TCL_OK;
    default: {
        Tcl_Obj* errRes = Tcl_NewObj();
        Tcl_AppendStringsToObj(errRes, "Wrong subcommand for edge: ", Tcl_GetString(objv[1]), NULL);
        Tcl_SetObjResult(interp, errRes);
        return TCL_ERROR;
    }
    }

}

static int CreateAndInsertNewDeltaEntry(Node* fromNodePtr, DeltaEntry** startRef, Node* nodePtr, Edge* edgePtr, Tcl_Interp* interp)
{
    DeltaEntry* newEntry;

    const DeltaEntry* entry = FindDeltaEntryByNode(*startRef, nodePtr);
    if (entry != NULL) {
        Tcl_Obj* result = Tcl_NewObj();
        Tcl_AppendStringsToObj(result, nodePtr->cmdName, " is already neighbor of ", fromNodePtr->cmdName, NULL);
        Tcl_SetObjResult(interp, result);
        Tcl_Free((char*)edgePtr);
        return TCL_ERROR;
    }

    newEntry = (DeltaEntry*) ckalloc(sizeof(DeltaEntry));
    newEntry->nodePtr = nodePtr;
    newEntry->edgePtr = edgePtr;
    newEntry->next = *startRef;
    *startRef = newEntry;
    return TCL_OK;
}

static void EdgeAddToGraph(Graph* graphPtr, Edge* edgePtr)
{
    int new;
    Tcl_HashEntry* entry;

    if (graphPtr != NULL) {
        entry = Tcl_CreateHashEntry(&graphPtr->edges, (ClientData)edgePtr, &new);
        if (new) {
            Tcl_SetHashValue(entry, edgePtr);
        }
    }
}

Edge*
Graphs_EdgeCreateEdge(GraphState* gState, Node* fromNodePtr, Node* toNodePtr, int unDirected, Tcl_Interp* interp,
    const char* cmdName, int objc, Tcl_Obj* const objv[])
{
    Tcl_HashEntry* entryPtr;
    int new;
    Edge* edgePtr;

    edgePtr = (Edge*)Tcl_Alloc(sizeof(Edge));
    edgePtr->statePtr = gState;
    edgePtr->data = Tcl_NewListObj(0, NULL);

    if (Tcl_StringMatch(cmdName, "new")) {
        sprintf(edgePtr->cmdName, "::graphs::Edge%d", gState->edgeUid);
        gState->edgeUid++;
    }
    else {
        sprintf(edgePtr->cmdName, "%s", cmdName);
    }

    if (GraphsInt_CheckCommandExists(interp, edgePtr->cmdName)) {
        Tcl_Obj* result = Tcl_NewObj();
        Tcl_AppendStringsToObj(result, edgePtr->cmdName, " exists already", NULL);
        Tcl_SetObjResult(interp, result);
        Tcl_Free((char*)edgePtr);
        return NULL;
    }

    sprintf(edgePtr->name, "%s", "");
    edgePtr->weight = 0.;
    edgePtr->directionType = EDGE_DIRECTED;
    edgePtr->fromNode = fromNodePtr;
    edgePtr->toNode = toNodePtr;
    edgePtr->marks = 0;

    if (objc > 0 && EdgeCmdConfigure(edgePtr, interp, objc, objv) != TCL_OK) {
        Tcl_Free((char*)edgePtr);
        return NULL;
    }

    /* create neighbor and check if exists */
    {
        if (CreateAndInsertNewDeltaEntry(fromNodePtr, &fromNodePtr->outgoing, toNodePtr, edgePtr, interp)) {
            return NULL;
        }

        if (unDirected) {
            /*
             * undirected edges are outgoing from both sides, and are flagged as EDGE_UNDIRECTED
             */
            if (CreateAndInsertNewDeltaEntry(toNodePtr, &toNodePtr->outgoing, fromNodePtr, edgePtr, interp)) {
                return NULL;
            }
            edgePtr->directionType = EDGE_UNDIRECTED;
            fromNodePtr->degreeundir++;
            toNodePtr->degreeundir++;
        }
        else {
            if (CreateAndInsertNewDeltaEntry(toNodePtr, &toNodePtr->incoming, fromNodePtr, edgePtr, interp)) {
                return NULL;
            }
            fromNodePtr->degreeplus++;
            toNodePtr->degreeminus++;
        }
    }

    Tcl_InitHashTable(&edgePtr->labels, TCL_STRING_KEYS);
    EdgeAddToGraph(fromNodePtr->graph, edgePtr);
    EdgeAddToGraph(toNodePtr->graph, edgePtr);

    edgePtr->commandTkn = Tcl_CreateObjCommand(interp, edgePtr->cmdName, EdgeSubCmd, edgePtr, EdgeDestroyCmd);
    entryPtr = Tcl_CreateHashEntry(&gState->edges, edgePtr->cmdName, &new);
    Tcl_SetHashValue(entryPtr, edgePtr);
    return edgePtr;
}


int Graphs_EdgeHasMarks(const Edge* edgePtr, unsigned marksMask)
{
    if (marksMask == 0) {
        /* return the default: all edges */
        return 1;
    }

    unsigned returnIf = 0;
    if ((marksMask & GRAPHS_MARK_HIDDEN) == GRAPHS_MARK_HIDDEN) {
        returnIf |= ((edgePtr->marks & GRAPHS_MARK_HIDDEN) == GRAPHS_MARK_HIDDEN);
    }
    if ((marksMask & ~GRAPHS_MARK_HIDDEN) == ~GRAPHS_MARK_HIDDEN) {
        returnIf |= ((edgePtr->marks & ~GRAPHS_MARK_HIDDEN) == edgePtr->marks);
    }
    return returnIf;
}

Edge*
Graphs_EdgeGetEdge(const GraphState* gState, CONST Node* fromNodePtr, CONST Node* toNodePtr, int unDirected, unsigned int marksMask)
{
    const DeltaEntry* entry = FindDeltaEntryByNode((CONST DeltaEntry*)fromNodePtr->outgoing, toNodePtr);
    if (entry == NULL) {
        return NULL;
    }
    Edge* edgePtr1 = entry->edgePtr;

    if (unDirected) {
        entry = FindDeltaEntryByNode((CONST DeltaEntry*)toNodePtr->outgoing, fromNodePtr);
        if (entry == NULL) {
            return NULL;
        }

        Edge* edgePtr2 = entry->edgePtr;
        if (edgePtr1 != edgePtr2) {
            return NULL;
        }
    }

    return Graphs_EdgeHasMarks(edgePtr1, marksMask) ? edgePtr1 : NULL;
}


void GraphsInt_EdgeCleanupCmd(ClientData data)
{

}

