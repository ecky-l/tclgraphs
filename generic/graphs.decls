library graphs
interface graphs
scspec GRAPHSAPI

declare 0 generic {
	GraphState* Graphs_GetState(Tcl_Interp* interp)
}
declare 1 generic {
	Graph* Graphs_GraphGetByCommand(const GraphState* statePtr, const char* graphCmd)
}
declare 2 generic {
	Node* Graphs_NodeGetByCommand(const GraphState* statePtr, const char* graphCmd)
}
declare 3 generic {
	Edge* Graphs_EdgeGetEdge(const GraphState* statePtr, const Node* fromNodePtr, const Node* toNodePtr, int unDirected, unsigned int marksMask)
}
declare 4 generic {
	int Graphs_EdgeHasMarks(const Edge* edgePtr, unsigned marksMask)
}
