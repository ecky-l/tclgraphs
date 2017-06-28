#ifndef GRAPH_H
#define GRAPH_H

#include <tcl.h>

typedef struct _graphState {
    Tcl_HashTable graphs;
    Tcl_HashTable nodes;
    Tcl_HashTable edges;
    int graphUid;
    int nodeUid;
    int edgeUid;
} GraphState;

typedef struct _graph {
    char cmdName[30];
    char* name;
    Tcl_Command commandTkn;
    GraphState* statePtr;
    Tcl_HashTable nodes;
} Graph;

typedef struct _node {
    char cmdName[30];
    char* name;
    GraphState* statePtr;
    Tcl_Command commandTkn;
    Tcl_HashTable edges;
    Tcl_HashTable tags;
} Node;

typedef struct _edge {
    char cmdName[30];
    char* name;
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

int Edge_CreateCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
void Edge_CleanupCmd(ClientData data);

#endif /* GRAPH_H */
