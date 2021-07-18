#include "graphs.h"
#include <string.h>

/*
 * Node API
 */
#define GRAPHS_MARKS_SET_HIDDEN(flags, newFlag) newFlag ? (flags | GRAPHS_MARK_HIDDEN) : (flags & ~GRAPHS_MARK_HIDDEN); 


static const char* LabelFilterOptions[] = { "-name", "-labels", "-notlabels", "-all", NULL };

static const char* NodeOptions[] = { "-name", "-graph", "-data", NULL };
enum NodeOptsIdx { NameIx, GraphIx, DataIx };

typedef enum _EdgeDirection
{
    EDGE_DIRECTION_NONE,
    EDGE_DIRECTION_OUT,
    EDGE_DIRECTION_IN
} EdgeDirection;


static const char* NodeInfoOptions[] = {
        "delta+",
        "deltaplus",
        "delta-",
        "deltaminus",
        "delta",
        "degree+",
        "degreeplus",
        "degree-",
        "degreeminus",
        "degree",
        "graph",
        "labels",
        NULL
};

enum NodeInfoIndex
{
    NodeInfoDeltaPlus1Ix,
    NodeInfoDeltaPlus2Ix,
    NodeInfoDeltaMinus1Ix,
    NodeInfoDeltaMinus2Ix,
    NodeInfoDeltaIx,
    NodeInfoDegreePlus1Ix,
    NodeInfoDegreePlus2Ix,
    NodeInfoDegreeMinus1Ix,
    NodeInfoDegreeMinus2Ix,
    NodeInfoDegreeIx,
    NodeInfoGraphIx,
    NodeInfoLabelsIx
};

static const char* NodeMarks[] = { "hidden", NULL };
enum NodeMarksIndex
{
    NodeMarkHiddenIx
};


static int NodeCmdConfigure(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int i, optIdx;

    if (objc < 2 || objc > 4 || (objc % 2) != 0) {
        Tcl_WrongNumArgs(interp, 0, objv, "option arg ?option arg ...?");
        return TCL_ERROR;
    }

    for (i = 0; i < objc; i += 2) {
        if (Tcl_GetIndexFromObj(interp, objv[i], NodeOptions, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        switch (optIdx) {
        case NameIx: {
            sprintf(nodePtr->name, "%s", Tcl_GetString(objv[i + 1]));
            break;
        }
        case GraphIx: {
            Graph* g = NULL;
            if (Tcl_StringMatch(Tcl_GetString(objv[i+1]), "")) {
                Graphs_AddNodeToGraph(g, nodePtr);
                return TCL_OK;
            }
            g = Graphs_GraphGetByCommand(nodePtr->statePtr, Tcl_GetString(objv[i+1]));
            if (g == NULL) {
                Tcl_Obj* res = Tcl_NewStringObj("No such graph: ", -1);
                Tcl_AppendObjToObj(res, objv[2]);
                Tcl_SetObjResult(interp, res);
                return TCL_ERROR;
            }
            Graphs_AddNodeToGraph(g, nodePtr);
            break;
        }
        case DataIx: {
            Tcl_DecrRefCount(nodePtr->data);
            nodePtr->data = objv[i + 1];
            Tcl_IncrRefCount(nodePtr->data);
            break;
        }
        default: {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong options in node configure", -1));
            return TCL_ERROR;
        }
        }
    }

    return TCL_OK;
}

static int NodeCmdCget(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int optIdx;

    if (objc != 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[0], NodeOptions, "option", 0, &optIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (optIdx) {
    case NameIx: {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->name, -1));
        return TCL_OK;
    }
    case GraphIx: {
        if (nodePtr->graph == NULL) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("",-1));
        } else {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->graph->cmdName, -1));
        }
        return TCL_OK;
    }
    case DataIx: {
        Tcl_SetObjResult(interp, nodePtr->data);
        return TCL_OK;
    }
    default: {
        break;
    }
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong options in node configure", -1));
    return TCL_ERROR;
}

/*
 * The delete subcommand, calls the command delete procedure.
 */
static int NodeCmdDestroy(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Tcl_DeleteCommandFromToken(interp, nodePtr->commandTkn);
    return TCL_OK;
}

