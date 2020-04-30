/*
 * Copyright 2018 Uber Technologies, Inc.
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
 * @brief tests IJ grid functions and IJK distance functions.
 *
 *  usage: `testCoordIj`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algos.h"
#include "baseCells.h"
#include "constants.h"
#include "h3Index.h"
#include "h3api.h"
#include "localij.h"
#include "test.h"
#include "utility.h"

SUITE(coordIj) {
    TEST(ijkToIj_zero) {
        CoordIJK ijk = {0};
        CoordIJ ij = {0};

        ijkToIj(&ijk, &ij);
        t_assert(ij.i == 0, "ij.i zero");
        t_assert(ij.j == 0, "ij.j zero");

        ijToIjk(&ij, &ijk);
        t_assert(ijk.i == 0, "ijk.i zero");
        t_assert(ijk.j == 0, "ijk.j zero");
        t_assert(ijk.k == 0, "ijk.k zero");
    }

    TEST(ijkToIj_roundtrip) {
        for (Direction dir = CENTER_DIGIT; dir < NUM_DIGITS; dir++) {
            CoordIJK ijk = {0};
            _neighbor(&ijk, dir);

            CoordIJ ij = {0};
            ijkToIj(&ijk, &ij);

            CoordIJK recovered = {0};
            ijToIjk(&ij, &recovered);

            t_assert(_ijkMatches(&ijk, &recovered),
                     "got same ijk coordinates back");
        }
    }

    TEST(ijkToCube_roundtrip) {
        for (Direction dir = CENTER_DIGIT; dir < NUM_DIGITS; dir++) {
            CoordIJK ijk = {0};
            _neighbor(&ijk, dir);
            CoordIJK original = {ijk.i, ijk.j, ijk.k};

            ijkToCube(&ijk);
            cubeToIjk(&ijk);

            t_assert(_ijkMatches(&ijk, &original),
                     "got same ijk coordinates back");
        }
    }
}
