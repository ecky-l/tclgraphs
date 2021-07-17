
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
GRAPHSAPI Graph*	Graphs_GraphGetByCommand(GraphState*statePtr,
				const char*graphCmd);
/* 2 */
GRAPHSAPI Node*		Graphs_NodeGetByCommand(GraphState*statePtr,
				const char*graphCmd);
/* 3 */
GRAPHSAPI Edge*		Graphs_EdgeGetEdge(GraphState*statePtr,
				CONST Node*fromNodePtr, CONST Node*toNodePtr,
				int unDirected, unsigned int marksMask);
/* 4 */
GRAPHSAPI int		Graphs_EdgeHasMarks(Edge*edgePtr, unsigned marksMask);

typedef struct GraphsStubs {
    int magic;
    void *hooks;

    GraphState* (*graphs_GetState) (Tcl_Interp*interp); /* 0 */
    Graph* (*graphs_GraphGetByCommand) (GraphState*statePtr, const char*graphCmd); /* 1 */
    Node* (*graphs_NodeGetByCommand) (GraphState*statePtr, const char*graphCmd); /* 2 */
    Edge* (*graphs_EdgeGetEdge) (GraphState*statePtr, CONST Node*fromNodePtr, CONST Node*toNodePtr, int unDirected, unsigned int marksMask); /* 3 */
    int (*graphs_EdgeHasMarks) (Edge*edgePtr, unsigned marksMask); /* 4 */
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
#define Graphs_EdgeGetEdge \
	(graphsStubsPtr->graphs_EdgeGetEdge) /* 3 */
#define Graphs_EdgeHasMarks \
	(graphsStubsPtr->graphs_EdgeHasMarks) /* 4 */

#endif /* defined(USE_GRAPHS_STUBS) */

/* !END!: Do not edit above this line. */