static int NodeInfoDelta(Node* nodePtr, DeltaT deltaType, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int optIdx = LABELS_ALL_IDX;
    if (objc > 0) {
        if (Tcl_GetIndexFromObj(interp, objv[0], LabelFilterOptions, "option", 0, &optIdx) != TCL_OK) {
            return TCL_ERROR;
        }
        if (Graphs_CheckLabelsOptions(optIdx, interp, objc, objv) != TCL_OK) {
            return TCL_ERROR;
        }
    }

    Tcl_Obj* deltaList = Tcl_NewObj();
    struct LabelFilter lblFilt;
    lblFilt.filterType = optIdx;
    lblFilt.objc = objc - 1;
    lblFilt.objv = (Tcl_Obj**) objv + 1;
    if (Graphs_GetDelta(nodePtr, NULL, deltaType, lblFilt, interp, &deltaList) != TCL_OK) {
        return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, deltaList);
    return TCL_OK;
}

static int NodeCmdInfo(Node* nodePtr, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    int cmdIdx;
    if (objc < 1) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], NodeInfoOptions, "option", TCL_EXACT, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case NodeInfoDeltaPlus1Ix:
    case NodeInfoDeltaPlus2Ix: {
        return NodeInfoDelta(nodePtr, DELTA_PLUS, interp, objc - 1, objv + 1);
    }
    case NodeInfoDeltaMinus1Ix:
    case NodeInfoDeltaMinus2Ix: {
        return NodeInfoDelta(nodePtr, DELTA_MINUS, interp, objc - 1, objv + 1);
    }
    case NodeInfoDeltaIx: {
        return NodeInfoDelta(nodePtr, DELTA_ALL, interp, objc - 1, objv + 1);
    }
    case NodeInfoDegreePlus1Ix:
    case NodeInfoDegreePlus2Ix: {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nodePtr->degreeplus));
        return TCL_OK;
    }
    case NodeInfoDegreeMinus1Ix:
    case NodeInfoDegreeMinus2Ix: {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nodePtr->degreeminus));
        return TCL_OK;
    }
    case NodeInfoDegreeIx: {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(nodePtr->degreeundir));
        return TCL_OK;
    }
    case NodeInfoGraphIx: {
        if (nodePtr->graph == NULL) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("",-1));
        } else {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->graph->cmdName, -1));
        }
        return TCL_OK;
    }
    case NodeInfoLabelsIx: {
        if (objc > 1) {
            Tcl_WrongNumArgs(interp, 0, objv, "option");
            return TCL_ERROR;
        }
        return Graphs_LabelsCommand(nodePtr->labels, interp, objc - 1, objv + 1);
    }

    }

    return TCL_OK;
}

