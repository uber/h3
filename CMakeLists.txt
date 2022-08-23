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

cmake_minimum_required(VERSION 3.20) # Due to dependency on CMP0115

set(H3_PREFIX "" CACHE STRING "Prefix for exported symbols")
set(H3_ALLOC_PREFIX "" CACHE STRING "Prefix for allocation functions")

# Do not try to append extensions to source files
cmake_policy(SET CMP0115 NEW)
# Needed due to CMP0042
set(CMAKE_MACOSX_RPATH 1)
# YCM needs compilation database
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain.cmake"
    CACHE FILEPATH
    "Toolchain to use for building this project")

# Some misc apps do not work with shared libraries on Windows
# because they require access to internal H3 functions. Build these
# using either static libraries or an option to expose all
# function symbols.
if(NOT (WIN32 AND BUILD_SHARED_LIBS))
    set(ENABLE_REQUIRES_ALL_SYMBOLS ON)
else()
    set(ENABLE_REQUIRES_ALL_SYMBOLS OFF)
endif()

option(ENABLE_COVERAGE "Enable compiling tests with coverage." ON)
option(BUILD_BENCHMARKS "Build benchmarking applications." ON)
option(BUILD_FUZZERS "Build fuzzer applications (for use with afl)." ON)
option(BUILD_FILTERS "Build filter applications." ON)
option(BUILD_GENERATORS "Build code generation applications." ON)
# If ON, libfuzzer settings are used to build the fuzzer harnesses. If OFF, a frontend
# for afl++ is provided instead.
option(ENABLE_LIBFUZZER "Build fuzzers with libFuzzer support." OFF)

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
# Remove any trailing qualifier
string(REGEX REPLACE "-.*$" "" H3_VERSION "${H3_VERSION}")
string(REPLACE "." ";" H3_VERSION_LIST "${H3_VERSION}")
list(GET H3_VERSION_LIST 0 H3_VERSION_MAJOR)
list(GET H3_VERSION_LIST 1 H3_VERSION_MINOR)
list(GET H3_VERSION_LIST 2 H3_VERSION_PATCH)
set(H3_SOVERSION 1)

project(h3 LANGUAGES C VERSION ${H3_VERSION})

# Detect if someone else is including the package
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(H3_IS_ROOT_PROJECT ON)
endif()

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
    if(ENABLE_LIBFUZZER)
        list(APPEND H3_COMPILE_FLAGS -fsanitize=fuzzer,address,undefined)
        list(APPEND H3_LINK_FLAGS -fsanitize=fuzzer,address,undefined)
    endif()

    option(WARNINGS_AS_ERRORS "Warnings are treated as errors" OFF)
    if(WARNINGS_AS_ERRORS)
        list(APPEND H3_COMPILE_FLAGS -Werror)
    endif()
endif()

include(CMakeDependentOption)
include(CheckIncludeFile)
if(H3_IS_ROOT_PROJECT)
    include(CTest)
endif()

set(LIB_SOURCE_FILES
    src/h3lib/include/bbox.h
    src/h3lib/include/polygon.h
    src/h3lib/include/polygonAlgos.h
    src/h3lib/include/h3Index.h
    src/h3lib/include/directedEdge.h
    src/h3lib/include/latLng.h
    src/h3lib/include/vec2d.h
    src/h3lib/include/vec3d.h
    src/h3lib/include/linkedGeo.h
    src/h3lib/include/localij.h
    src/h3lib/include/baseCells.h
    src/h3lib/include/faceijk.h
    src/h3lib/include/vertexGraph.h
    src/h3lib/include/mathExtensions.h
    src/h3lib/include/iterators.h
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
    src/h3lib/lib/latLng.c
    src/h3lib/lib/directedEdge.c
    src/h3lib/lib/mathExtensions.c
    src/h3lib/lib/iterators.c
    src/h3lib/lib/vertexGraph.c
    src/h3lib/lib/faceijk.c
    src/h3lib/lib/baseCells.c)
set(APP_SOURCE_FILES
    src/apps/applib/include/kml.h
    src/apps/applib/include/benchmark.h
    src/apps/applib/include/utility.h
    src/apps/applib/include/args.h
    src/apps/applib/include/aflHarness.h
    src/apps/applib/lib/kml.c
    src/apps/applib/lib/utility.c
    src/apps/applib/lib/args.c)
set(TEST_APP_SOURCE_FILES
    src/apps/applib/include/test.h
    src/apps/applib/lib/test.c)
