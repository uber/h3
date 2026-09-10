#!/bin/sh
# Copyright 2026 Uber Technologies, Inc.
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

# This is a wrapper that applies https://github.com/mull-project/mull/issues/1133
# It skips mutations that have already been tested by other tests, and instead
# just reports them as having been excluded (exit 1). This makes the test report
# cumulative, and more importantly avoids spending time testing cases that have
# already been excluded by quicker tests.

set -u
db=${H3_MULL_SKIP_DB-}
real=${H3_MULL_REAL_EXEC-}
mutant=$(env | awk -F= '$2=="1" && $1 ~ /^(cxx_|negate_)/ {print $1; exit}')

if [ -n "$mutant" ] && [ -s "$db" ]; then
    # 1=failed, 3=timeout, 4=crashed, 5=abnormal exit
    hit=$(sqlite3 -readonly "$db" \
    "SELECT 1 FROM mutant WHERE mutant_id='$mutant' \
    AND execution_status in (1,3,4,5) LIMIT 1;" 2>/dev/null)
    if [ "$hit" = 1 ]; then
      # echo "$real $mutant" >> /tmp/mull_squash.txt
      exit 1
    # else
    #   echo "$real $mutant" >> /tmp/mull_continue.txt
    fi
fi

exec "$real" "$@"
