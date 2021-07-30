
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
/* 3 */
GRAPHSAPI void		Graphs_NodeAddToGraph(Graph*graphPtr, Node*nodePtr);
/* 4 */
GRAPHSAPI void		Graphs_NodeDeleteFromGraph(Graph*graphPtr,
				Node*nodePtr);
/* 5 */
GRAPHSAPI void		Graphs_NodeDeleteNode(Node*nodePtr,
				Tcl_Interp*interp);
/* 6 */
GRAPHSAPI Edge*		Graphs_EdgeGetEdge(const GraphState*statePtr,
				const Node*fromNodePtr, const Node*toNodePtr,
				int unDirected, unsigned int marksMask);
/* 7 */
GRAPHSAPI int		Graphs_EdgeHasMarks(const Edge*edgePtr,
				unsigned marksMask);
/* 8 */
GRAPHSAPI Edge*		Graphs_EdgeCreateEdge(GraphState*statePtr,
				Node*fromNodePtr, Node*toNodePtr,
				int undirected, Tcl_Interp*interp,
				const char*cmdName, int objc,
				Tcl_Obj* const objv[]);
/* 9 */
GRAPHSAPI void		Graphs_EdgeDeleteEdge(Edge*nodePtr,
				Tcl_Interp*interp);

typedef struct GraphsStubs {
    int magic;
    void *hooks;

    GraphState* (*graphs_GetState) (Tcl_Interp*interp); /* 0 */
    Graph* (*graphs_GraphGetByCommand) (const GraphState*statePtr, const char*graphCmd); /* 1 */
    Node* (*graphs_NodeGetByCommand) (const GraphState*statePtr, const char*graphCmd); /* 2 */
    void (*graphs_NodeAddToGraph) (Graph*graphPtr, Node*nodePtr); /* 3 */
    void (*graphs_NodeDeleteFromGraph) (Graph*graphPtr, Node*nodePtr); /* 4 */
    void (*graphs_NodeDeleteNode) (Node*nodePtr, Tcl_Interp*interp); /* 5 */
    Edge* (*graphs_EdgeGetEdge) (const GraphState*statePtr, const Node*fromNodePtr, const Node*toNodePtr, int unDirected, unsigned int marksMask); /* 6 */
    int (*graphs_EdgeHasMarks) (const Edge*edgePtr, unsigned marksMask); /* 7 */
    Edge* (*graphs_EdgeCreateEdge) (GraphState*statePtr, Node*fromNodePtr, Node*toNodePtr, int undirected, Tcl_Interp*interp, const char*cmdName, int objc, Tcl_Obj* const objv[]); /* 8 */
    void (*graphs_EdgeDeleteEdge) (Edge*nodePtr, Tcl_Interp*interp); /* 9 */
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
#define Graphs_NodeAddToGraph \
	(graphsStubsPtr->graphs_NodeAddToGraph) /* 3 */
#define Graphs_NodeDeleteFromGraph \
	(graphsStubsPtr->graphs_NodeDeleteFromGraph) /* 4 */
#define Graphs_NodeDeleteNode \
	(graphsStubsPtr->graphs_NodeDeleteNode) /* 5 */
#define Graphs_EdgeGetEdge \
	(graphsStubsPtr->graphs_EdgeGetEdge) /* 6 */
#define Graphs_EdgeHasMarks \
	(graphsStubsPtr->graphs_EdgeHasMarks) /* 7 */
#define Graphs_EdgeCreateEdge \
	(graphsStubsPtr->graphs_EdgeCreateEdge) /* 8 */
#define Graphs_EdgeDeleteEdge \
	(graphsStubsPtr->graphs_EdgeDeleteEdge) /* 9 */

#endif /* defined(USE_GRAPHS_STUBS) */

/* !END!: Do not edit above this line. */
