## util.tcl (created by Tloona here)
package require graphs @CMAKE_PROJECT_VERSION@

namespace eval ::graphs {
    namespace export get-or-create-node clone-edges-to-graph
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

## clone edges into a new graph.
#
# A new edge with connected nodes is created in targetGraph for each edge in the list.
# The args can contain a -post function, which is applied after cloning to each new edge.
proc ::graphs::clone-edges-to-graph {edges targetGraph args} {
    foreach {e} $edges {
        set from [get-or-create-node $targetGraph [[$e cget -from] cget -name]]
        set to [get-or-create-node $targetGraph [[$e cget -to] cget -name]]
        set edge [edge new $from -> $to -name [$e cget -name] \
            -weight [$e cget -weight] -data [$e cget -data]]
        $edge labels + {*}[$e labels]
        if {[dict exists $args -post]} {
            apply [dict get $args -post] $edge
        }
    }
    return $targetGraph
}

package provide graphs::util @CMAKE_PROJECT_VERSION@
