/*
 * Copyright 2017-2018, 2020-2021 Uber Technologies, Inc.
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
/** @file
 * @brief tests H3 function `gridDisk` and `gridDiskDistances`
 *
 *  usage: `testGridDiskInternal`
 */

#include <stdlib.h>

#include "algos.h"
#include "baseCells.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

SUITE(gridDiskInternal) {
    TEST(h3NeighborRotations_identity) {
        // This is not used in gridDisk, but it's helpful for it to make sense.
        H3Index origin = 0x811d7ffffffffffL;
        int rotations = 0;
        H3Index out;
        t_assertSuccess(
            h3NeighborRotations(origin, CENTER_DIGIT, &rotations, &out));
        t_assert(out == origin, "Moving to self goes to self");
        t_assert(rotations == 0, "Expected rotations");
    }

    TEST(h3NeighborRotations_rotationsOverflow) {
        // Check for possible signed integer overflow of `rotations`
        H3Index origin;
        setH3Index(&origin, 0, 0, CENTER_DIGIT);
        // A multiple of 6, so effectively no rotation. Very close
        // to INT32_MAX.
        int rotations = 2147483646;
        H3Index out;
        t_assertSuccess(
            h3NeighborRotations(origin, K_AXES_DIGIT, &rotations, &out));
        H3Index expected;
        // Determined by looking at the base cell table
        setH3Index(&expected, 0, 1, CENTER_DIGIT);
        t_assert(out == expected, "Expected neighbor");
        t_assert(rotations == 5, "Expected rotations value");
    }

    TEST(h3NeighborRotations_rotationsOverflow2) {
        // Check for possible signed integer overflow of `rotations`
        H3Index origin;
        setH3Index(&origin, 0, 4, CENTER_DIGIT);
        // This modulo 6 is 1.
        int rotations = INT32_MAX;
        H3Index out;
        // This will try to move in the K direction off of origin,
        // which will be adjusted to the IK direction.
        t_assertSuccess(
            h3NeighborRotations(origin, JK_AXES_DIGIT, &rotations, &out));
        H3Index expected;
        // Determined by looking at the base cell table
        setH3Index(&expected, 0, 0, CENTER_DIGIT);
        t_assert(out == expected, "Expected neighbor");
        // 1 (original value) + 4 (newRotations for IK direction) + 1 (applied
        // when adjusting to the IK direction) = 6, 6 modulo 6 = 0
        t_assert(rotations == 0, "Expected rotations value");
    }

    TEST(h3NeighborRotations_invalid) {
        H3Index origin = 0x811d7ffffffffffL;
        int rotations = 0;
        H3Index out;
        t_assert(h3NeighborRotations(origin, -1, &rotations, &out) == E_FAILED,
                 "Invalid direction fails (-1)");
        t_assert(h3NeighborRotations(origin, 7, &rotations, &out) == E_FAILED,
                 "Invalid direction fails (7)");
        t_assert(h3NeighborRotations(origin, 100, &rotations, &out) == E_FAILED,
                 "Invalid direction fails (100)");
    }

    TEST(cwOffsetPent) {
        // Try to find a case where h3NeighborRotations would not pass the
        // cwOffsetPent check, and would hit a line marked as unreachable.

        // To do this, we need to find a case that would move from one
        // non-pentagon base cell into the deleted k-subsequence of a pentagon
        // base cell, and neither of the cwOffsetPent values are the original
        // base cell's face.

        for (int pentagon = 0; pentagon < NUM_BASE_CELLS; pentagon++) {
            if (!_isBaseCellPentagon(pentagon)) {
                continue;
            }

            for (int neighbor = 0; neighbor < NUM_BASE_CELLS; neighbor++) {
                FaceIJK homeFaceIjk;
                _baseCellToFaceIjk(neighbor, &homeFaceIjk);
                int neighborFace = homeFaceIjk.face;

                // Only direction 2 needs to be checked, because that is the
                // only direction where we can move from digit 2 to digit 1, and
                // into the deleted k subsequence.
                t_assert(
                    _getBaseCellNeighbor(neighbor, J_AXES_DIGIT) != pentagon ||
                        _baseCellIsCwOffset(pentagon, neighborFace),
                    "cwOffsetPent is reachable");
            }
        }
    }
}
