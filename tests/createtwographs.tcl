## createtwographs.tcl (created by Tloona here)
foreach g {g1 g2} {graph create $g}
foreach n {n1 n2 n3 n4} {node create $n}
n1 configure -graph g1
n2 configure -graph g1
n3 configure -graph g2
n4 configure -graph g2
