library graphs
interface graphs
scspec GRAPHSAPI

declare 0 generic {
	GraphState* Graphs_GetState(Tcl_Interp* interp)
}

declare 1 generic {
	Graph* Graphs_GraphGetByCommand(GraphState* statePtr, const char* graphCmd)
}

declare 2 generic {
	Node* Graphs_NodeGetByCommand(GraphState* statePtr, const char* graphCmd)
}
