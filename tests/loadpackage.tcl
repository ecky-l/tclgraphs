## loadpackage.tcl (created by Tloona here)
if {$tcl_platform(platform) eq "windows"} {
    load libgraphs0.9.dll
}
package require graphs
namespace import graphs::*
