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
    TEST(getRes0Indexes) {
        int count = H3_EXPORT(res0IndexCount)();
        H3Index* indexes = malloc(count * sizeof(H3Index));
        H3_EXPORT(getRes0Indexes)(indexes);
        t_assert(indexes[0] == 0x8001fffffffffff, "correct first basecell");
        t_assert(indexes[121] == 0x80f3fffffffffff, "correct last basecell");
        free(indexes);
    }

    TEST(baseCellToCCWrot60) {
        // a few random spot-checks
        t_assert(_baseCellToCCWrot60(16, 0) == 0, "got expected rotation");
        t_assert(_baseCellToCCWrot60(32, 0) == 3, "got expected rotation");
        t_assert(_baseCellToCCWrot60(7, 3) == 1, "got expected rotation");
    }

    TEST(baseCellToCCWrot60_invalid) {
        t_assert(_baseCellToCCWrot60(16, 42) == INVALID_ROTATIONS, "should return invalid rotation for invalid face");
        t_assert(_baseCellToCCWrot60(16, -1) == INVALID_ROTATIONS, "should return invalid rotation for invalid face (negative)");
        t_assert(_baseCellToCCWrot60(1, 0) == INVALID_ROTATIONS, "should return invalid rotation for base cell not appearing on face");
    }
}
