# Copyright 2017 Uber Technologies, Inc.
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

cmake_minimum_required(VERSION 2.8.7)
project(h3)

set(H3_PREFIX "" CACHE STRING "Prefix for exported symbols")
add_definitions(-DH3_PREFIX=${H3_PREFIX})

set(H3_COMPILER "Default" CACHE STRING "C Compiler to use for H3")
set_property(
    CACHE H3_COMPILER
    PROPERTY STRINGS
    "Default" "clang" "gcc" "cc"
)

if(H3_COMPILER MATCHES clang)
    set(CMAKE_C_COMPILER clang)
elseif(H3_COMPILER MATCHES gcc)
    set(CMAKE_C_COMPILER gcc)
elseif(H3_COMPILER MATCHES cc)
    set(CMAKE_C_COMPILER cc)
endif()

# Needed due to CMP0042
set(CMAKE_MACOSX_RPATH 1)
# H3 doesn't export selected symbols right now
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(WIN32)
    set(SHELL PowerShell -Command)
else()
    set(SHELL sh -c)

    # Compiler options are set only on non-Windows, since these options
    # are not correct for MSVC.
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -gdwarf-2 -g3 -O0 -Wall -fno-inline -fno-eliminate-unused-debug-types --coverage -std=c1x")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -fPIC -std=c1x")
    endif()
endif()

file(READ VERSION H3_VERSION LIMIT_COUNT 1)
# Clean any newlines
string(REPLACE "\n" "" H3_VERSION "${H3_VERSION}")

find_library(M_LIB m)

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)

include_directories(src/h3lib/include)

