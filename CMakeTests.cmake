# Copyright 2017-2022 Uber Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Test code for H3

option(BUILD_ALLOC_TESTS "Build tests for custom allocation functions" ON)
option(PRINT_TEST_FILES "Print which test files correspond to which tests" OFF)

include(TestWrapValgrind)

enable_testing()

# Macros and support code needed to build and add the tests
set(test_number 0)

if(ENABLE_COVERAGE)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/scripts/coverage.sh"
                    INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/coverage.sh.in")
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage.info")
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage.cleaned.info")
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage")
    add_custom_target(coverage
        COMMAND bash "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/scripts/coverage.sh" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_target(clean-coverage
        # Before running coverage, clear all counters
        COMMAND lcov --rc lcov_branch_coverage=1 --directory '${CMAKE_CURRENT_BINARY_DIR}' --zerocounters
        COMMENT "Zeroing counters"
        )
endif()

macro(add_h3_memory_test name srcfile)
    # Like other test code, but these need to be linked against
    # a different copy of the H3 library which has known intercepted
    # allocator functions.
    add_executable(${ARGV} ${APP_SOURCE_FILES} ${TEST_APP_SOURCE_FILES})

    if(TARGET ${name})
        target_link_libraries(${name} PUBLIC h3WithTestAllocators)
        target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/apps/applib/include>)
        target_compile_options(${name} PRIVATE ${H3_COMPILE_FLAGS})
        target_link_libraries(${name} PRIVATE ${H3_LINK_FLAGS})
    endif()

    math(EXPR test_number "${test_number}+1")

    add_test(NAME ${name}_test${test_number} COMMAND ${TEST_WRAPPER} "$<TARGET_FILE:${name}>")

    if(ENABLE_COVERAGE)
        add_custom_target(${name}_coverage${test_number}
            COMMAND ${name} > /dev/null
            COMMENT "Running ${name}_coverage${test_number}"
            )

        add_dependencies(coverage ${name}_coverage${test_number})
        add_dependencies(${name}_coverage${test_number} clean-coverage)
    endif()
endmacro()

macro(add_h3_test_common name srcfile)
    # need to actually make the test target
    if(NOT TARGET ${name})
        add_h3_executable(${name} ${srcfile} ${APP_SOURCE_FILES} ${TEST_APP_SOURCE_FILES})
    endif()

    math(EXPR test_number "${test_number}+1")
endmacro()

macro(add_h3_test name srcfile)
    add_h3_test_common(${name} ${srcfile})
    add_test(NAME ${name}_test${test_number} COMMAND ${TEST_WRAPPER} "$<TARGET_FILE:${name}>")

    if(ENABLE_COVERAGE)
        add_custom_target(${name}_coverage${test_number}
            COMMAND ${name} > /dev/null
            COMMENT "Running ${name}_coverage${test_number}"
            )

        add_dependencies(coverage ${name}_coverage${test_number})
        add_dependencies(${name}_coverage${test_number} clean-coverage)
    endif()
endmacro()

macro(add_h3_test_with_file name srcfile argfile)
    add_h3_test_common(${name} ${srcfile})
    # add a special command (so we don't need to read the test file from the test program)
    set(dump_command "cat")

    add_test(NAME ${name}_test${test_number}
                COMMAND ${SHELL} "${dump_command} ${argfile} | ${TEST_WRAPPER_STR} $<TARGET_FILE:${name}>")

    if(PRINT_TEST_FILES)
        message("${name}_test${test_number} - ${argfile}")
    endif()

    if(ENABLE_COVERAGE)
        add_custom_target(${name}_coverage${test_number}
            COMMAND ${name} < ${argfile} > /dev/null
            COMMENT "Running ${name}_coverage${test_number}"
            )

        add_dependencies(coverage ${name}_coverage${test_number})
        add_dependencies(${name}_coverage${test_number} clean-coverage)
    endif()
endmacro()

macro(add_h3_cli_test name h3_args expect_string)
    add_test(NAME ${name}_test${test_number}
        COMMAND ${SHELL} "test \"`$<TARGET_FILE:h3_bin> ${h3_args}`\" = '${expect_string}'")

    if(PRINT_TEST_FILES)
        message("${name}_test${test_number} - ${h3_args} - ${expect_string}")
    endif()

    # TODO: Build a coverage-enabled variant of the h3 cli app to enable coverage
endmacro()

macro(add_h3_test_with_arg name srcfile arg)
    add_h3_test_common(${name} ${srcfile})
    add_test(NAME ${name}_test${test_number}
        COMMAND ${TEST_WRAPPER} $<TARGET_FILE:${name}> ${arg}
        )
    if(PRINT_TEST_FILES)
        message("${name}_test${test_number} - ${arg}")
    endif()

    if(ENABLE_COVERAGE)
        add_custom_target(${name}_coverage${test_number}
            COMMAND ${name} ${arg}
            COMMENT "Running ${name}_coverage${test_number}"
            )

        add_dependencies(coverage ${name}_coverage${test_number})
        add_dependencies(${name}_coverage${test_number} clean-coverage)
    endif()
endmacro()

# Add each individual test
file(GLOB all_centers tests/inputfiles/bc*centers.txt)
foreach(file ${all_centers})
    add_h3_test_with_file(testLatLngToCell src/apps/testapps/testLatLngToCell.c ${file})
endforeach()

