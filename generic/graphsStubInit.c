#include "graphs.h"

MODULE_SCOPE const GraphsStubs graphsStubs;

/* !BEGIN!: Do not edit below this line. */

const GraphsStubs graphsStubs = {
    TCL_STUB_MAGIC,
    0,
    Graphs_GetState, /* 0 */
    Graphs_GraphGetByCommand, /* 1 */
    Graphs_NodeGetByCommand, /* 2 */
    Graphs_NodeAddToGraph, /* 3 */
    Graphs_NodeDeleteFromGraph, /* 4 */
    Graphs_NodeDeleteNode, /* 5 */
    Graphs_EdgeGetEdge, /* 6 */
    Graphs_EdgeHasMarks, /* 7 */
    Graphs_EdgeCreateEdge, /* 8 */
    Graphs_EdgeDeleteEdge, /* 9 */
};

/* !END!: Do not edit above this line. */