file(GLOB_RECURSE ALL_SOURCE_FILES src/*.c src/*.h)
file(GLOB_RECURSE EXAMPLE_SOURCE_FILES examples/*.c)

# Build the H3 library
file(GLOB_RECURSE SOURCE_FILES src/h3lib/*.c src/h3lib/*.h)
option(BUILD_STATIC "Build a static version of the h3 library." OFF)
if (BUILD_STATIC)
    add_library(h3.1 STATIC ${SOURCE_FILES})
else()
    add_library(h3.1 SHARED ${SOURCE_FILES})
endif()

# Automatic code formatting
option(ENABLE_FORMAT "Enable running clang-format before compiling." ON)
find_program(CLANG_FORMAT_PATH clang-format)
if (CLANG_FORMAT_PATH AND ENABLE_FORMAT)
    # Format
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_PATH}
        -style='{BasedOnStyle: Google, IndentWidth: 4}'
        -i
        ${ALL_SOURCE_FILES}
        ${EXAMPLE_SOURCE_FILES}
        COMMENT "Formatting sources"
        )
    # Always do formatting
    add_dependencies(h3.1 format)
elseif(ENABLE_FORMAT)
    message(WARNING "clang-format was not detected, so automatic source code reformatting is disabled.")
endif()

# Docs
find_package(Doxygen)
option(ENABLE_DOCS "Enable building documentation." ON)
if(DOXYGEN_FOUND AND ENABLE_DOCS)
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_BINARY_DIR}/docs/_build")
    configure_file(docs/Doxyfile.in
        docs/Doxyfile
        ESCAPE_QUOTES
        )
    add_custom_target(docs
        ALL
        ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/docs
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
        COMMAND PowerShell -ExecutionPolicy Bypass -File ${PROJECT_SOURCE_DIR}/scripts/binding_functions.ps1
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        )
else()
    add_custom_target(binding-functions
        COMMAND ${PROJECT_SOURCE_DIR}/scripts/binding_functions.sh
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        )
endif()

# Link all executables against H3
macro(add_h3_executable name)
    # invoke built-in add_executable
    add_executable(${ARGV})
    if(TARGET ${name})
        target_link_libraries(${name} h3.1)
        if (M_LIB)
            target_link_libraries(${name} ${M_LIB})
        endif()
        target_include_directories(${name} PUBLIC src/apps/applib/include)
    endif()
endmacro()

# Additional source files common to the applications
file(GLOB_RECURSE APP_SOURCE_FILES src/apps/applib/lib/*.c src/apps/applib/include/*.h)

add_h3_executable(geoToH3 src/apps/filters/geoToH3.c ${APP_SOURCE_FILES})
add_h3_executable(h3ToComponents src/apps/filters/h3ToComponents.c ${APP_SOURCE_FILES})
add_h3_executable(h3ToGeo src/apps/filters/h3ToGeo.c ${APP_SOURCE_FILES})
add_h3_executable(h3ToGeoBoundary src/apps/filters/h3ToGeoBoundary.c ${APP_SOURCE_FILES})
add_h3_executable(hexRange src/apps/filters/hexRange.c ${APP_SOURCE_FILES})
add_h3_executable(kRing src/apps/filters/kRing.c ${APP_SOURCE_FILES})
add_h3_executable(generateBaseCellNeighbors src/apps/miscapps/generateBaseCellNeighbors.c ${APP_SOURCE_FILES})
add_h3_executable(h3ToGeoBoundaryHier src/apps/miscapps/h3ToGeoBoundaryHier.c ${APP_SOURCE_FILES})
add_h3_executable(h3ToGeoHier src/apps/miscapps/h3ToGeoHier.c ${APP_SOURCE_FILES})
add_h3_executable(h3ToHier src/apps/miscapps/h3ToHier.c ${APP_SOURCE_FILES})

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
        COMMAND ${SHELL} "$<TARGET_FILE:h3ToHier> ${resolution} | $<TARGET_FILE:h3ToGeoBoundary> 1 res${resolution}cells.kml \"Res ${resolution} Cells\" > KML/res${resolution}cells.kml"
        VERBATIM
        DEPENDS create-kml-dir)
    add_custom_target(kml_centers_${resolution}
        COMMAND ${SHELL} "$<TARGET_FILE:h3ToHier> ${resolution} | $<TARGET_FILE:h3ToGeo> 1 res${resolution}centers.kml \"Res ${resolution} Centers\" > KML/res${resolution}centers.kml"
        VERBATIM
        DEPENDS create-kml-dir)
    add_dependencies(kml
        kml_cells_${resolution}
        kml_centers_${resolution})
endforeach()

option(ENABLE_TESTS "Enable building tests." ON)
if(ENABLE_TESTS)
    option(PRINT_TEST_FILES "Print which test files correspond to which tests." OFF)

    enable_testing()

    # Macros and support code needed to build and add the tests
    set(test_number 0)

    option(ENABLE_COVERAGE "Enable compiling tests with coverage." ON)
    if(ENABLE_COVERAGE)
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage.info")
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage.cleaned.info")
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "coverage")
        add_custom_target(coverage
            # Compile generated coverage counts into a single coverage.info
            COMMAND lcov --directory '${PROJECT_BINARY_DIR}' --capture --output-file coverage.info
            # Take only coverage of the core library, not apps
            COMMAND lcov --extract coverage.info '${PROJECT_SOURCE_DIR}/src/h3lib/*' --output-file coverage.cleaned.info
            # Generate output html at coverage/
            COMMAND genhtml -o coverage coverage.cleaned.info --title 'h3 coverage'
            COMMENT "Coverage done"
            )
        add_custom_target(clean-coverage
            # Before running coverage, clear all counters
            COMMAND lcov --directory '${PROJECT_BINARY_DIR}' --zerocounters
            COMMENT "Zeroing counters"
            )
    endif()

    macro(add_h3_test_common name srcfile)
        # need to actually make the test target
        if(NOT TARGET ${name})
            add_h3_executable(${name} ${srcfile} ${APP_SOURCE_FILES})
        endif()

        math(EXPR test_number "${test_number}+1")
    endmacro()

    macro(add_h3_test name srcfile)
        add_h3_test_common(${name} ${srcfile})
        add_test(NAME ${name}_test${test_number} COMMAND ${name})

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
        add_test(NAME ${name}_test${test_number}
            COMMAND ${SHELL} "$<TARGET_FILE:${name}> < ${argfile}"
            )
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
            COMMAND $<TARGET_FILE:${name}> ${arg}
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
    add_h3_test(testH3ToChildren src/apps/testapps/testH3ToChildren.c)
    add_h3_test(testMaxH3ToChildrenSize src/apps/testapps/testMaxH3ToChildrenSize.c)
    add_h3_test(testH3Index src/apps/testapps/testH3Index.c)
    add_h3_test(testH3Api src/apps/testapps/testH3Api.c)
    add_h3_test(testH3SetToLinkedGeo src/apps/testapps/testH3SetToLinkedGeo.c)
    add_h3_test(testH3SetToVertexGraph src/apps/testapps/testH3SetToVertexGraph.c)
    add_h3_test(testLinkedGeo src/apps/testapps/testLinkedGeo.c)
    add_h3_test(testPolyfill src/apps/testapps/testPolyfill.c)
    add_h3_test(testVertexGraph src/apps/testapps/testVertexGraph.c)
    add_h3_test(testH3UniEdge src/apps/testapps/testH3UniEdge.c)
    add_h3_test(testGeoCoord src/apps/testapps/testGeoCoord.c)
    add_h3_test(testBBox src/apps/testapps/testBBox.c)

    add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 0)
    add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 1)
    add_h3_test_with_arg(testH3NeighborRotations src/apps/testapps/testH3NeighborRotations.c 2)

    # Miscellaneous testing applications
    add_h3_executable(mkRandGeo src/apps/testapps/mkRandGeo.c ${APP_SOURCE_FILES})
    add_h3_executable(mkRandGeoBoundary src/apps/testapps/mkRandGeoBoundary.c ${APP_SOURCE_FILES})

    # Benchmarks
    add_custom_target(benchmarks)

    macro(add_h3_benchmark name srcfile)
        add_h3_executable(${name} ${srcfile} ${APP_SOURCE_FILES})
        add_custom_target(bench_${name} COMMAND ${name})
        add_dependencies(benchmarks bench_${name})
    endmacro()

    add_h3_benchmark(benchmarkH3Api src/apps/benchmarks/benchmarkH3Api.c)
    add_h3_benchmark(benchmarkPolyfill src/apps/benchmarks/benchmarkPolyfill.c)
endif()

# Installing the library and filters system-wide.
install(TARGETS h3.1
    geoToH3
    h3ToComponents
    h3ToGeo
    h3ToGeoBoundary
    hexRange
    kRing
    h3ToGeoBoundaryHier
    h3ToGeoHier
    h3ToHier
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static)
# Only the h3api.h header is needed by applications using H3.
install(FILES src/h3lib/include/h3api.h DESTINATION include/h3)
