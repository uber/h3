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

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
if(NOT WIN32)
    # Compiler options are set only on non-Windows, since these options
    # are not correct for MSVC.
    set(CMAKE_C_FLAGS_INIT "-Wall -Werror")
    string(CONCAT CMAKE_C_FLAGS_DEBUG_INIT
           "-g -gdwarf-2 -g3 -O0 -fno-inline -fno-eliminate-unused-debug-types "
           "--coverage")
endif()