set(EXAMPLE_SOURCE_FILES
    examples/index.c
    examples/distance.c
    examples/neighbors.c
    examples/compactCells.c
    examples/edge.c)
set(OTHER_SOURCE_FILES
    src/apps/filters/h3.c
    src/apps/filters/cellToLatLng.c
    src/apps/filters/cellToLocalIj.c
    src/apps/filters/localIjToCell.c
    src/apps/filters/h3ToComponents.c
    src/apps/filters/latLngToCell.c
    src/apps/filters/cellToBoundary.c
    src/apps/filters/gridDisk.c
    src/apps/filters/gridDiskUnsafe.c
    src/apps/testapps/testVertexGraph.c
    src/apps/testapps/testCompactCells.c
    src/apps/testapps/testPolygonToCells.c
    src/apps/testapps/testPolygonToCellsReported.c
    src/apps/testapps/testPentagonIndexes.c
    src/apps/testapps/testGridDisk.c
    src/apps/testapps/testCellToBoundary.c
    src/apps/testapps/testCellToParent.c
    src/apps/testapps/testH3Index.c
    src/apps/testapps/mkRandGeoBoundary.c
    src/apps/testapps/testLatLngToCell.c
    src/apps/testapps/testH3NeighborRotations.c
    src/apps/testapps/testCellToChildrenSize.c
    src/apps/testapps/testGridDisksUnsafe.c
    src/apps/testapps/testCellToLatLng.c
    src/apps/testapps/testCellToCenterChild.c
    src/apps/testapps/testCellToChildren.c
    src/apps/testapps/testGetIcosahedronFaces.c
    src/apps/testapps/testLatLng.c
    src/apps/testapps/testGridRingUnsafe.c
    src/apps/testapps/testH3SetToVertexGraph.c
    src/apps/testapps/testBBox.c
    src/apps/testapps/testVertex.c
    src/apps/testapps/testVertexExhaustive.c
    src/apps/testapps/testPolygon.c
    src/apps/testapps/testVec2d.c
    src/apps/testapps/testVec3d.c
    src/apps/testapps/testDirectedEdge.c
    src/apps/testapps/testDirectedEdgeExhaustive.c
    src/apps/testapps/testLinkedGeo.c
    src/apps/testapps/mkRandGeo.c
    src/apps/testapps/testH3Api.c
    src/apps/testapps/testCellsToLinkedMultiPolygon.c
    src/apps/testapps/testCellToLocalIj.c
    src/apps/testapps/testCellToLocalIjExhaustive.c
    src/apps/testapps/testGridDistance.c
    src/apps/testapps/testGridDistanceExhaustive.c
    src/apps/testapps/testGridPathCells.c
    src/apps/testapps/testGridPathCellsExhaustive.c
    src/apps/testapps/testH3CellArea.c
    src/apps/testapps/testH3CellAreaExhaustive.c
    src/apps/testapps/testCoordIj.c
    src/apps/testapps/testCoordIjk.c
    src/apps/testapps/testH3Memory.c
    src/apps/testapps/testH3Iterators.c
    src/apps/miscapps/cellToBoundaryHier.c
    src/apps/miscapps/cellToLatLngHier.c
    src/apps/miscapps/generateBaseCellNeighbors.c
    src/apps/miscapps/generatePentagonDirectionFaces.c
    src/apps/miscapps/generateFaceCenterPoint.c
    src/apps/miscapps/h3ToHier.c
    src/apps/fuzzers/fuzzerLatLngToCell.c
    src/apps/fuzzers/fuzzerCellToLatLng.c
    src/apps/fuzzers/fuzzerGridDisk.c
    src/apps/fuzzers/fuzzerCellsToLinkedMultiPolygon.c
    src/apps/fuzzers/fuzzerDistances.c
    src/apps/fuzzers/fuzzerCellArea.c
    src/apps/fuzzers/fuzzerEdgeLength.c
    src/apps/fuzzers/fuzzerCellProperties.c
    src/apps/fuzzers/fuzzerIndexIO.c
    src/apps/fuzzers/fuzzerResolutions.c
    src/apps/fuzzers/fuzzerHierarchy.c
    src/apps/fuzzers/fuzzerVertexes.c
    src/apps/fuzzers/fuzzerCompact.c
    src/apps/fuzzers/fuzzerDirectedEdge.c
    src/apps/fuzzers/fuzzerLocalIj.c
    src/apps/fuzzers/fuzzerPolygonToCells.c
    src/apps/fuzzers/fuzzerPolygonToCellsNoHoles.c
    src/apps/benchmarks/benchmarkPolygonToCells.c
    src/apps/benchmarks/benchmarkPolygon.c
    src/apps/benchmarks/benchmarkCellsToLinkedMultiPolygon.c
    src/apps/benchmarks/benchmarkGridDiskCells.c
    src/apps/benchmarks/benchmarkGridPathCells.c
    src/apps/benchmarks/benchmarkDirectedEdge.c
    src/apps/benchmarks/benchmarkVertex.c
    src/apps/benchmarks/benchmarkIsValidCell.c
    src/apps/benchmarks/benchmarkH3Api.c)

