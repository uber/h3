# Copyright 2018 Uber Technologies, Inc.
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

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

project(h3examples)

find_package(h3 CONFIG REQUIRED)

set(EXAMPLES
    index
    distance
    neighbors
    compact
    edge)

set(EXECUTABLE_OUTPUT_PATH bin)
link_libraries(h3::h3)

# Build each example
foreach(name ${EXAMPLES})
    add_executable(${name} "${name}.c")
endforeach()

# Testing the examples
include(CTest)
if(BUILD_TESTING)
    include(../cmake/TestWrapValgrind.cmake)

    enable_testing()

    foreach(name ${EXAMPLES})
        add_test(NAME ${name} COMMAND ${TEST_WRAPPER} "$<TARGET_FILE:${name}>")
    endforeach()
endif()
