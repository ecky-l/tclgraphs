#
# creating dot files from graphs objects
#
package require graphs @CMAKE_PROJECT_VERSION@
package require graphs::util @CMAKE_PROJECT_VERSION@

namespace eval graphs {
    namespace export digraph-to-dot diedge-to-dot digraph-from-dot
}

proc ::graphs::digraph-to-dot {graph args} {
    set pretty [expr {[dict exists $args -pretty] && [dict get $args -pretty]}]

    if {[dict exists $args -name]} {
        append result "digraph [dict get $args -name] \{"
    } else {
        append result "digraph \{"
    }
    if {$pretty} {
        append result \n
    }

    foreach {edge} [$graph info edges] {
        set markArgs {}
        if {[dict exists $args -mark]} {
            if {[lsearch [dict get $args -mark] $edge] >= 0} {
                set markArgs { color=red penwidth=2.0 }
            }
        }
        append result [diedge-to-dot $edge {*}$markArgs] \;
        if {$pretty} {
            append result \n
        }
    }
    append result "\}"
}

proc ::graphs::diedge-to-dot {edge args} {
    set from [$edge cget -from]
    set to [$edge cget -to]
    set weight [$edge cget -weight]
    set data [$edge cget -data]
    if {[$from cget -name] == "" || [$to cget -name] == ""} {
        throw {GRAPHS EDGE_TO_DOT} "from and to nodes must have names"
    }
    append result \" [$from cget -name] \" -> \" [$to cget -name] \"
    append result \[ label = \" [$edge cget -name] \" , weight = \" $weight \" , data= \" $data \"
    if {$args != {}} {
        append result , [join $args ,]
    }
    append result \]
}

proc ::graphs::digraph-from-dot {graph dot} {

    set start [expr {[string first "\{" $dot] + 1}]
    set end [expr {[string last "\}" $dot] - 1}]

    set graphTkn [string range $dot 0 [expr {$start - 2}]]
    set name [if {[llength $graphTkn] > 1} {
        lindex $graphTkn 1
    } else {
        list
    }]

    set result {}
    set edgesStr [lrange [split [string range $dot $start $end] \;] 0 end-1]
    foreach {edge} $edgesStr {
        set edge [string trim $edge]
        set attrStart [expr {[string first "\[" $edge] + 1}]
        set attrEnd [expr {[string last "\]" $edge]} - 1]
        set edgeDesc [string range $edge 0 [expr {$attrStart - 2}]]
        set attrStr [string range $edge $attrStart $attrEnd]
        foreach {attr} [split $attrStr ,] {
            set key -[lindex [split $attr =] 0]
            set val [lindex [split $attr =] 1]
            dict set attrs $key [string trim $val \"]
        }
        set nodes [split [regsub {\->} $edgeDesc ,] ,]
        set from [get-or-create-node $graph [string trim [lindex $nodes 0] \"]]
        set to [get-or-create-node $graph [string trim [lindex $nodes 1] \"]]
        set e [::graphs::edge new $from -> $to]
        foreach {key} { -weight -data } {
            if {[dict exists $attrs $key]} {
                $e configure $key [dict get $attrs $key]
            }
        }
        if {[dict exists $attrs -label]} {
            $e configure -name [dict get $attrs -label]
        }
    }

    return $name
}

package provide graphs::dot @CMAKE_PROJECT_VERSION@
