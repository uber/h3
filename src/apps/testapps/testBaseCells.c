/*
 * Copyright 2017-2020 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>

#include "baseCells.h"
#include "h3api.h"
#include "test.h"

SUITE(baseCells) {
    TEST(getRes0Cells) {
        int count = H3_EXPORT(res0CellCount)();
        H3Index *indexes = malloc(count * sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(getRes0Cells)(indexes));
        t_assert(indexes[0] == 0x8001fffffffffff, "correct first basecell");
        t_assert(indexes[121] == 0x80f3fffffffffff, "correct last basecell");
        free(indexes);
    }
}
