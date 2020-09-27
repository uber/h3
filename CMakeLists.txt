# Copyright 2017-2020 Uber Technologies, Inc.
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

cmake_minimum_required(VERSION 3.1)

set(H3_PREFIX "" CACHE STRING "Prefix for exported symbols")
set(H3_ALLOC_PREFIX "" CACHE STRING "Prefix for allocation functions")

# Needed due to CMP0042
set(CMAKE_MACOSX_RPATH 1)
# H3 doesn't export selected symbols right now
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# YCM needs compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain.cmake"
    CACHE FILEPATH
    "Toolchain to use for building this project")

option(ENABLE_COVERAGE "Enable compiling tests with coverage." ON)
option(BUILD_BENCHMARKS "Build benchmarking applications." ON)
option(BUILD_FILTERS "Build filter applications." ON)
option(BUILD_GENERATORS "Build code generation applications." ON)

if(WIN32)
    # Use bash (usually from Git for Windows) for piping results
    set(SHELL bash -c)

    set(EXECUTABLE_OUTPUT_PATH bin)
    set(LIBRARY_OUTPUT_PATH bin)
else()
    set(SHELL sh -c)

    set(EXECUTABLE_OUTPUT_PATH bin)
    set(LIBRARY_OUTPUT_PATH lib)
endif()

file(READ VERSION H3_VERSION LIMIT_COUNT 1)
# Clean any newlines
string(REPLACE "\n" "" H3_VERSION "${H3_VERSION}")
string(REPLACE "." ";" H3_VERSION_LIST "${H3_VERSION}")
list(GET H3_VERSION_LIST 0 H3_VERSION_MAJOR)
list(GET H3_VERSION_LIST 1 H3_VERSION_MINOR)
list(GET H3_VERSION_LIST 2 H3_VERSION_PATCH)
set(H3_SOVERSION 1)

project(h3 LANGUAGES C VERSION ${H3_VERSION})

set(H3_COMPILE_FLAGS "")
set(H3_LINK_FLAGS "")
if(NOT WIN32)
    # Compiler options are set only on non-Windows, since these options
    # are not correct for MSVC.
    list(APPEND H3_COMPILE_FLAGS -Wall)

    list(APPEND H3_COMPILE_FLAGS $<$<CONFIG:Debug>:-gdwarf-2 -g3 -O0 -fno-inline -fno-eliminate-unused-debug-types>)

    if(ENABLE_COVERAGE)
        list(APPEND H3_COMPILE_FLAGS $<$<CONFIG:Debug>:--coverage>)
        # --coverage is not passed to the linker, so this option is needed
        # to fully enable coverage.
        list(APPEND H3_LINK_FLAGS $<$<CONFIG:Debug>:--coverage>)
    endif()

    option(WARNINGS_AS_ERRORS "Warnings are treated as errors" OFF)
    if(WARNINGS_AS_ERRORS)
        list(APPEND H3_COMPILE_FLAGS -Werror)
    endif()
endif()

include(CMakeDependentOption)
include(CheckIncludeFile)
include(CTest)

set(LIB_SOURCE_FILES
    src/h3lib/include/bbox.h
    src/h3lib/include/polygon.h
    src/h3lib/include/polygonAlgos.h
    src/h3lib/include/h3Index.h
    src/h3lib/include/h3UniEdge.h
    src/h3lib/include/geoCoord.h
    src/h3lib/include/vec2d.h
    src/h3lib/include/vec3d.h
    src/h3lib/include/linkedGeo.h
    src/h3lib/include/localij.h
    src/h3lib/include/baseCells.h
    src/h3lib/include/faceijk.h
    src/h3lib/include/vertexGraph.h
    src/h3lib/include/mathExtensions.h
    src/h3lib/include/constants.h
    src/h3lib/include/coordijk.h
    src/h3lib/include/algos.h
    src/h3lib/lib/algos.c
    src/h3lib/lib/coordijk.c
    src/h3lib/lib/bbox.c
    src/h3lib/lib/polygon.c
    src/h3lib/lib/h3Index.c
    src/h3lib/lib/vec2d.c
    src/h3lib/lib/vec3d.c
    src/h3lib/lib/vertex.c
    src/h3lib/lib/linkedGeo.c
    src/h3lib/lib/localij.c
    src/h3lib/lib/geoCoord.c
    src/h3lib/lib/h3UniEdge.c
    src/h3lib/lib/mathExtensions.c
    src/h3lib/lib/vertexGraph.c
    src/h3lib/lib/faceijk.c
    src/h3lib/lib/baseCells.c)
