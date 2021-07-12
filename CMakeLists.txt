cmake_minimum_required(VERSION 3.16)

# set the project name
project(tclgraphs VERSION 0.9.0)

find_package(TCL)
find_package(Tclsh)
find_package(TclStub)

#
# Common defines for all project targets
#
include_directories(${TCL_INCLUDE_PATH})
set(TCLGRAPHS_DEFINES -DUSE_TCL_STUBS=1
                      -DPACKAGE_NAME="graphs"
                      -DPACKAGE_VERSION="${CMAKE_PROJECT_VERSION}")

IF (WIN32)
	list(APPEND TCLGRAPHS_DEFINES -D_CRT_SECURE_NO_WARNINGS=1)
ENDIF()

#
# graphs Tcl extension
#
set(GRAPHS_SOURCES generic/common.c
                   generic/edge.c
                   generic/graph.c
                   generic/graphs.c
                   generic/node.c
                   generic/graphsStubInit.c)
set(GRAPHS_HEADERS generic/graphs.h
                   generic/graphsDecls.h)
add_library(graphs SHARED ${GRAPHS_SOURCES} ${GRAPHS_HEADERS} generic/graphs.decls pkgIndex.tcl.in)
add_custom_command(
	TARGET graphs
	PRE_BUILD
	COMMAND ${TCL_TCLSH} ARGS tools/genStubs.tcl generic generic/graphs.decls
	MAIN_DEPENDENCY generic/graphs.decls
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(graphs PRIVATE ${TCLGRAPHS_DEFINES} -DBUILD_graphs=1 -DMODULE_SCOPE=extern)
target_link_libraries(graphs ${TCL_STUB_LIBRARY})
set_target_properties(graphs PROPERTIES PREFIX "")

#
# graphsstub library for other Tcl extensions
#
add_library(graphsstub STATIC generic/graphsStubLib.c)
target_compile_definitions(graphs PRIVATE ${TCLGRAPHS_DEFINES} -DBUILD_graphsstub=1)
if(UNIX)
    target_compile_options(graphs PRIVATE -O2 -fomit-frame-pointer -DNDEBUG -Wall -pipe)
endif(UNIX)
set_target_properties(graphsstub PROPERTIES POSITION_INDEPENDENT_CODE ON)

#
# The tests
#
enable_testing()
add_test(
    NAME "graph-testsuite"
    COMMAND ${TCL_TCLSH} tests/all.tcl -load $<TARGET_FILE:graphs> -dir ${CMAKE_CURRENT_BINARY_DIR} -tests graph
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_test(
    NAME "node-testsuite"
    COMMAND ${TCL_TCLSH} tests/all.tcl -load $<TARGET_FILE:graphs> -dir ${CMAKE_CURRENT_BINARY_DIR} -tests node
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_test(
    NAME "edge-testsuite"
    COMMAND ${TCL_TCLSH} tests/all.tcl -load $<TARGET_FILE:graphs> -dir ${CMAKE_CURRENT_BINARY_DIR} -tests edge
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_test(
    NAME "util-testsuite"
    COMMAND ${TCL_TCLSH} tests/all.tcl -load $<TARGET_FILE:graphs> -dir ${CMAKE_CURRENT_BINARY_DIR} -tests util
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


#
# process library files, substitute version
#
configure_file(pkgIndex.tcl.in pkgIndex.tcl)
set(LIBRARY_FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl")
file(GLOB libraryFiles "library/*.tcl")
foreach(file ${libraryFiles})
    cmake_path(GET file FILENAME outFile)
    list(APPEND LIBRARY_FILES "${CMAKE_CURRENT_BINARY_DIR}/${outFile}")
    configure_file(${file} "${outFile}")
endforeach()

#
# install section for packaging
#

set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local/tcl/lib")
file(TO_CMAKE_PATH ${CMAKE_INSTALL_PREFIX} CMAKE_INSTALL_PREFIX)
message(STATUS "install prefix: ${CMAKE_INSTALL_PREFIX}")
install(
    TARGETS graphs
    RUNTIME DESTINATION "graphs${CMAKE_PROJECT_VERSION}"
    LIBRARY DESTINATION "graphs${CMAKE_PROJECT_VERSION}"
)
install(
    FILES ${GRAPHS_HEADERS}
    DESTINATION "graphs${CMAKE_PROJECT_VERSION}/include"
)
install(
    #FILES "${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl"
    FILES ${LIBRARY_FILES}
    DESTINATION "graphs${CMAKE_PROJECT_VERSION}"
)
install(
    TARGETS graphsstub
    ARCHIVE DESTINATION "graphs${CMAKE_PROJECT_VERSION}/lib"
)
install(
    FILES cmake/FindTclGraphs.cmake
    DESTINATION "graphs${CMAKE_PROJECT_VERSION}/cmake"
)

#
# packaging section
#
IF (WIN32)
    set(CPACK_GENERATOR "ZIP")
ELSE()
    set(CPACK_GENERATOR "TGZ")
ENDIF()

set(CPACK_PACKAGE_NAME "graphs")
set(CPACK_PACKAGE_VENDOR "EL")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 9)
set(CPACK_PACKAGE_VERSION_PATCH 0)
include(CPack)