file(GLOB all_ic_files tests/inputfiles/res*ic.txt)
foreach(file ${all_ic_files})
    add_h3_test_with_file(testCellToLatLng src/apps/testapps/testCellToLatLng.c ${file})
endforeach()

file(GLOB all_centers tests/inputfiles/rand*centers.txt)
foreach(file ${all_centers})
    add_h3_test_with_file(testLatLngToCell src/apps/testapps/testLatLngToCell.c ${file})
endforeach()

file(GLOB all_cells tests/inputfiles/*cells.txt)
foreach(file ${all_cells})
    add_h3_test_with_file(testCellToBoundary src/apps/testapps/testCellToBoundary.c ${file})
endforeach()

add_h3_test(testCellToBoundaryEdgeCases src/apps/testapps/testCellToBoundaryEdgeCases.c)
add_h3_test(testCompactCells src/apps/testapps/testCompactCells.c)
add_h3_test(testGridDisk src/apps/testapps/testGridDisk.c)
add_h3_test(testGridRingUnsafe src/apps/testapps/testGridRingUnsafe.c)
add_h3_test(testGridDisksUnsafe src/apps/testapps/testGridDisksUnsafe.c)
add_h3_test(testCellToParent src/apps/testapps/testCellToParent.c)
add_h3_test(testCellToCenterChild src/apps/testapps/testCellToCenterChild.c)
add_h3_test(testCellToChildren src/apps/testapps/testCellToChildren.c)
add_h3_test(testCellToChildPos src/apps/testapps/testCellToChildPos.c)
add_h3_test(testGetIcosahedronFaces src/apps/testapps/testGetIcosahedronFaces.c)
add_h3_test(testCellToChildrenSize src/apps/testapps/testCellToChildrenSize.c)
add_h3_test(testH3Index src/apps/testapps/testH3Index.c)
add_h3_test(testH3Api src/apps/testapps/testH3Api.c)
add_h3_test(testCellsToLinkedMultiPolygon src/apps/testapps/testCellsToLinkedMultiPolygon.c)
add_h3_test(testH3SetToVertexGraph src/apps/testapps/testH3SetToVertexGraph.c)
add_h3_test(testLinkedGeo src/apps/testapps/testLinkedGeo.c)
add_h3_test(testPolygonToCells src/apps/testapps/testPolygonToCells.c)
add_h3_test(testPolygonToCellsReported src/apps/testapps/testPolygonToCellsReported.c)
add_h3_test(testVertexGraph src/apps/testapps/testVertexGraph.c)
add_h3_test(testDirectedEdge src/apps/testapps/testDirectedEdge.c)
add_h3_test(testLatLng src/apps/testapps/testLatLng.c)
add_h3_test(testBBox src/apps/testapps/testBBox.c)
add_h3_test(testVertex src/apps/testapps/testVertex.c)
add_h3_test(testPolygon src/apps/testapps/testPolygon.c)
add_h3_test(testVec2d src/apps/testapps/testVec2d.c)
add_h3_test(testVec3d src/apps/testapps/testVec3d.c)
add_h3_test(testCellToLocalIj src/apps/testapps/testCellToLocalIj.c)
add_h3_test(testGridDistance src/apps/testapps/testGridDistance.c)
add_h3_test(testGridPathCells src/apps/testapps/testGridPathCells.c)
add_h3_test(testH3CellArea src/apps/testapps/testH3CellArea.c)
add_h3_test(testCoordIj src/apps/testapps/testCoordIj.c)
add_h3_test(testCoordIjk src/apps/testapps/testCoordIjk.c)
add_h3_test(testBaseCells src/apps/testapps/testBaseCells.c)
add_h3_test(testPentagonIndexes src/apps/testapps/testPentagonIndexes.c)
add_h3_test(testH3Iterators src/apps/testapps/testH3Iterators.c)
add_h3_test(testMathExtensions src/apps/testapps/testMathExtensions.c)

add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 0)
add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 1)
add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 2)

# The "Exhaustive" part of the test name is used by the test-fast to exclude these files.
# test-fast exists so that Travis CI can run Valgrind on tests without taking a very long time.
add_h3_test(testDirectedEdgeExhaustive src/apps/testapps/testDirectedEdgeExhaustive.c)
add_h3_test(testVertexExhaustive src/apps/testapps/testVertexExhaustive.c)
add_h3_test(testCellToLocalIjExhaustive src/apps/testapps/testCellToLocalIjExhaustive.c)
add_h3_test(testGridPathCellsExhaustive src/apps/testapps/testGridPathCellsExhaustive.c)
add_h3_test(testGridDistanceExhaustive src/apps/testapps/testGridDistanceExhaustive.c)
add_h3_test(testH3CellAreaExhaustive src/apps/testapps/testH3CellAreaExhaustive.c)

add_h3_cli_test(testCliCellToLatLng "cellToLatLng -c 8928342e20fffff" "37.5012466151, -122.5003039349")
add_h3_cli_test(testCliLatLngToCell "latLngToCell --lat 20 --lng 123 -r 2" "824b9ffffffffff")

if(BUILD_ALLOC_TESTS)
    add_h3_library(h3WithTestAllocators test_prefix_)

    add_h3_memory_test(testH3Memory src/apps/testapps/testH3Memory.c)
endif()

add_custom_target(test-fast COMMAND ctest -E Exhaustive)
