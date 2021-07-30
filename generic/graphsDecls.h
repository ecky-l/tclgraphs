
/* !BEGIN!: Do not edit below this line. */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Exported function declarations:
 */

/* 0 */
GRAPHSAPI GraphState*	Graphs_GetState(Tcl_Interp*interp);
/* 1 */
GRAPHSAPI Graph*	Graphs_GraphGetByCommand(const GraphState*statePtr,
				const char*graphCmd);
/* 2 */
GRAPHSAPI Node*		Graphs_NodeGetByCommand(const GraphState*statePtr,
				const char*graphCmd);
/* Slot 3 is reserved */
/* 4 */
GRAPHSAPI void		Graphs_NodeAddToGraph(Graph*graphPtr, Node*nodePtr);
/* 5 */
GRAPHSAPI void		Graphs_NodeDeleteFromGraph(Graph*graphPtr,
				Node*nodePtr);
/* 6 */
GRAPHSAPI void		Graphs_NodeDeleteNode(Node*nodePtr,
				Tcl_Interp*interp);
/* 7 */
GRAPHSAPI Edge*		Graphs_EdgeGetEdge(const GraphState*statePtr,
				const Node*fromNodePtr, const Node*toNodePtr,
				int unDirected, unsigned int marksMask);
/* 8 */
GRAPHSAPI int		Graphs_EdgeHasMarks(const Edge*edgePtr,
				unsigned marksMask);
/* 9 */
GRAPHSAPI Edge*		Graphs_EdgeCreateEdge(GraphState*statePtr,
				Node*fromNodePtr, Node*toNodePtr,
				int undirected, Tcl_Interp*interp,
				const char*cmdName, int objc,
				Tcl_Obj* const objv[]);
/* 10 */
GRAPHSAPI void		Graphs_EdgeDeleteEdge(Edge*nodePtr,
				Tcl_Interp*interp);

typedef struct GraphsStubs {
    int magic;
    void *hooks;

    GraphState* (*graphs_GetState) (Tcl_Interp*interp); /* 0 */
    Graph* (*graphs_GraphGetByCommand) (const GraphState*statePtr, const char*graphCmd); /* 1 */
    Node* (*graphs_NodeGetByCommand) (const GraphState*statePtr, const char*graphCmd); /* 2 */
    void (*reserved3)(void);
    void (*graphs_NodeAddToGraph) (Graph*graphPtr, Node*nodePtr); /* 4 */
    void (*graphs_NodeDeleteFromGraph) (Graph*graphPtr, Node*nodePtr); /* 5 */
    void (*graphs_NodeDeleteNode) (Node*nodePtr, Tcl_Interp*interp); /* 6 */
    Edge* (*graphs_EdgeGetEdge) (const GraphState*statePtr, const Node*fromNodePtr, const Node*toNodePtr, int unDirected, unsigned int marksMask); /* 7 */
    int (*graphs_EdgeHasMarks) (const Edge*edgePtr, unsigned marksMask); /* 8 */
    Edge* (*graphs_EdgeCreateEdge) (GraphState*statePtr, Node*fromNodePtr, Node*toNodePtr, int undirected, Tcl_Interp*interp, const char*cmdName, int objc, Tcl_Obj* const objv[]); /* 9 */
    void (*graphs_EdgeDeleteEdge) (Edge*nodePtr, Tcl_Interp*interp); /* 10 */
} GraphsStubs;

extern const GraphsStubs *graphsStubsPtr;

#ifdef __cplusplus
}
#endif

#if defined(USE_GRAPHS_STUBS)

/*
 * Inline function declarations:
 */

#define Graphs_GetState \
	(graphsStubsPtr->graphs_GetState) /* 0 */
#define Graphs_GraphGetByCommand \
	(graphsStubsPtr->graphs_GraphGetByCommand) /* 1 */
#define Graphs_NodeGetByCommand \
	(graphsStubsPtr->graphs_NodeGetByCommand) /* 2 */
/* Slot 3 is reserved */
#define Graphs_NodeAddToGraph \
	(graphsStubsPtr->graphs_NodeAddToGraph) /* 4 */
#define Graphs_NodeDeleteFromGraph \
	(graphsStubsPtr->graphs_NodeDeleteFromGraph) /* 5 */
#define Graphs_NodeDeleteNode \
	(graphsStubsPtr->graphs_NodeDeleteNode) /* 6 */
#define Graphs_EdgeGetEdge \
	(graphsStubsPtr->graphs_EdgeGetEdge) /* 7 */
#define Graphs_EdgeHasMarks \
	(graphsStubsPtr->graphs_EdgeHasMarks) /* 8 */
#define Graphs_EdgeCreateEdge \
	(graphsStubsPtr->graphs_EdgeCreateEdge) /* 9 */
#define Graphs_EdgeDeleteEdge \
	(graphsStubsPtr->graphs_EdgeDeleteEdge) /* 10 */

#endif /* defined(USE_GRAPHS_STUBS) */

/* !END!: Do not edit above this line. */
