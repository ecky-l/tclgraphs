## loadpackage.tcl (created by Tloona here)
if {$tcl_platform(platform) eq "windows"} {
    if {$::tcl_platform(pointerSize) == 4} {
        load libtclgraphs0.8-win32.dll
    } else {
        load libtclgraphs0.8-win64.dll
    }
}
package require tclgraphs
namespace import tclgraphs::*