set(ALL_SOURCE_FILES
    ${LIB_SOURCE_FILES} ${APP_SOURCE_FILES} ${TEST_APP_SOURCE_FILES} ${OTHER_SOURCE_FILES})

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
        target_compile_definitions(${name} PRIVATE BUILD_SHARED_LIBS=1)
    endif()

    target_compile_definitions(${name} PUBLIC H3_PREFIX=${H3_PREFIX})
    target_compile_definitions(${name} PRIVATE BUILDING_H3=1)
    set(has_alloc_prefix NO)
    if(h3_alloc_prefix_override)
        set(has_alloc_prefix YES)
        target_compile_definitions(${name} PUBLIC H3_ALLOC_PREFIX=${h3_alloc_prefix_override})
    elseif(H3_ALLOC_PREFIX)
        set(has_alloc_prefix YES)
        target_compile_definitions(${name} PUBLIC H3_ALLOC_PREFIX=${H3_ALLOC_PREFIX})
    endif()
    # Mac OSX defaults to not looking up undefined symbols dynamically,
    # so enable that explicitly. Windows needs something similar.
    if(has_alloc_prefix AND APPLE)
        target_link_libraries(${name} PRIVATE "-undefined dynamic_lookup")
    elseif(has_alloc_prefix AND MSVC)
        set(TARGET ${name} PROPERTY APPEND LINK_FLAGS "/FORCE:UNRESOLVED")
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

    add_h3_filter(h3_bin src/apps/filters/h3.c ${APP_SOURCE_FILES})
    set_target_properties(h3_bin PROPERTIES OUTPUT_NAME h3) # Special logic for the `h3` executable
    add_h3_filter(latLngToCell src/apps/filters/latLngToCell.c ${APP_SOURCE_FILES})
    add_h3_filter(h3ToComponents src/apps/filters/h3ToComponents.c ${APP_SOURCE_FILES})
    add_h3_filter(cellToLatLng src/apps/filters/cellToLatLng.c ${APP_SOURCE_FILES})
    add_h3_filter(cellToLocalIj src/apps/filters/cellToLocalIj.c ${APP_SOURCE_FILES})
    add_h3_filter(localIjToCell src/apps/filters/localIjToCell.c ${APP_SOURCE_FILES})
    add_h3_filter(cellToBoundary src/apps/filters/cellToBoundary.c ${APP_SOURCE_FILES})
    add_h3_filter(gridDiskUnsafe src/apps/filters/gridDiskUnsafe.c ${APP_SOURCE_FILES})
    add_h3_filter(gridDisk src/apps/filters/gridDisk.c ${APP_SOURCE_FILES})
    add_h3_filter(cellToBoundaryHier src/apps/miscapps/cellToBoundaryHier.c ${APP_SOURCE_FILES})
    add_h3_filter(cellToLatLngHier src/apps/miscapps/cellToLatLngHier.c ${APP_SOURCE_FILES})
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
            COMMAND ${SHELL} "$<TARGET_FILE:h3ToHier> -r ${resolution} | $<TARGET_FILE:cellToBoundary> --kml --kml-name res${resolution}cells.kml --kml-description \"Res ${resolution} Cells\" > KML/res${resolution}cells.kml"
            VERBATIM
            DEPENDS create-kml-dir)
        add_custom_target(kml_centers_${resolution}
            COMMAND ${SHELL} "$<TARGET_FILE:h3ToHier> -r ${resolution} | $<TARGET_FILE:cellToLatLng> --kml --kml-name res${resolution}centers.kml --kml-description \"Res ${resolution} Centers\" > KML/res${resolution}centers.kml"
            VERBATIM
            DEPENDS create-kml-dir)
        add_dependencies(kml
            kml_cells_${resolution}
            kml_centers_${resolution})
    endforeach()
endif()

