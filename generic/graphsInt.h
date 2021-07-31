#ifndef GRAPHSINT_H
#define GRAPHSINT_H

#include "graphs.h"

int GraphsInt_CheckCommandExists(Tcl_Interp* interp, const char* cmdName);

int GraphsInt_MatchesLabels(const Tcl_HashTable* labels, const char* name, struct LabelFilter lblFilt, int* matchPtr);

int GraphsInt_GetNodes(Tcl_HashTable fromTbl, struct LabelFilter lblFilt, Tcl_Interp* interp);

int GraphsInt_GraphCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
void GraphsInt_GraphCleanupCmd(ClientData data);

int GraphsInt_NodeCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
void GraphsInt_NodeCleanupCmd(ClientData data);

int GraphsInt_EdgeCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
void GraphsInt_EdgeCleanupCmd(ClientData data);

/*
 * \brief Checks the argument count for label filters.
 *
 * Places an appropriate error message in the interp result, if the objc count
 * is not correct and returns TCL_ERROR. Otherwise returns TCL_OK
 */
int GraphsInt_CheckLabelsOptions(LabelFilterT, Tcl_Interp*, int, Tcl_Obj* const []);

/*
 * Common procedure to add/remove or get labels for nodes and edges
 */
int GraphsInt_LabelsCommand(Tcl_HashTable, Tcl_Interp*, int, Tcl_Obj* const []);

/*
 * Get delta (neighborhood) of a node or graph
 */
int GraphsInt_GetDelta(Node*, Graph*, DeltaT, struct LabelFilter, Tcl_Interp* interp, Tcl_Obj** resultObj);

#endif // GRAPHSINT_H
