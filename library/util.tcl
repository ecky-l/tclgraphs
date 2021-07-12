## util.tcl (created by Tloona here)
package require graphs @CMAKE_PROJECT_VERSION@

namespace eval ::graphs {
    namespace export get-or-create-node
}

## Get or create a node with given name
#
# Determines whether a node with a name already exists and returns it.
# Otherwise creates a new node with that name.
proc ::graphs::get-or-create-node {graph name} {
    if {[$graph info nodes -name $name] != {}} {
        $graph info nodes -name $name
    } else {
        ::graphs::node new -name $name -graph $graph
    }
}

package provide graphs::util @CMAKE_PROJECT_VERSION@
