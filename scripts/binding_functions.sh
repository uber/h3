#!/usr/bin/env bash
# Copyright 2017 Uber Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This file is used to extract lists of functions that should be provided
# by bindings of the H3 library. It is invoked by the `binding-functions`
# make target and produces a file `binding-functions`.

set -eo pipefail

cat "src/h3lib/include/h3api.h" | sed -n '/@defgroup/s/.*@defgroup \([A-Za-z0-9_]*\) .*/\1/gp' > binding-functions