set(APP_SOURCE_FILES
    src/apps/applib/include/test.h
    src/apps/applib/include/kml.h
    src/apps/applib/include/benchmark.h
    src/apps/applib/include/utility.h
    src/apps/applib/include/args.h
    src/apps/applib/lib/test.c
    src/apps/applib/lib/kml.c
    src/apps/applib/lib/utility.c
    src/apps/applib/lib/args.c)
set(EXAMPLE_SOURCE_FILES
    examples/index.c
    examples/distance.c
    examples/neighbors.c
    examples/compact.c
    examples/edge.c)
set(OTHER_SOURCE_FILES
    src/apps/filters/h3ToGeo.c
    src/apps/filters/h3ToLocalIj.c
    src/apps/filters/localIjToH3.c
    src/apps/filters/h3ToComponents.c
    src/apps/filters/geoToH3.c
    src/apps/filters/h3ToGeoBoundary.c
    src/apps/filters/kRing.c
    src/apps/filters/hexRange.c
    src/apps/testapps/testVertexGraph.c
    src/apps/testapps/testCompact.c
    src/apps/testapps/testPolyfill.c
    src/apps/testapps/testPolyfillReported.c
    src/apps/testapps/testPentagonIndexes.c
    src/apps/testapps/testKRing.c
    src/apps/testapps/testH3ToGeoBoundary.c
    src/apps/testapps/testH3ToParent.c
    src/apps/testapps/testH3Index.c
    src/apps/testapps/mkRandGeoBoundary.c
    src/apps/testapps/testGeoToH3.c
    src/apps/testapps/testH3NeighborRotations.c
    src/apps/testapps/testMaxH3ToChildrenSize.c
    src/apps/testapps/testHexRanges.c
    src/apps/testapps/testH3ToGeo.c
    src/apps/testapps/testH3ToCenterChild.c
    src/apps/testapps/testH3ToChildren.c
    src/apps/testapps/testH3GetFaces.c
    src/apps/testapps/testGeoCoord.c
    src/apps/testapps/testHexRing.c
    src/apps/testapps/testH3SetToVertexGraph.c
    src/apps/testapps/testBBox.c
    src/apps/testapps/testVertex.c
    src/apps/testapps/testPolygon.c
    src/apps/testapps/testVec2d.c
    src/apps/testapps/testVec3d.c
    src/apps/testapps/testH3UniEdge.c
    src/apps/testapps/testH3UniEdgeExhaustive.c
    src/apps/testapps/testLinkedGeo.c
    src/apps/testapps/mkRandGeo.c
    src/apps/testapps/testH3Api.c
    src/apps/testapps/testH3SetToLinkedGeo.c
    src/apps/testapps/testH3ToLocalIj.c
    src/apps/testapps/testH3ToLocalIjExhaustive.c
    src/apps/testapps/testH3Distance.c
    src/apps/testapps/testH3DistanceExhaustive.c
    src/apps/testapps/testH3Line.c
    src/apps/testapps/testH3LineExhaustive.c
    src/apps/testapps/testH3CellArea.c
    src/apps/testapps/testH3CellAreaExhaustive.c
    src/apps/testapps/testCoordIj.c
    src/apps/testapps/testCoordIjk.c
    src/apps/testapps/testH3Memory.c
    src/apps/miscapps/h3ToGeoBoundaryHier.c
    src/apps/miscapps/h3ToGeoHier.c
    src/apps/miscapps/generateBaseCellNeighbors.c
    src/apps/miscapps/generatePentagonDirectionFaces.c
    src/apps/miscapps/generateNumHexagons.c
    src/apps/miscapps/generateFaceCenterPoint.c
    src/apps/miscapps/h3ToHier.c
    src/apps/benchmarks/benchmarkPolyfill.c
    src/apps/benchmarks/benchmarkPolygon.c
    src/apps/benchmarks/benchmarkH3SetToLinkedGeo.c
    src/apps/benchmarks/benchmarkKRing.c
    src/apps/benchmarks/benchmarkH3Line.c
    src/apps/benchmarks/benchmarkH3UniEdge.c
    src/apps/benchmarks/benchmarkH3Api.c)

set(ALL_SOURCE_FILES
    ${LIB_SOURCE_FILES} ${APP_SOURCE_FILES} ${OTHER_SOURCE_FILES})