if(BUILD_GENERATORS AND ENABLE_REQUIRES_ALL_SYMBOLS)
    # Code generation
    add_h3_executable(generateBaseCellNeighbors src/apps/miscapps/generateBaseCellNeighbors.c ${APP_SOURCE_FILES})
    add_h3_executable(generateFaceCenterPoint src/apps/miscapps/generateFaceCenterPoint.c ${APP_SOURCE_FILES})
    add_h3_executable(generatePentagonDirectionFaces src/apps/miscapps/generatePentagonDirectionFaces.c ${APP_SOURCE_FILES})

    # Miscellaneous testing applications - generating random data
    add_h3_executable(mkRandGeo src/apps/testapps/mkRandGeo.c ${APP_SOURCE_FILES})
    add_h3_executable(mkRandGeoBoundary src/apps/testapps/mkRandGeoBoundary.c ${APP_SOURCE_FILES})
endif()

if(H3_IS_ROOT_PROJECT AND BUILD_TESTING)
    include(CMakeTests.cmake)
endif()

if(BUILD_FUZZERS)
    add_custom_target(fuzzers)

    macro(add_h3_fuzzer name srcfile)
        add_h3_executable(${name} ${srcfile} ${APP_SOURCE_FILES})
        if(ENABLE_LIBFUZZER)
            target_compile_definitions(${name} PRIVATE H3_USE_LIBFUZZER)
        endif()
        add_dependencies(fuzzers ${name})
    endmacro()

    add_h3_fuzzer(fuzzerLatLngToCell src/apps/fuzzers/fuzzerLatLngToCell.c)
    add_h3_fuzzer(fuzzerCellToLatLng src/apps/fuzzers/fuzzerCellToLatLng.c)
    add_h3_fuzzer(fuzzerGridDisk src/apps/fuzzers/fuzzerGridDisk.c)
    add_h3_fuzzer(fuzzerCellsToLinkedMultiPolygon src/apps/fuzzers/fuzzerCellsToLinkedMultiPolygon.c)
    add_h3_fuzzer(fuzzerDistances src/apps/fuzzers/fuzzerDistances.c)
    add_h3_fuzzer(fuzzerCellArea src/apps/fuzzers/fuzzerCellArea.c)
    add_h3_fuzzer(fuzzerEdgeLength src/apps/fuzzers/fuzzerEdgeLength.c)
    add_h3_fuzzer(fuzzerCellProperties src/apps/fuzzers/fuzzerCellProperties.c)
    add_h3_fuzzer(fuzzerIndexIO src/apps/fuzzers/fuzzerIndexIO.c)
    add_h3_fuzzer(fuzzerResolutions src/apps/fuzzers/fuzzerResolutions.c)
    add_h3_fuzzer(fuzzerHierarchy src/apps/fuzzers/fuzzerHierarchy.c)
    add_h3_fuzzer(fuzzerVertexes src/apps/fuzzers/fuzzerVertexes.c)
    add_h3_fuzzer(fuzzerCompact src/apps/fuzzers/fuzzerCompact.c)
    add_h3_fuzzer(fuzzerDirectedEdge src/apps/fuzzers/fuzzerDirectedEdge.c)
    add_h3_fuzzer(fuzzerLocalIj src/apps/fuzzers/fuzzerLocalIj.c)
    add_h3_fuzzer(fuzzerPolygonToCells src/apps/fuzzers/fuzzerPolygonToCells.c)
    add_h3_fuzzer(fuzzerPolygonToCellsNoHoles src/apps/fuzzers/fuzzerPolygonToCellsNoHoles.c)
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
    add_h3_benchmark(benchmarkGridDiskCells src/apps/benchmarks/benchmarkGridDiskCells.c)
    add_h3_benchmark(benchmarkGridPathCells src/apps/benchmarks/benchmarkGridPathCells.c)
    add_h3_benchmark(benchmarkDirectedEdge src/apps/benchmarks/benchmarkDirectedEdge.c)
    add_h3_benchmark(benchmarkVertex src/apps/benchmarks/benchmarkVertex.c)
    add_h3_benchmark(benchmarkIsValidCell src/apps/benchmarks/benchmarkIsValidCell.c)
    add_h3_benchmark(benchmarkCellsToLinkedMultiPolygon src/apps/benchmarks/benchmarkCellsToLinkedMultiPolygon.c)
    add_h3_benchmark(benchmarkPolygonToCells src/apps/benchmarks/benchmarkPolygonToCells.c)
    if(ENABLE_REQUIRES_ALL_SYMBOLS)
        add_h3_benchmark(benchmarkPolygon src/apps/benchmarks/benchmarkPolygon.c)
    endif()
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
    DESTINATION "bin"
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
