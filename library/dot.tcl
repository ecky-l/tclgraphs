#
# creating dot files from graphs objects
#
namespace eval graphs {
    namespace export graph-to-dot edge-to-dot graph-from-dot
}

proc ::graphs::graph-to-dot {graph args} {
    if {[dict exists $args -name]} {
        append result "digraph [dict get $args -name] \{"
    } else {
        append result "digraph \{"
    }
    foreach {edge} [$graph info edges] {
        set markArgs {}
        if {[dict exists $args -mark]} {
            if {[lsearch [dict get $args -mark] $edge] >= 0} {
                set markArgs { color=red penwidth=2.0 }
            }
        }
        append result [edge-to-dot $edge {*}$markArgs] \;
    }
    append result "\}"
}

proc ::graphs::edge-to-dot {edge args} {
    set from [$edge cget -from]
    set to [$edge cget -to]
    set weight [$edge cget -weight]
    set data [$edge cget -data]
    if {[$from cget -name] == "" || [$to cget -name] == ""} {
        throw {GRAPHS EDGE_TO_DOT} "from and to nodes must have names"
    }
    append result [$from cget -name]->[$to cget -name]
    append result \[ label = $weight , weight = $weight , data= \" $data \"
    if {$args != {}} {
        append result , [join $args ,]
    }
    append result \]
}

proc ::graphs::graph-from-dot {graph dot} {

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
        set from [GetNodeByNameOrNew $graph [lindex $nodes 0]]
        set to [GetNodeByNameOrNew $graph [lindex $nodes 1]]
        set e [::graphs::edge new $from -> $to]
        foreach {key} { -weight -data } {
            if {[dict exists $attrs $key]} {
                $e configure $key [dict get $attrs $key]
            }
        }
    }

    return $name
}


proc ::graphs::GetNodeByNameOrNew {graph name} {
    if {[$graph info nodes -name $name] != {}} {
        $graph info nodes -name $name
    } else {
        ::graphs::node new -name $name -graph $graph
    }
}