set(UNCONFIGURED_API_HEADER src/h3lib/include/h3api.h.in)
set(CONFIGURED_API_HEADER src/h3lib/include/h3api.h)
configure_file(${UNCONFIGURED_API_HEADER} ${CONFIGURED_API_HEADER})

set(INSTALL_TARGETS)

function(add_h3_library name h3_alloc_prefix_override)
    add_library(${name} ${LIB_SOURCE_FILES} ${CONFIGURED_API_HEADER})

    target_compile_options(${name} PRIVATE ${H3_COMPILE_FLAGS})
    target_link_libraries(${name} PRIVATE ${H3_LINK_FLAGS})

    find_library(M_LIB m)
    if(M_LIB)
        target_link_libraries(${name} PUBLIC ${M_LIB})
    endif()

    if(BUILD_SHARED_LIBS)
        set_target_properties(${name} PROPERTIES SOVERSION ${H3_SOVERSION})
    endif()

    target_compile_definitions(${name} PUBLIC H3_PREFIX=${H3_PREFIX})
    set(has_alloc_prefix NO)
    if(h3_alloc_prefix_override)
        set(has_alloc_prefix YES)
        target_compile_definitions(${name} PUBLIC H3_ALLOC_PREFIX=${h3_alloc_prefix_override})
    elseif(H3_ALLOC_PREFIX)
        set(has_alloc_prefix YES)
        target_compile_definitions(${name} PUBLIC H3_ALLOC_PREFIX=${H3_ALLOC_PREFIX})
    endif()
    # Mac OSX defaults to not looking up undefined symbols dynamically,
    # so enable that explicitly.
    if(has_alloc_prefix AND APPLE)
        target_link_libraries(${name} PRIVATE "-undefined dynamic_lookup")
    endif()

    if(have_alloca)
        target_compile_definitions(${name} PUBLIC H3_HAVE_ALLOCA)
    endif()
    if(have_vla)
        target_compile_definitions(${name} PUBLIC H3_HAVE_VLA)
    endif()
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/h3lib/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src/h3lib/include>)
endfunction()

# Build the H3 library
add_h3_library(h3 "")

# Automatic code formatting
# Give preference to clang-format-9
find_program(CLANG_FORMAT_PATH NAMES clang-format-9 clang-format)
cmake_dependent_option(
    ENABLE_FORMAT "Enable running clang-format before compiling" ON
    "CLANG_FORMAT_PATH" OFF)
if(ENABLE_FORMAT)
    # Format
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_PATH}
        -style=file
        -i
        ${ALL_SOURCE_FILES}
        ${EXAMPLE_SOURCE_FILES}
        ${UNCONFIGURED_API_HEADER}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting sources"
        )
    # Always do formatting
    add_dependencies(h3 format)
elseif(NOT CLANG_FORMAT_PATH)
    message(WARNING "clang-format was not detected, "
                    "so automatic source code reformatting is disabled")
endif()

option(ENABLE_LINTING "Run clang-tidy on source files" ON)
find_program(CLANG_TIDY_PATH "clang-tidy")
cmake_dependent_option(
    ENABLE_LINTING "Enable running clang-tidy on sources during compilation" ON
    "CLANG_TIDY_PATH" OFF)
if(ENABLE_LINTING)
    set_target_properties(h3 PROPERTIES C_CLANG_TIDY "${CLANG_TIDY_PATH}")
elseif(NOT CLANG_TIDY_PATH)
    message(WARNING "clang-tidy was not detected, "
                  "so source code linting is disabled")
endif()

