#include "graphs.h"

MODULE_SCOPE const GraphsStubs graphsStubs;

/* !BEGIN!: Do not edit below this line. */

const GraphsStubs graphsStubs = {
    TCL_STUB_MAGIC,
    0,
    Graphs_GetState, /* 0 */
    Graphs_GraphGetByCommand, /* 1 */
    Graphs_NodeGetByCommand, /* 2 */
    0, /* 3 */
    Graphs_NodeAddToGraph, /* 4 */
    Graphs_NodeDeleteFromGraph, /* 5 */
    Graphs_NodeDeleteNode, /* 6 */
    Graphs_EdgeGetEdge, /* 7 */
    Graphs_EdgeHasMarks, /* 8 */
    Graphs_EdgeCreateEdge, /* 9 */
    Graphs_EdgeDeleteEdge, /* 10 */
};

/* !END!: Do not edit above this line. */
