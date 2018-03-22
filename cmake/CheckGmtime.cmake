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

if(__check_gmtime)
  return()
endif()
set(__check_gmtime 1)

include(CheckFunctionExists)

function(check_gmtime var)
    if(NOT DEFINED have_reentrant_gmtime)
        if(MSVC)
            check_function_exists(gmtime_s have_reentrant_gmtime)
        else()
            check_function_exists(gmtime_r have_reentrant_gmtime)
        endif()
    endif()

    if(have_reentrant_gmtime)
        set(${var} ON PARENT_SCOPE)
    endif()
endfunction()
