
/* !BEGIN!: Do not edit below this line. */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Exported function declarations:
 */

/* 0 */
EXTERN GraphState*	Graphs_GetState(Tcl_Interp*interp);

typedef struct GraphsStubs {
    int magic;
    void *hooks;

    GraphState* (*graphs_GetState) (Tcl_Interp*interp); /* 0 */
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

#endif /* defined(USE_GRAPHS_STUBS) */

/* !END!: Do not edit above this line. */
