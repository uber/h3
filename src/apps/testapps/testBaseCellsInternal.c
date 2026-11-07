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
        t_assert(_baseCellToCCWrot60(16, NUM_ICOSA_FACES) == INVALID_ROTATIONS,
                 "should return invalid rotation for one past the last face");
        t_assert(_baseCellToCCWrot60(1, 0) == INVALID_ROTATIONS,
                 "should return invalid rotation for base cell not appearing "
                 "on face");
    }

    TEST(isBaseCellPentagon_invalid) {
        t_assert(_isBaseCellPentagon(-1) == false,
                 "isBaseCellPentagon handles negative");
    }

    TEST(getBaseCellNeighbor_invalid) {
        t_assert(_getBaseCellNeighbor(NUM_BASE_CELLS, CENTER_DIGIT) ==
                     INVALID_BASE_CELL,
                 "getBaseCellNeighbor handles one past the last base cell");
        t_assert(_getBaseCellNeighbor(-1, CENTER_DIGIT) == INVALID_BASE_CELL,
                 "getBaseCellNeighbor handles negative base cell");
    }

    TEST(baseCellToFaceIjk_invalid) {
        FaceIJK fijk = {1, {1, 1, 1}};
        _baseCellToFaceIjk(NUM_BASE_CELLS, &fijk);
        t_assert(fijk.face == 0 && fijk.coord.i == 0 && fijk.coord.j == 0 &&
                     fijk.coord.k == 0,
                 "baseCellToFaceIjk zeroes output for one past the last base "
                 "cell");
        _baseCellToFaceIjk(-1, &fijk);
        t_assert(fijk.face == 0 && fijk.coord.i == 0 && fijk.coord.j == 0 &&
                     fijk.coord.k == 0,
                 "baseCellToFaceIjk zeroes output for negative base cell");
    }

    TEST(baseCellIsCwOffset_invalid) {
        t_assert(_baseCellIsCwOffset(NUM_BASE_CELLS, 0) == false,
                 "baseCellIsCwOffset handles one past the last base cell");
        t_assert(_baseCellIsCwOffset(-1, 0) == false,
                 "baseCellIsCwOffset handles negative base cell");
    }
}