# Docs
find_package(Doxygen)
option(ENABLE_DOCS "Enable building documentation." ON)
if(DOXYGEN_FOUND AND ENABLE_DOCS)
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/dev-docs/_build")
    configure_file(dev-docs/Doxyfile.in
        dev-docs/Doxyfile
        ESCAPE_QUOTES
        )
    add_custom_target(docs
        ALL
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/dev-docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dev-docs
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
else()
    add_custom_target(docs
        echo "Doxygen was not installed when CMake was run or ENABLE_DOCS was OFF. Check that Doxygen is installed and rerun `cmake .`" VERBATIM
        )
endif()

# Metadata for bindings
if (WIN32)
    add_custom_target(binding-functions
        COMMAND PowerShell -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/scripts/binding_functions.ps1
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
else()
    add_custom_target(binding-functions
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/binding_functions.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
endif()

# Release publishing
add_custom_target(update-version
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/update_version.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

# Website publishing
add_custom_target(publish-website
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/publish_website.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

# Link all executables against H3
macro(add_h3_executable name)
    # invoke built-in add_executable
    add_executable(${ARGV})
    if(TARGET ${name})
        target_link_libraries(${name} PUBLIC h3)
        target_include_directories(${name} PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/apps/applib/include>)
        target_compile_options(${name} PRIVATE ${H3_COMPILE_FLAGS})
        target_link_libraries(${name} PRIVATE ${H3_LINK_FLAGS})
    endif()
endmacro()

if(BUILD_FILTERS)
    macro(add_h3_filter name)
        add_h3_executable(${ARGV})
        list(APPEND INSTALL_TARGETS ${name})
    endmacro()

    add_h3_filter(geoToH3 src/apps/filters/geoToH3.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToComponents src/apps/filters/h3ToComponents.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToGeo src/apps/filters/h3ToGeo.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToLocalIj src/apps/filters/h3ToLocalIj.c ${APP_SOURCE_FILES})
    add_h3_filter(localIjToH3 src/apps/filters/localIjToH3.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToGeoBoundary src/apps/filters/h3ToGeoBoundary.c ${APP_SOURCE_FILES})
    add_h3_filter(hexRange src/apps/filters/hexRange.c ${APP_SOURCE_FILES})
    add_h3_filter(kRing src/apps/filters/kRing.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToGeoBoundaryHier src/apps/miscapps/h3ToGeoBoundaryHier.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToGeoHier src/apps/miscapps/h3ToGeoHier.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToHier src/apps/miscapps/h3ToHier.c ${APP_SOURCE_FILES})

    # Generate KML files for visualizing the H3 grid
    add_custom_target(create-kml-dir
        COMMAND ${CMAKE_COMMAND} -E make_directory KML)
    add_custom_target(kml)

    # Only the first 3 resolution grids are generated. The others can be generated,
    # but the file sizes would be very, very large.
    foreach(resolution RANGE 3)
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "KML/res${resolution}cells.kml")
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "KML/res${resolution}centers.kml")
        add_custom_target(kml_cells_${resolution}
            COMMAND ${SHELL} "$<TARGET_FILE:h3ToHier> -r ${resolution} | $<TARGET_FILE:h3ToGeoBoundary> --kml --kml-name res${resolution}cells.kml --kml-description \"Res ${resolution} Cells\" > KML/res${resolution}cells.kml"
            VERBATIM
            DEPENDS create-kml-dir)
        add_custom_target(kml_centers_${resolution}
            COMMAND ${SHELL} "$<TARGET_FILE:h3ToHier> -r ${resolution} | $<TARGET_FILE:h3ToGeo> --kml --kml-name res${resolution}centers.kml --kml-description \"Res ${resolution} Centers\" > KML/res${resolution}centers.kml"
            VERBATIM
            DEPENDS create-kml-dir)
        add_dependencies(kml
            kml_cells_${resolution}
            kml_centers_${resolution})
    endforeach()
endif()

if(BUILD_GENERATORS)
    # Code generation
    add_h3_executable(generateBaseCellNeighbors src/apps/miscapps/generateBaseCellNeighbors.c ${APP_SOURCE_FILES})
    add_h3_executable(generatePentagonDirectionFaces src/apps/miscapps/generatePentagonDirectionFaces.c ${APP_SOURCE_FILES})
    add_h3_executable(generateNumHexagons src/apps/miscapps/generateNumHexagons.c ${APP_SOURCE_FILES})
    add_h3_executable(generateFaceCenterPoint src/apps/miscapps/generateFaceCenterPoint.c ${APP_SOURCE_FILES})

    # Miscellaneous testing applications - generating random data
    add_h3_executable(mkRandGeo src/apps/testapps/mkRandGeo.c ${APP_SOURCE_FILES})
    add_h3_executable(mkRandGeoBoundary src/apps/testapps/mkRandGeoBoundary.c ${APP_SOURCE_FILES})
endif()

if(BUILD_TESTING)
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

    add_h3_library(h3WithTestAllocators test_prefix_)

    macro(add_h3_memory_test name srcfile)
        # Like other test code, but these need to be linked against
        # a different copy of the H3 library which has known intercepted
        # allocator functions.
        add_executable(${ARGV} ${APP_SOURCE_FILES})

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
            add_h3_executable(${name} ${srcfile} ${APP_SOURCE_FILES})
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
        add_h3_test_with_file(testH3ToGeo src/apps/testapps/testH3ToGeo.c ${file})
    endforeach()

    file(GLOB all_ic_files tests/inputfiles/res*ic.txt)
    foreach(file ${all_ic_files})
        add_h3_test_with_file(testH3ToGeo src/apps/testapps/testH3ToGeo.c ${file})
    endforeach()

    file(GLOB all_centers tests/inputfiles/rand*centers.txt)
    foreach(file ${all_centers})
        add_h3_test_with_file(testGeoToH3 src/apps/testapps/testGeoToH3.c ${file})
    endforeach()

    file(GLOB all_cells tests/inputfiles/*cells.txt)
    foreach(file ${all_cells})
        add_h3_test_with_file(testH3ToGeoBoundary src/apps/testapps/testH3ToGeoBoundary.c ${file})
    endforeach()

    add_h3_test(testCompact src/apps/testapps/testCompact.c)
    add_h3_test(testKRing src/apps/testapps/testKRing.c)
    add_h3_test(testHexRing src/apps/testapps/testHexRing.c)
    add_h3_test(testHexRanges src/apps/testapps/testHexRanges.c)
    add_h3_test(testH3ToParent src/apps/testapps/testH3ToParent.c)
    add_h3_test(testH3ToCenterChild src/apps/testapps/testH3ToCenterChild.c)
    add_h3_test(testH3ToChildren src/apps/testapps/testH3ToChildren.c)
    add_h3_test(testH3GetFaces src/apps/testapps/testH3GetFaces.c)
    add_h3_test(testMaxH3ToChildrenSize src/apps/testapps/testMaxH3ToChildrenSize.c)
    add_h3_test(testH3Index src/apps/testapps/testH3Index.c)
    add_h3_test(testH3Api src/apps/testapps/testH3Api.c)
    add_h3_test(testH3SetToLinkedGeo src/apps/testapps/testH3SetToLinkedGeo.c)
    add_h3_test(testH3SetToVertexGraph src/apps/testapps/testH3SetToVertexGraph.c)
    add_h3_test(testLinkedGeo src/apps/testapps/testLinkedGeo.c)
    add_h3_test(testPolyfill src/apps/testapps/testPolyfill.c)
    add_h3_test(testPolyfillReported src/apps/testapps/testPolyfillReported.c)
    add_h3_test(testVertexGraph src/apps/testapps/testVertexGraph.c)
    add_h3_test(testH3UniEdge src/apps/testapps/testH3UniEdge.c)
    add_h3_test(testGeoCoord src/apps/testapps/testGeoCoord.c)
    add_h3_test(testBBox src/apps/testapps/testBBox.c)
    add_h3_test(testVertex src/apps/testapps/testVertex.c)
    add_h3_test(testPolygon src/apps/testapps/testPolygon.c)
    add_h3_test(testVec2d src/apps/testapps/testVec2d.c)
    add_h3_test(testVec3d src/apps/testapps/testVec3d.c)
    add_h3_test(testH3ToLocalIj src/apps/testapps/testH3ToLocalIj.c)
    add_h3_test(testH3Distance src/apps/testapps/testH3Distance.c)
    add_h3_test(testH3Line src/apps/testapps/testH3Line.c)
    add_h3_test(testH3CellArea src/apps/testapps/testH3CellArea.c)
    add_h3_test(testCoordIj src/apps/testapps/testCoordIj.c)
    add_h3_test(testCoordIjk src/apps/testapps/testCoordIjk.c)
    add_h3_test(testBaseCells src/apps/testapps/testBaseCells.c)
    add_h3_test(testPentagonIndexes src/apps/testapps/testPentagonIndexes.c)

    add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 0)
    add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 1)
    add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 2)

    # The "Exhaustive" part of the test name is used by the test-fast to exclude these files.
    # test-fast exists so that Travis CI can run Valgrind on tests without taking a very long time.
    add_h3_test(testH3UniEdgeExhaustive src/apps/testapps/testH3UniEdgeExhaustive.c)
    add_h3_test(testH3ToLocalIjExhaustive src/apps/testapps/testH3ToLocalIjExhaustive.c)
    add_h3_test(testH3LineExhaustive src/apps/testapps/testH3LineExhaustive.c)
    add_h3_test(testH3DistanceExhaustive src/apps/testapps/testH3DistanceExhaustive.c)
    add_h3_test(testH3CellAreaExhaustive src/apps/testapps/testH3CellAreaExhaustive.c)

    add_h3_memory_test(testH3Memory src/apps/testapps/testH3Memory.c)

    add_custom_target(test-fast COMMAND ctest -E Exhaustive)
endif()

if(BUILD_BENCHMARKS)
    # Benchmarks
    add_custom_target(benchmarks)

    macro(add_h3_benchmark name srcfile)
        add_h3_executable(${name} ${srcfile} ${APP_SOURCE_FILES})
        add_custom_target(bench_${name} COMMAND ${TEST_WRAPPER} $<TARGET_FILE:${name}>)
        add_dependencies(benchmarks bench_${name})
    endmacro()

    add_h3_benchmark(benchmarkH3Api src/apps/benchmarks/benchmarkH3Api.c)
    add_h3_benchmark(benchmarkKRing src/apps/benchmarks/benchmarkKRing.c)
    add_h3_benchmark(benchmarkH3Line src/apps/benchmarks/benchmarkH3Line.c)
    add_h3_benchmark(benchmarkH3UniEdge src/apps/benchmarks/benchmarkH3UniEdge.c)
    add_h3_benchmark(benchmarkH3SetToLinkedGeo src/apps/benchmarks/benchmarkH3SetToLinkedGeo.c)
    add_h3_benchmark(benchmarkPolyfill src/apps/benchmarks/benchmarkPolyfill.c)
    add_h3_benchmark(benchmarkPolygon src/apps/benchmarks/benchmarkPolygon.c)
endif()

# Installation (https://github.com/forexample/package-example)

# Layout. This works for all platforms:
#   * <prefix>/lib/cmake/<PROJECT-NAME>
#   * <prefix>/lib/
#   * <prefix>/include/
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# TODO: Unclear why this is needed to get the libh3 Debian package to build correctly
# with shared libraries.
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "libh3")

# Include module with fuction 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Use:
#   * PROJECT_VERSION
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * TARGETS_EXPORT_NAME
#   * PROJECT_NAME
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

# Targets:
#   * <prefix>/lib/libh3.so
#   * header location after install: <prefix>/include/h3/h3api.h
#   * headers can be included by C++ code `#include <h3/h3api.h>`
# Installing the library and filters system-wide.
install(
    TARGETS ${INSTALL_TARGETS}
    EXPORT "${TARGETS_EXPORT_NAME}"
    RUNTIME DESTINATION "bin"
    COMPONENT h3
)

install(
    TARGETS h3
    EXPORT "${TARGETS_EXPORT_NAME}"
    COMPONENT libh3
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${include_install_dir}"
)

# Headers:
#   * src/h3lib/include/h3api.h -> <prefix>/include/h3/h3api.h
# Only the h3api.h header is needed by applications using H3.
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/src/h3lib/include/h3api.h"
    DESTINATION "${include_install_dir}/h3"
    COMPONENT libh3-dev
)

# Config
#   * <prefix>/lib/cmake/h3/h3Config.cmake
#   * <prefix>/lib/cmake/h3/h3ConfigVersion.cmake
install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
    COMPONENT libh3-dev
)

# Config
#   * <prefix>/lib/cmake/h3/h3Targets.cmake
install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
    COMPONENT libh3-dev
)

# Debian package build
set(CPACK_DEB_COMPONENT_INSTALL 1)
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
# set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TEST PACKAGE") # Required
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.h3geo.org")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_DEBIAN_LIBH3_PACKAGE_DEPENDS "libc6 (>= 2.27)")
set(CPACK_DEBIAN_LIBH3-DEV_PACKAGE_DEPENDS "libh3 (= ${H3_VERSION})")
set(CPACK_DEBIAN_H3_PACKAGE_DEPENDS "libc6 (>= 2.27), libh3 (= ${H3_VERSION})")
set(CPACK_DEBIAN_LIBH3_DESCRIPTION "Library files for the H3 hexagonal discrete global grid system.")
set(CPACK_DEBIAN_LIBH3-DEV_DESCRIPTION "Development files and headers for the H3 hexagonal discrete global grid system.")
set(CPACK_DEBIAN_H3_DESCRIPTION "UNIX style filter (command line) tools for the H3 hexagonal discrete global grid system.")
set(CPACK_DEBIAN_LIBH3_PACKAGE_NAME "libh3")
set(CPACK_DEBIAN_LIBH3-DEV_PACKAGE_NAME "libh3-dev")
set(CPACK_DEBIAN_H3_PACKAGE_NAME "h3")
set(CPACK_DEBIAN_LIBH3_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_LIBH3-DEV_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_H3_PACKAGE_SECTION "science")

include(CPack)