static int NodeCmdMark(Node* nodePtr, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
{
    int markIndx;

    if (objc < 1 || objc > 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "mark <mark> ?true|false?");
        return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[0], NodeMarks, "mark", 0, &markIndx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (markIndx) {
    case NodeMarkHiddenIx: {
        unsigned hasMark;

        if (objc == 2) {
            int newMark = 0;
            if (Tcl_GetBooleanFromObj(interp, objv[1], &newMark) != TCL_OK) {
                return TCL_ERROR;
            }
            nodePtr->marks = GRAPHS_MARKS_SET_HIDDEN(nodePtr->marks, newMark);
        }

        hasMark = (nodePtr->marks & GRAPHS_MARK_HIDDEN);
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

static int NodeSubCmd(Node* nodePtr, Tcl_Obj* cmd, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    const char* nodeSubCommands[] ={"destroy","configure","cget","info","labels","mark",NULL};
    enum nodeCommandIndex {destroyIx,configureIx,cgetIx,infoIx,labelsIx,markIx};


    int cmdIdx;

    if (Tcl_GetIndexFromObj(interp, cmd, nodeSubCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    switch (cmdIdx) {
    case destroyIx:
        return NodeCmdDestroy(nodePtr, interp, objc, objv);
    case configureIx:
        return NodeCmdConfigure(nodePtr, interp, objc, objv);
    case cgetIx:
        return NodeCmdCget(nodePtr, interp, objc, objv);
    case infoIx:
        return NodeCmdInfo(nodePtr, interp, objc, objv);
    case labelsIx:
        return Graphs_LabelsCommand(nodePtr->labels, interp, objc, objv);
    case markIx:
        return NodeCmdMark(nodePtr, interp, objc, objv);
    default:
        Tcl_SetObjResult(interp, Tcl_NewStringObj("Wrong subcommand to node object!", -1));
        return TCL_ERROR;
    }

}

static int Node_NodeSubCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    Node* nodePtr = (Node*) clientData;
    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 1, objv, "option ?arg ...?");
        return TCL_ERROR;
    }
    return NodeSubCmd(nodePtr, objv[1], interp, objc - 2, objv + 2);
}

/*
 * Command Delete callback
 */
static void NodeDestroyCmd(ClientData clientData)
{
    Node* nodePtr = (Node*) clientData;
    Tcl_HashEntry* entry;
    Tcl_HashSearch search;

    /* delete outgoing edges (neighbors) */
    entry = Tcl_FirstHashEntry(&nodePtr->outgoing, &search);
    while (entry != NULL) {
        Edge* edgePtr = (Edge*) Tcl_GetHashValue(entry);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        entry = Tcl_FirstHashEntry(&nodePtr->outgoing, &search);
    }

    /* delete incoming edges */
    entry = Tcl_FirstHashEntry(&nodePtr->incoming, &search);
    while (entry != NULL) {
        Edge* edgePtr = (Edge*) Tcl_GetHashValue(entry);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        Graphs_DeleteEdge(edgePtr, edgePtr->statePtr->interp);
        entry = Tcl_FirstHashEntry(&nodePtr->incoming, &search);
    }

    /* remove myself from the graphs where I am part of */
    if (nodePtr->graph != NULL) {
        Graphs_DeleteNodeFromGraph(nodePtr->graph, nodePtr);
    }

    if (nodePtr->data != NULL) {
        Tcl_DecrRefCount(nodePtr->data);
    }

    Tcl_DeleteHashTable(&nodePtr->outgoing);
    Tcl_DeleteHashTable(&nodePtr->incoming);
    Tcl_DeleteHashTable(&nodePtr->labels);

    entry = Tcl_FindHashEntry(&nodePtr->statePtr->nodes, nodePtr->cmdName);
    Tcl_DeleteHashEntry(entry);

    Tcl_Free((char*) nodePtr);
}

int Node_NodeCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[])
{
    const char* subCommands[] = { "new", "create", NULL };
    enum subCommandIdx { newIdx, createIdx };

    GraphState* gState = (GraphState*) clientData;
    int new, cmdIdx;
    Node* nodePtr;
    Tcl_HashEntry* entryPtr;

    if (objc < 2) {
        Tcl_WrongNumArgs(interp, 0, objv, "option");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], subCommands, "method", 0, &cmdIdx) != TCL_OK) {
        return TCL_ERROR;
    }

    objc -= 2;
    objv += 2;

    nodePtr = (Node*)Tcl_Alloc(sizeof(Node));
    nodePtr->statePtr = gState;
    nodePtr->graph = NULL;
    nodePtr->data = Tcl_NewListObj(0, NULL);
    nodePtr->marks = 0;
    nodePtr->degreeplus = 0;
    nodePtr->degreeminus = 0;
    nodePtr->degreeundir = 0;
    sprintf(nodePtr->name, "%s", "");

    switch (cmdIdx) {
    case newIdx:
        sprintf(nodePtr->cmdName, "::graphs::Node%d", gState->nodeUid++);
        break;
    case createIdx:
        if (objc < 1) {

            Tcl_WrongNumArgs(interp, 1, objv, "<name>");
            Tcl_Free((char*)nodePtr);
            return TCL_ERROR;
        }
        const char* cmdName = Tcl_GetString(objv[0]);
        if (Graphs_CheckCommandExists(interp, cmdName)) {
            Tcl_Free((char*)nodePtr);
            return TCL_ERROR;
        }
        sprintf(nodePtr->cmdName, "%s", cmdName);
        objc--;
        objv++;
        break;
    }

    Tcl_InitHashTable(&nodePtr->outgoing, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&nodePtr->incoming, TCL_ONE_WORD_KEYS);
    Tcl_InitHashTable(&nodePtr->labels, TCL_STRING_KEYS);

    entryPtr = Tcl_CreateHashEntry(&gState->nodes, nodePtr->cmdName, &new);
    Tcl_SetHashValue(entryPtr, nodePtr);

    if (objc > 0) {
        if (NodeCmdConfigure(nodePtr, interp, objc, objv) != TCL_OK) {
            NodeDestroyCmd((ClientData)nodePtr);
            return TCL_ERROR;
        }
    }

    nodePtr->commandTkn = Tcl_CreateObjCommand(interp, nodePtr->cmdName, Node_NodeSubCmd, nodePtr, NodeDestroyCmd);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(nodePtr->cmdName, -1));
    return TCL_OK;
}

void Node_CleanupCmd(ClientData data)
{
}
