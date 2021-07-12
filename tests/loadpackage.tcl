## loadpackage.tcl (created by Tloona here)
load [dict get $argv -load]
lappend auto_path [dict get $argv -dir]
package require graphs
namespace import graphs::*
