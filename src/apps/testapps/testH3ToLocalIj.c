/*
 * Copyright 2018-2019 Uber Technologies, Inc.
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
 * @brief tests H3 index to local IJ and IJK+ grid functions.
 *
 *  usage: `testH3ToLocalIj`
 */

#include <h3api.h>
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

SUITE(h3ToLocalIj) {
    // Some indexes that represent base cells. Base cells
    // are hexagons except for `pent1`.
    H3Index bc1 = H3_INIT;
    setH3Index(&bc1, 0, 15, 0);

    H3Index bc2 = H3_INIT;
    setH3Index(&bc2, 0, 8, 0);

    H3Index bc3 = H3_INIT;
    setH3Index(&bc3, 0, 31, 0);

    H3Index pent1 = H3_INIT;
    setH3Index(&pent1, 0, 4, 0);

    TEST(ijkBaseCells) {
        CoordIJK ijk;
        t_assert(h3ToLocalIjk(pent1, bc1, &ijk) == 0,
                 "got ijk for base cells 4 and 15");
        t_assert(_ijkMatches(&ijk, &UNIT_VECS[2]) == 1,
                 "neighboring base cell at 0,1,0");
    }

    TEST(ijBaseCells) {
        CoordIJ ij = {.i = 0, .j = 0};
        H3Index origin = 0x8029fffffffffff;
        H3Index retrieved;
        t_assert(
            H3_EXPORT(experimentalLocalIjToH3)(origin, &ij, &retrieved) == 0,
            "got origin back");
        t_assert(retrieved == 0x8029fffffffffff, "origin matches self");
        ij.i = 1;
        t_assert(
            H3_EXPORT(experimentalLocalIjToH3)(origin, &ij, &retrieved) == 0,
            "got offset index");
        t_assert(retrieved == 0x8051fffffffffff,
                 "modified index matches expected");
        ij.i = 2;
        t_assert(
            H3_EXPORT(experimentalLocalIjToH3)(origin, &ij, &retrieved) != 0,
            "out of range base cell (1)");
        ij.i = 0;
        ij.j = 2;
        t_assert(
            H3_EXPORT(experimentalLocalIjToH3)(origin, &ij, &retrieved) != 0,
            "out of range base cell (2)");
        ij.i = -2;
        ij.j = -2;
        t_assert(
            H3_EXPORT(experimentalLocalIjToH3)(origin, &ij, &retrieved) != 0,
            "out of range base cell (3)");
    }

    TEST(ijOutOfRange) {
        const int numCoords = 7;
        const CoordIJ coords[] = {{0, 0}, {1, 0},  {2, 0}, {3, 0},
                                  {4, 0}, {-4, 0}, {0, 4}};
        const H3Index expected[] = {0x81283ffffffffff,
                                    0x81293ffffffffff,
                                    0x8150bffffffffff,
                                    0x8151bffffffffff,
                                    H3_NULL,
                                    H3_NULL,
                                    H3_NULL};

        for (int i = 0; i < numCoords; i++) {
            H3Index result;
            const int err = H3_EXPORT(experimentalLocalIjToH3)(
                expected[0], &coords[i], &result);
            if (expected[i] == H3_NULL) {
                t_assert(err != 0, "coordinates out of range");
            } else {
                t_assert(err == 0, "coordinates in range");
                t_assert(result == expected[i], "result matches expectation");
            }
        }
    }

    TEST(experimentalH3ToLocalIjFailed) {
        CoordIJ ij;

        t_assert(H3_EXPORT(experimentalH3ToLocalIj)(bc1, bc1, &ij) == 0,
                 "found IJ (1)");
        t_assert(ij.i == 0 && ij.j == 0, "ij correct (1)");
        t_assert(H3_EXPORT(experimentalH3ToLocalIj)(bc1, pent1, &ij) == 0,
                 "found IJ (2)");
        t_assert(ij.i == 1 && ij.j == 0, "ij correct (2)");
        t_assert(H3_EXPORT(experimentalH3ToLocalIj)(bc1, bc2, &ij) == 0,
                 "found IJ (3)");
        t_assert(ij.i == 0 && ij.j == -1, "ij correct (3)");
        t_assert(H3_EXPORT(experimentalH3ToLocalIj)(bc1, bc3, &ij) == 0,
                 "found IJ (4)");
        t_assert(ij.i == -1 && ij.j == 0, "ij correct (4)");
        t_assert(H3_EXPORT(experimentalH3ToLocalIj)(pent1, bc3, &ij) != 0,
                 "found IJ (5)");
    }

    /**
     * Test that coming from the same direction outside the pentagon is handled
     * the same as coming from the same direction inside the pentagon.
     */
    TEST(onOffPentagonSame) {
        for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
            for (int res = 1; res <= MAX_H3_RES; res++) {
                // K_AXES_DIGIT is the first internal direction, and it's also
                // invalid for pentagons, so skip to next.
                Direction startDir = K_AXES_DIGIT;
                if (_isBaseCellPentagon(bc)) {
                    startDir++;
                }

                for (Direction dir = startDir; dir < NUM_DIGITS; dir++) {
                    H3Index internalOrigin;
                    setH3Index(&internalOrigin, res, bc, dir);

                    H3Index externalOrigin;
                    setH3Index(&externalOrigin, res,
                               _getBaseCellNeighbor(bc, dir), CENTER_DIGIT);

                    for (Direction testDir = startDir; testDir < NUM_DIGITS;
                         testDir++) {
                        H3Index testIndex;
                        setH3Index(&testIndex, res, bc, testDir);

                        CoordIJ internalIj;
                        int internalIjFailed =
                            H3_EXPORT(experimentalH3ToLocalIj)(
                                internalOrigin, testIndex, &internalIj);
                        CoordIJ externalIj;
                        int externalIjFailed =
                            H3_EXPORT(experimentalH3ToLocalIj)(
                                externalOrigin, testIndex, &externalIj);

                        t_assert(
                            (bool)internalIjFailed == (bool)externalIjFailed,
                            "internal/external failed matches when getting IJ");

                        if (internalIjFailed) {
                            continue;
                        }

                        H3Index internalIndex;
                        int internalIjFailed2 =
                            H3_EXPORT(experimentalLocalIjToH3)(
                                internalOrigin, &internalIj, &internalIndex);
                        H3Index externalIndex;
                        int externalIjFailed2 =
                            H3_EXPORT(experimentalLocalIjToH3)(
                                externalOrigin, &externalIj, &externalIndex);

                        t_assert(
                            (bool)internalIjFailed2 == (bool)externalIjFailed2,
                            "internal/external failed matches when getting "
                            "index");

                        if (internalIjFailed2) {
                            continue;
                        }

                        t_assert(internalIndex == externalIndex,
                                 "internal/external index matches");
                    }
                }
            }
        }
    }
}
