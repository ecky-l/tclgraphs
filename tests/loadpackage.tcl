## loadpackage.tcl (created by Tloona here)
load [lindex $argv 0]
puts dfgfdgfg,[file dirname [info script]]
source [file join [file dirname [info script]] .. library util.tcl]
source [file join [file dirname [info script]] .. library dot.tcl]
package require graphs
namespace import graphs::*
