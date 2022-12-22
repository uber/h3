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
 *  usage: `testCellToLocalIj`
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
        t_assert(cellToLocalIjk(pent1, bc1, &ijk) == E_SUCCESS,
                 "got ijk for base cells 4 and 15");
        t_assert(_ijkMatches(&ijk, &UNIT_VECS[2]) == 1,
                 "neighboring base cell at 0,1,0");
    }

    TEST(ijBaseCells) {
        CoordIJ ij = {.i = 0, .j = 0};
        H3Index origin = 0x8029fffffffffff;
        H3Index retrieved;
        t_assert(
            H3_EXPORT(localIjToCell)(origin, &ij, 0, &retrieved) == E_SUCCESS,
            "got origin back");
        t_assert(retrieved == 0x8029fffffffffff, "origin matches self");
        ij.i = 1;
        t_assert(
            H3_EXPORT(localIjToCell)(origin, &ij, 0, &retrieved) == E_SUCCESS,
            "got offset index");
        t_assert(retrieved == 0x8051fffffffffff,
                 "modified index matches expected");
        ij.i = 2;
        t_assert(
            H3_EXPORT(localIjToCell)(origin, &ij, 0, &retrieved) == E_FAILED,
            "out of range base cell (1)");
        ij.i = 0;
        ij.j = 2;
        t_assert(
            H3_EXPORT(localIjToCell)(origin, &ij, 0, &retrieved) == E_FAILED,
            "out of range base cell (2)");
        ij.i = -2;
        ij.j = -2;
        t_assert(
            H3_EXPORT(localIjToCell)(origin, &ij, 0, &retrieved) == E_FAILED,
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
            const H3Error err =
                H3_EXPORT(localIjToCell)(expected[0], &coords[i], 0, &result);
            if (expected[i] == H3_NULL) {
                t_assert(err != 0, "coordinates out of range");
            } else {
                t_assert(err == 0, "coordinates in range");
                t_assert(result == expected[i], "result matches expectation");
            }
        }
    }

    TEST(cellToLocalIjFailed) {
        CoordIJ ij;

        t_assert(H3_EXPORT(cellToLocalIj)(bc1, bc1, 0, &ij) == 0,
                 "found IJ (1)");
        t_assert(ij.i == 0 && ij.j == 0, "ij correct (1)");
        t_assert(H3_EXPORT(cellToLocalIj)(bc1, pent1, 0, &ij) == 0,
                 "found IJ (2)");
        t_assert(ij.i == 1 && ij.j == 0, "ij correct (2)");
        t_assert(H3_EXPORT(cellToLocalIj)(bc1, bc2, 0, &ij) == 0,
                 "found IJ (3)");
        t_assert(ij.i == 0 && ij.j == -1, "ij correct (3)");
        t_assert(H3_EXPORT(cellToLocalIj)(bc1, bc3, 0, &ij) == 0,
                 "found IJ (4)");
        t_assert(ij.i == -1 && ij.j == 0, "ij correct (4)");
        t_assert(H3_EXPORT(cellToLocalIj)(pent1, bc3, 0, &ij) == E_FAILED,
                 "found IJ (5)");
    }

    TEST(cellToLocalIjInvalid) {
        CoordIJ ij;
        H3Index invalidIndex = 0x7fffffffffffffff;
        H3_SET_RESOLUTION(invalidIndex, H3_GET_RESOLUTION(bc1));
        t_assert(H3_EXPORT(cellToLocalIj)(bc1, invalidIndex, 0, &ij) ==
                     E_CELL_INVALID,
                 "invalid index");
        t_assert(H3_EXPORT(cellToLocalIj)(0x7fffffffffffffff, bc1, 0, &ij) ==
                     E_RES_MISMATCH,
                 "invalid origin");
        t_assert(
            H3_EXPORT(cellToLocalIj)(0x7fffffffffffffff, 0x7fffffffffffffff, 0,
                                     &ij) == E_CELL_INVALID,
            "invalid origin and index");
    }

    TEST(localIjToCellInvalid) {
        CoordIJ ij = {0, 0};
        H3Index index;
        t_assert(H3_EXPORT(localIjToCell)(0x7fffffffffffffff, &ij, 0, &index) ==
                     E_CELL_INVALID,
                 "invalid origin for ijToH3");
    }

    /**
     * Tests for INVALID_DIGIT being detected and failed on in various cases.
     */
    TEST(indexOnPentInvalid) {
        H3Index onPentInvalid;
        setH3Index(&onPentInvalid, 1, 4, INVALID_DIGIT);
        H3Index offPent;
        setH3Index(&offPent, 1, 3, CENTER_DIGIT);
        CoordIJ ij;
        t_assert(H3_EXPORT(cellToLocalIj)(offPent, onPentInvalid, 0, &ij) ==
                     E_CELL_INVALID,
                 "invalid index on pentagon");

        H3Index onPentValid;
        setH3Index(&onPentValid, 1, 4, CENTER_DIGIT);
        t_assert(H3_EXPORT(cellToLocalIj)(onPentInvalid, onPentValid, 0, &ij) ==
                     E_CELL_INVALID,
                 "invalid both on pentagon");
        t_assert(H3_EXPORT(cellToLocalIj)(onPentValid, onPentInvalid, 0, &ij) ==
                     E_CELL_INVALID,
                 "invalid both on pentagon");

        ij.i = 0;
        ij.j = 0;
        H3Index out;
        t_assert(H3_EXPORT(localIjToCell)(onPentInvalid, &ij, 0, &out) ==
                     E_CELL_INVALID,
                 "invalid both on pentagon");

        ij.i = 3;
        ij.j = 3;
        t_assert(H3_EXPORT(localIjToCell)(onPentInvalid, &ij, 0, &out) ==
                     E_CELL_INVALID,
                 "invalid origin on pentagon");
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
                        int internalIjFailed = H3_EXPORT(cellToLocalIj)(
                            internalOrigin, testIndex, 0, &internalIj);
                        CoordIJ externalIj;
                        int externalIjFailed = H3_EXPORT(cellToLocalIj)(
                            externalOrigin, testIndex, 0, &externalIj);

                        t_assert(
                            (bool)internalIjFailed == (bool)externalIjFailed,
                            "internal/external failed matches when getting IJ");

                        if (internalIjFailed) {
                            continue;
                        }

                        H3Index internalIndex;
                        int internalIjFailed2 = H3_EXPORT(localIjToCell)(
                            internalOrigin, &internalIj, 0, &internalIndex);
                        H3Index externalIndex;
                        int externalIjFailed2 = H3_EXPORT(localIjToCell)(
                            externalOrigin, &externalIj, 0, &externalIndex);

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

    TEST(invalidMode) {
        CoordIJ ij;
        H3Index cell = 0x85283473fffffff;
        t_assertSuccess(H3_EXPORT(cellToLocalIj)(cell, cell, 0, &ij));

        for (uint32_t i = 1; i <= 32; i++) {
            CoordIJ ij2;
            t_assert(H3_EXPORT(cellToLocalIj)(cell, cell, i, &ij2) ==
                         E_OPTION_INVALID,
                     "Invalid mode fail for cellToLocalIj");
            H3Index cell2;
            t_assert(H3_EXPORT(localIjToCell)(cell, &ij2, i, &cell2) ==
                         E_OPTION_INVALID,
                     "Invalid mode fail for cellToLocalIj");
        }
    }

    TEST(invalid_negativeIj) {
        H3Index index = 0x200f202020202020;
        CoordIJ ij = {.i = -14671840, .j = -2147483648};
        H3Index out;
        t_assert(H3_EXPORT(localIjToCell)(index, &ij, 0, &out) == E_FAILED,
                 "Negative I and J components fail");
    }

    TEST(localIjToCell_overflow_i) {
        H3Index origin;
        setH3Index(&origin, 2, 2, CENTER_DIGIT);
        CoordIJ ij = {.i = INT32_MIN, .j = INT32_MAX};
        H3Index out;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "High magnitude I and J components fail");
    }

    TEST(localIjToCell_overflow_j) {
        H3Index origin;
        setH3Index(&origin, 2, 2, CENTER_DIGIT);
        CoordIJ ij = {.i = INT32_MAX, .j = INT32_MIN};
        H3Index out;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "High magnitude J and I components fail");
    }

    TEST(localIjToCell_overflow_ij) {
        H3Index origin;
        setH3Index(&origin, 2, 2, CENTER_DIGIT);
        CoordIJ ij = {.i = INT32_MIN, .j = INT32_MIN};
        H3Index out;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "High magnitude J and I components fail");
    }

    TEST(localIjToCell_overflow_particularCases) {
        H3Index origin;
        setH3Index(&origin, 2, 2, CENTER_DIGIT);
        H3Index originRes3;
        setH3Index(&originRes3, 2, 2, CENTER_DIGIT);

        CoordIJ ij = {.i = 553648127, .j = -2145378272};
        H3Index out;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "Particular high magnitude J and I components fail (1)");

        ij.i = INT32_MAX - 10;
        ij.j = -11;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "Particular high magnitude J and I components fail (2)");

        ij.i = 553648127;
        ij.j = -2145378272;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "Particular high magnitude J and I components fail (3)");

        ij.i = INT32_MAX - 10;
        ij.j = -10;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "Particular high magnitude J and I components fail (4)");

        ij.i = INT32_MAX - 10;
        ij.j = -9;
        t_assert(H3_EXPORT(localIjToCell)(origin, &ij, 0, &out) == E_FAILED,
                 "Particular high magnitude J and I components fail (5)");
    }
}
