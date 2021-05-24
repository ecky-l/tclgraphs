#ifndef GRAPHS_H
#define GRAPHS_H

#include <tcl.h>

typedef struct _graphState
{
    Tcl_Interp* interp;
    Tcl_HashTable graphs;
    Tcl_HashTable nodes;
    Tcl_HashTable edges;
    int graphUid;
    int nodeUid;
    int edgeUid;
} GraphState;

typedef struct _graph
{
    char cmdName[30];
    char name[30];
    Tcl_Command commandTkn;
    GraphState* statePtr;
    Tcl_HashTable nodes;
    int order;
} Graph;

typedef struct _node
{
    char cmdName[30];
    char name[30];
    GraphState* statePtr;
    Tcl_Command commandTkn;

    /* Neighbor nodes reachable from this node. Map of nodes to edges */
    Tcl_HashTable outgoing;

    /* Nodes with incoming edges, mapped to their edges. Mainly used for cleanup */
    Tcl_HashTable incoming;

    /* labels assigned to the node. Can be used for filtering */
    Tcl_HashTable labels;

    /* graph where the node is part of */
    Graph* graph;

    /* additional arbitrary data attached to the node */
    Tcl_Obj* data;

    int degreeplus;
    int degreeminus;
    int degreeundir;

} Node;

typedef enum _EdgeDirectionType
{
    EDGE_DIRECTED,
    EDGE_UNDIRECTED
} EdgeDirectionT;

typedef enum DeltaType
{
    DELTA_PLUS,
    DELTA_MINUS,
    DELTA_ALL
} DeltaT;

typedef struct _edge
{
    char cmdName[30];
    char name[30];
    GraphState* statePtr;
    Tcl_Command commandTkn;
    Node* fromNode;
    Node* toNode;
    Tcl_Obj* dataObjPtr;
    /* Labels for the edge */
    Tcl_HashTable labels;

    double weight;

    /* Indicates whether the edge is undirected. Default is directed, then this is 0 */
    EdgeDirectionT directionType;
} Edge;

/*
 * Enum to specify filter options, used to lookup entities by label contained,
 * or labels not contained.
 */
typedef enum ELabelFilterOption
{
    LABELS_NAME_IDX,
    LABELS_IDX,
    LABELS_NOT_IDX,
    LABELS_ALL_IDX
} LabelFilterT;

struct LabelFilter
{
    LabelFilterT filterType;
    int objc;
    Tcl_Obj** objv;
};

int Graph_GraphCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]);
void Graph_CleanupCmd(ClientData data);

int Node_NodeCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]);
void Node_CleanupCmd(ClientData data);

int Edge_EdgeCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]);
void Edge_CleanupCmd(ClientData data);
Edge* Edge_CreateEdge(GraphState*, Node*, Node*, int, Tcl_Interp*, const char*, int, Tcl_Obj* const []);
Edge* Edge_GetEdge(GraphState*, Node*, Node*, int, Tcl_Interp*);
int Edge_EdgeSubCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);

/*
 * Checks whether a command already exists and sets an appropriate error message.
 */
int Graphs_CheckCommandExists(Tcl_Interp* interp, const char* cmdName);

/*
 * Validate a graph command name
 * Return NULL if the command does not lead to a valid node and set an appropriate error result
 */
Graph* Graphs_ValidateGraphCommand(GraphState* statePtr, Tcl_Interp* interp, const char* gName);

/*
 * Validate a node command name
 * Return NULL if the command does not lead to a valid node and set an appropriate error result
 */
Node* Graphs_ValidateNodeCommand(GraphState* statePtr, Tcl_Interp* interp, const char* nName);

/*
 * Validate a edge command name
 * Return NULL if the command does not lead to a valid node and set an appropriate error result
 */
Edge* Graphs_ValidateEdgeCommand(GraphState* statePtr, Tcl_Interp* interp, const char* gName);

/*
 * Add a node to a graph
 */
void Graphs_AddNodeToGraph(Graph* graphPtr, Node* nodePtr);

/*
 * Add a node to a graph
 */
void Graphs_DeleteNodeFromGraph(Graph* graphPtr, Node* nodePtr);

/*
 * Delete and free an edge.
 * Additionally, the edge is removed from statePtr and other node.
 */
void Graphs_DeleteEdge(Edge* nodePtr, Tcl_Interp* interp);

/*
 * Delete and free a node.
 * Additionally, all edges incident with that node are deleted and freed.
 */
void Graphs_DeleteNode(Node* nodePtr, Tcl_Interp* interp);

/*
 * Get and filter nodes
 */
int Graphs_GetNodes(Tcl_HashTable, LabelFilterT, Tcl_Interp*, int, Tcl_Obj* const []);

/*
 * \brief Checks the argument count for label filters.
 *
 * Places an appropriate error message in the interp result, if the objc count
 * is not correct and returns TCL_ERROR. Otherwise returns TCL_OK
 */
int Graphs_CheckLabelsOptions(LabelFilterT, Tcl_Interp*, int, Tcl_Obj* const[]);

/*
 * Common procedure to add/remove or get labels for nodes and edges
 */
int Graphs_LabelsCommand(Tcl_HashTable, Tcl_Interp*, int, Tcl_Obj* const []);

/*
 * Get delta (neighborhood) of a node or graph
 */
int Graphs_GetDelta(Node*, Graph*, DeltaT, struct LabelFilter, Tcl_Interp* interp, Tcl_Obj** resultObj);

#ifdef USE_TCL_STUBS
DLLEXPORT CONST char* Graphs_InitStubs _ANSI_ARGS_((Tcl_Interp* interp, CONST char* version, int exact));
#else
#	define Graphs_InitStubs(interp, version, exact) Tcl_PkgRequire(interp, "graphs", version, exact)
#endif
#include "graphsDecls.h"


#endif /* GRAPHS_H */
