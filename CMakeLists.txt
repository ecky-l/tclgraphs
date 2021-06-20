cmake_minimum_required(VERSION 3.10)

# set the project name
project(tclgraphs VERSION 0.9)

find_package(TCL)
find_package(Tclsh)
find_package(TclStub)

include_directories(${TCL_INCLUDE_PATH})

set(GRAPHS_SOURCES generic/common.c
                   generic/edge.c
                   generic/graph.c
                   generic/graphs.c
                   generic/node.c
                   generic/graphsStubInit.c)
set(GRAPHS_HEADERS generic/graphs.h
                   generic/graphsDecls.h)

set(TCLGRAPHS_DEFINES -DUSE_TCL_STUBS=1 
                      -DPACKAGE_NAME="graphs"
                      -DPACKAGE_VERSION="${CMAKE_PROJECT_VERSION}")

IF (WIN32)
	list(APPEND TCLGRAPHS_DEFINES -D_CRT_SECURE_NO_WARNINGS=1)
ENDIF()

add_library(graphs SHARED ${GRAPHS_SOURCES} ${GRAPHS_HEADERS} generic/graphs.decls)
add_custom_command(
	TARGET graphs
	PRE_BUILD
	COMMAND ${TCL_TCLSH} ARGS tools/genStubs.tcl generic generic/graphs.decls
	MAIN_DEPENDENCY generic/graphs.decls
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(graphs PRIVATE ${TCLGRAPHS_DEFINES} -DBUILD_graphs=1 -DMODULE_SCOPE=extern)
target_link_libraries(graphs ${TCL_STUB_LIBRARY})
set_target_properties(graphs PROPERTIES PREFIX "")

add_library(graphsstub STATIC generic/graphsStubLib.c)
target_compile_definitions(graphs PRIVATE ${TCLGRAPHS_DEFINES} -DBUILD_graphsstub=1)
set_target_properties(graphsstub PROPERTIES PREFIX "")
