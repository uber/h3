/*
 * Copyright 2017-2018 Uber Technologies, Inc.
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
#include "constants.h"
#include "h3api.h"
#include "test.h"

SUITE(baseCells) {
    TEST(getBaseCells) {
        H3Index* baseCells = calloc(NUM_BASE_CELLS, sizeof(H3Index));
        H3_EXPORT(getBaseCells)(baseCells);
        t_assert(baseCells[0] == 0x8001fffffffffff, "correct first basecell");
        t_assert(baseCells[121] == 0x80f3fffffffffff, "correct last basecell");
    }
}
