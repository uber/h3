# Copyright 2018 Uber Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if(__check_vla)
  return()
endif()
set(__check_vla 1)

function(check_vla var)
    if(NOT DEFINED ${var})
        try_compile(have_vla
            ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/vla_test
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vla_test.c)
    endif()
    set(description "Checking for VLA support")
    if(have_vla)
        set(${var} ON PARENT_SCOPE)
        message(STATUS "${description} - Success")
    else()
        message(STATUS "${description} - Failed")
    endif()
endfunction()
