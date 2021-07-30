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
	void Graphs_NodeAddToGraph(Graph* graphPtr, Node* nodePtr)
}
declare 4 generic {
	void Graphs_NodeDeleteFromGraph(Graph* graphPtr, Node* nodePtr)
}
declare 5 generic {
	void Graphs_NodeDeleteNode(Node* nodePtr, Tcl_Interp* interp)
}
declare 6 generic {
	Edge* Graphs_EdgeGetEdge(const GraphState* statePtr, const Node* fromNodePtr, const Node* toNodePtr, int unDirected, unsigned int marksMask)
}
declare 7 generic {
	int Graphs_EdgeHasMarks(const Edge* edgePtr, unsigned marksMask)
}
declare 8 generic {
	Edge* Graphs_EdgeCreateEdge(GraphState* statePtr, Node* fromNodePtr, Node* toNodePtr, int undirected, Tcl_Interp* interp, const char* cmdName, int objc, Tcl_Obj* const objv[])
}
declare 9 generic {
	void Graphs_EdgeDeleteEdge(Edge* nodePtr, Tcl_Interp* interp)
}
