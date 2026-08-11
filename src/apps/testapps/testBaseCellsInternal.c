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

SUITE(baseCellsInternal) {
    TEST(baseCellToCCWrot60) {
        // a few random spot-checks
        t_assert(_baseCellToCCWrot60(16, 0) == 0, "got expected rotation");
        t_assert(_baseCellToCCWrot60(32, 0) == 3, "got expected rotation");
        t_assert(_baseCellToCCWrot60(7, 3) == 1, "got expected rotation");
    }

    TEST(baseCellToCCWrot60_invalid) {
        t_assert(_baseCellToCCWrot60(16, 42) == INVALID_ROTATIONS,
                 "should return invalid rotation for invalid face");
        t_assert(_baseCellToCCWrot60(16, -1) == INVALID_ROTATIONS,
                 "should return invalid rotation for invalid face (negative)");
        t_assert(_baseCellToCCWrot60(1, 0) == INVALID_ROTATIONS,
                 "should return invalid rotation for base cell not appearing "
                 "on face");
    }

    TEST(isBaseCellPentagon_invalid) {
        t_assert(_isBaseCellPentagon(-1) == false,
                 "isBaseCellPentagon handles negative");
        t_assert(_isBaseCellPentagon(NUM_BASE_CELLS) == false,
                 "isBaseCellPentagon handles too large");
    }

    TEST(baseCellIsPentagon_matchesBaseCellData) {
        // baseCellIsPentagon duplicates the isPentagon field of baseCellData so
        // that hot paths get a compact lookup. Nothing in the type system keeps
        // the two in sync, so assert it here.
        for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
            t_assert(
                baseCellIsPentagon[bc] == (bool)baseCellData[bc].isPentagon,
                "baseCellIsPentagon agrees with baseCellData");
        }
    }

    TEST(baseCellIsPentagon_paddingIsFalse) {
        // The array covers the whole 7 bit base cell range so an unvalidated
        // base cell number read out of an H3Index cannot index out of bounds.
        // Everything past the real base cells must read false.
        for (int bc = NUM_BASE_CELLS; bc < NUM_BASE_CELL_VALUES; bc++) {
            t_assert(baseCellIsPentagon[bc] == false,
                     "padding entries are not pentagons");
        }
    }
}
