#ifndef GRAPHS_H
#define GRAPHS_H

#include <tcl.h>

#ifndef GRAPHSAPI
#   if defined(BUILD_graphs)
#	define GRAPHSAPI MODULE_SCOPE
#   else
#	define GRAPHSAPI extern
#	undef USE_GRAPHS_STUBS
#	define USE_GRAPHS_STUBS 1
#   endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BUILD_graphs) && !defined(STATIC_BUILD)
#   define GRAPHS_EXTERN extern DLLEXPORT
#else
#   define GRAPHS_EXTERN extern
#endif

GRAPHS_EXTERN int Graphs_Init(Tcl_Interp* interp);

typedef struct _deltaEntry DeltaEntry;
/* 
 * Marks that can be set wit the [graph mark], [edge mark] or [node mark] commands.
 * These are useful for filtering these entities in algorithms.
 */
typedef enum _GraphsMarkT {
    GRAPHS_MARK_HIDDEN = 0x01
} GraphsMarkT;

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
    Tcl_HashTable edges;
    int order;

    /* Arbitrary data that can be attached to the graph */
    Tcl_Obj* data;

    /* OR-combined flag of marks that can be set to the graph */
    unsigned int marks;
} Graph;

typedef struct _node
{
    char cmdName[30];
    char name[30];
    GraphState* statePtr;
    Tcl_Command commandTkn;

    /* Neighbor nodes reachable from this node. Map of nodes to edges */
    DeltaEntry* outgoing;

    /* Nodes with incoming edges, mapped to their edges. Mainly used for cleanup */
    DeltaEntry* incoming;

    /* labels assigned to the node. Can be used for filtering */
    Tcl_HashTable labels;

    /* graph where the node is part of */
    Graph* graph;

    /* additional arbitrary data attached to the node */
    Tcl_Obj* data;

    int degreeplus;
    int degreeminus;
    int degreeundir;

    /* OR-combined flag of marks that can be set to the graph */
    unsigned int marks;
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

    /* Arbitrary data that can be attached to the edge */
    Tcl_Obj* data;

    /* Labels for the edge */
    Tcl_HashTable labels;

    double weight;

    /* Indicates whether the edge is undirected. Default is directed, then this is 0 */
    EdgeDirectionT directionType;

    /* OR-combined flag of marks that can be set to the graph */
    unsigned int marks;
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
    Tcl_Obj* const* objv;
};

struct _deltaEntry
{
    Node* nodePtr;
    Edge* edgePtr;
    struct _deltaEntry* next;
};


#include "graphsDecls.h"

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif /* GRAPHS_H */
