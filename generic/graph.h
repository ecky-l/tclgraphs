#ifndef GRAPH_H
#define GRAPH_H

#include <tcl.h>

typedef struct _graphState {
    Tcl_Interp* interp;
    Tcl_HashTable graphs;
    Tcl_HashTable nodes;
    Tcl_HashTable edges;
    int graphUid;
    int nodeUid;
    int edgeUid;
} GraphState;

typedef struct _graph {
    char cmdName[30];
    char name[30];
    Tcl_Command commandTkn;
    GraphState* statePtr;
    Tcl_HashTable nodes;
} Graph;

typedef struct _node {
    char cmdName[30];
    char name[30];
    GraphState* statePtr;
    Tcl_Command commandTkn;
    Tcl_HashTable neighbors;
    Tcl_HashTable tags;
    Tcl_HashTable graphs;
} Node;

typedef struct _edge {
    char cmdName[30];
    char name[30];
    GraphState* statePtr;
    Tcl_Command commandTkn;
    Node* fromNode;
    Node* toNode;
    double weight;
} Edge;

int Graph_CreateCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
void Graph_CleanupCmd(ClientData data);

int Node_CreateCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
void Node_CleanupCmd(ClientData data);

int Edge_EdgeCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
void Edge_CleanupCmd(ClientData data);
Edge* Edge_CreateEdge(GraphState*, Node*, Node*, int, Tcl_Interp*, const char*, int, Tcl_Obj* const[]);
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
 * Delete and free an edge.
 * Additionally, the edge is removed from statePtr and other node.
 */
void Graphs_DeleteEdge(Edge* nodePtr, Tcl_Interp* interp);

/*
 * Delete and free a node.
 * Additionally, all edges incident with that node are deleted and freed.
 */
void Graphs_DeleteNode(Node* nodePtr, Tcl_Interp* interp);

#endif /* GRAPH_H */
