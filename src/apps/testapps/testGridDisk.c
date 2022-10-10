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
 *  usage: `testGridDisk`
 */

#include <stdlib.h>

#include "algos.h"
#include "baseCells.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

static void gridDisk_equals_gridDiskDistancesSafe_assertions(H3Index h3) {
    for (int k = 0; k < 3; k++) {
        int64_t kSz;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &kSz));

        H3Index *neighbors = calloc(kSz, sizeof(H3Index));
        int *distances = calloc(kSz, sizeof(int));
        t_assertSuccess(
            H3_EXPORT(gridDiskDistances)(h3, k, neighbors, distances));

        H3Index *internalNeighbors = calloc(kSz, sizeof(H3Index));
        int *internalDistances = calloc(kSz, sizeof(int));
        t_assertSuccess(H3_EXPORT(gridDiskDistancesSafe)(
            h3, k, internalNeighbors, internalDistances));

        int found = 0;
        int internalFound = 0;
        for (int64_t iNeighbor = 0; iNeighbor < kSz; iNeighbor++) {
            if (neighbors[iNeighbor] != 0) {
                found++;

                for (int64_t iInternal = 0; iInternal < kSz; iInternal++) {
                    if (internalNeighbors[iInternal] == neighbors[iNeighbor]) {
                        internalFound++;

                        t_assert(distances[iNeighbor] ==
                                     internalDistances[iInternal],
                                 "External and internal agree on "
                                 "distance");

                        break;
                    }
                }

                t_assert(found == internalFound,
                         "External and internal implementations "
                         "produce same output");
            }
        }

        free(internalDistances);
        free(internalNeighbors);

        free(distances);
        free(neighbors);
    }
}

SUITE(gridDisk) {
    TEST(gridDisk0) {
        LatLng sf = {0.659966917655, 2 * 3.14159 - 2.1364398519396};
        H3Index sfHex0;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, 0, &sfHex0));

        H3Index k1[] = {0, 0, 0, 0, 0, 0, 0};
        int k1Dist[] = {0, 0, 0, 0, 0, 0, 0};
        H3Index expectedK1[] = {0x8029fffffffffff, 0x801dfffffffffff,
                                0x8013fffffffffff, 0x8027fffffffffff,
                                0x8049fffffffffff, 0x8051fffffffffff,
                                0x8037fffffffffff};
        t_assertSuccess(H3_EXPORT(gridDiskDistances)(sfHex0, 1, k1, k1Dist));

        for (int i = 0; i < 7; i++) {
            t_assert(k1[i] != 0, "index is populated");
            int inList = 0;
            for (int j = 0; j < 7; j++) {
                if (k1[i] == expectedK1[j]) {
                    t_assert(k1Dist[i] == (k1[i] == sfHex0 ? 0 : 1),
                             "distance is as expected");
                    inList++;
                }
            }
            t_assert(inList == 1, "index found in expected set");
        }
    }

    TEST(gridDisk0_PolarPentagon) {
        H3Index polar;
        setH3Index(&polar, 0, 4, 0);
        H3Index k2[] = {0, 0, 0, 0, 0, 0, 0};
        int k2Dist[] = {0, 0, 0, 0, 0, 0, 0};
        H3Index expectedK2[] = {0x8009fffffffffff,
                                0x8007fffffffffff,
                                0x8001fffffffffff,
                                0x8011fffffffffff,
                                0x801ffffffffffff,
                                0x8019fffffffffff,
                                0};
        t_assertSuccess(H3_EXPORT(gridDiskDistances)(polar, 1, k2, k2Dist));

        int k2present = 0;
        for (int i = 0; i < 7; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 7; j++) {
                    if (k2[i] == expectedK2[j]) {
                        t_assert(k2Dist[i] == (k2[i] == polar ? 0 : 1),
                                 "distance is as expected");
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 6, "pentagon has 5 neighbors");
    }

    TEST(gridDisk1_PolarPentagon) {
        H3Index polar;
        setH3Index(&polar, 1, 4, 0);
        H3Index k2[] = {0, 0, 0, 0, 0, 0, 0};
        int k2Dist[] = {0, 0, 0, 0, 0, 0, 0};
        H3Index expectedK2[] = {0x81083ffffffffff,
                                0x81093ffffffffff,
                                0x81097ffffffffff,
                                0x8108fffffffffff,
                                0x8108bffffffffff,
                                0x8109bffffffffff,
                                0};
        t_assertSuccess(H3_EXPORT(gridDiskDistances)(polar, 1, k2, k2Dist));

        int k2present = 0;
        for (int i = 0; i < 7; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 7; j++) {
                    if (k2[i] == expectedK2[j]) {
                        t_assert(k2Dist[i] == (k2[i] == polar ? 0 : 1),
                                 "distance is as expected");
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 6, "pentagon has 5 neighbors");
    }

    TEST(gridDisk1_PolarPentagon_k3) {
        H3Index polar;
        setH3Index(&polar, 1, 4, 0);
        H3Index k2[37] = {0};
        int k2Dist[37] = {0};
        H3Index expectedK2[37] = {0x81013ffffffffff,
                                  0x811fbffffffffff,
                                  0x81193ffffffffff,
                                  0x81097ffffffffff,
                                  0x81003ffffffffff,
                                  0x81183ffffffffff,
                                  0x8111bffffffffff,
                                  0x81077ffffffffff,
                                  0x811f7ffffffffff,
                                  0x81067ffffffffff,
                                  0x81093ffffffffff,
                                  0x811e7ffffffffff,
                                  0x81083ffffffffff,
                                  0x81117ffffffffff,
                                  0x8101bffffffffff,
                                  0x81107ffffffffff,
                                  0x81073ffffffffff,
                                  0x811f3ffffffffff,
                                  0x81063ffffffffff,
                                  0x8108fffffffffff,
                                  0x811e3ffffffffff,
                                  0x8119bffffffffff,
                                  0x81113ffffffffff,
                                  0x81017ffffffffff,
                                  0x81103ffffffffff,
                                  0x8109bffffffffff,
                                  0x81197ffffffffff,
                                  0x81007ffffffffff,
                                  0x8108bffffffffff,
                                  0x81187ffffffffff,
                                  0x8107bffffffffff,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0};
        int expectedK2Dist[37] = {2, 3, 2, 1, 3, 3, 3, 2, 2, 3, 1, 3, 0,
                                  2, 3, 3, 2, 2, 3, 1, 3, 3, 2, 2, 3, 1,
                                  2, 3, 1, 3, 3, 0, 0, 0, 0, 0, 0};
        t_assertSuccess(H3_EXPORT(gridDiskDistances)(polar, 3, k2, k2Dist));

        int k2present = 0;
        for (int i = 0; i < 37; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 37; j++) {
                    if (k2[i] == expectedK2[j]) {
                        t_assert(k2Dist[i] == expectedK2Dist[j],
                                 "distance is as expected");
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 31, "pentagon has 30 neighbors");
    }

    TEST(gridDisk1_Pentagon_k4) {
        H3Index pent;
        setH3Index(&pent, 1, 14, 0);
        H3Index k2[61] = {0};
        int k2Dist[61] = {0};
        H3Index expectedK2[61] = {0x811d7ffffffffff,
                                  0x810c7ffffffffff,
                                  0x81227ffffffffff,
                                  0x81293ffffffffff,
                                  0x81133ffffffffff,
                                  0x8136bffffffffff,
                                  0x81167ffffffffff,
                                  0x811d3ffffffffff,
                                  0x810c3ffffffffff,
                                  0x81223ffffffffff,
                                  0x81477ffffffffff,
                                  0x8128fffffffffff,
                                  0x81367ffffffffff,
                                  0x8112fffffffffff,
                                  0x811cfffffffffff,
                                  0x8123bffffffffff,
                                  0x810dbffffffffff,
                                  0x8112bffffffffff,
                                  0x81473ffffffffff,
                                  0x8128bffffffffff,
                                  0x81363ffffffffff,
                                  0x811cbffffffffff,
                                  0x81237ffffffffff,
                                  0x810d7ffffffffff,
                                  0x81127ffffffffff,
                                  0x8137bffffffffff,
                                  0x81287ffffffffff,
                                  0x8126bffffffffff,
                                  0x81177ffffffffff,
                                  0x810d3ffffffffff,
                                  0x81233ffffffffff,
                                  0x8150fffffffffff,
                                  0x81123ffffffffff,
                                  0x81377ffffffffff,
                                  0x81283ffffffffff,
                                  0x8102fffffffffff,
                                  0x811c3ffffffffff,
                                  0x810cfffffffffff,
                                  0x8122fffffffffff,
                                  0x8113bffffffffff,
                                  0x81373ffffffffff,
                                  0x8129bffffffffff,
                                  0x8102bffffffffff,
                                  0x811dbffffffffff,
                                  0x810cbffffffffff,
                                  0x8122bffffffffff,
                                  0x81297ffffffffff,
                                  0x81507ffffffffff,
                                  0x8136fffffffffff,
                                  0x8127bffffffffff,
                                  0x81137ffffffffff,
                                  0,
                                  0};
        t_assertSuccess(H3_EXPORT(gridDiskDistances)(pent, 4, k2, k2Dist));

        int k2present = 0;
        for (int i = 0; i < 61; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 61; j++) {
                    if (k2[i] == expectedK2[j]) {
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 51, "pentagon has 50 neighbors");
    }

    TEST(gridDisk_equals_gridDiskDistancesSafe) {
        // Check that gridDiskDistances output matches gridDiskDistancesSafe,
        // since gridDiskDistances will sometimes use a different
        // implementation.

        for (int res = 0; res < 2; res++) {
            iterateAllIndexesAtRes(
                res, gridDisk_equals_gridDiskDistancesSafe_assertions);
        }
    }

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

    TEST(gridDiskInvalid) {
        int k = 1000;
        int64_t kSz;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &kSz));
        H3Index *neighbors = calloc(kSz, sizeof(H3Index));
        t_assert(H3_EXPORT(gridDisk)(0x7fffffffffffffff, k, neighbors) ==
                     E_CELL_INVALID,
                 "gridDisk returns error for invalid input");
        free(neighbors);
    }

    TEST(gridDiskInvalidDigit) {
        int k = 2;
        int64_t kSz;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &kSz));
        H3Index *neighbors = calloc(kSz, sizeof(H3Index));
        t_assert(H3_EXPORT(gridDisk)(0x4d4b00fe5c5c3030, k, neighbors) ==
                     E_CELL_INVALID,
                 "gridDisk returns error for invalid input");
        free(neighbors);
    }

    TEST(gridDiskDistances_invalidK) {
        H3Index index = 0x811d7ffffffffff;
        t_assert(
            H3_EXPORT(gridDiskDistances)(index, -1, NULL, NULL) == E_DOMAIN,
            "gridDiskDistances invalid k");
        t_assert(H3_EXPORT(gridDiskDistancesUnsafe)(index, -1, NULL, NULL) ==
                     E_DOMAIN,
                 "gridDiskDistancesUnsafe invalid k");
        t_assert(
            H3_EXPORT(gridDiskDistancesSafe)(index, -1, NULL, NULL) == E_DOMAIN,
            "gridDiskDistancesSafe invalid k");
    }

    TEST(maxGridDiskSize_invalid) {
        int64_t sz;
        t_assert(H3_EXPORT(maxGridDiskSize)(-1, &sz) == E_DOMAIN,
                 "negative k is invalid");
    }

    TEST(maxGridDiskSize_large) {
        int64_t sz;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(26755, &sz));
        t_assert(sz == 2147570341, "large (> 32 bit signed int) k works");
    }

    TEST(maxGridDiskSize_numCells) {
        int64_t sz;
        int64_t prev = 0;
        int64_t max;
        t_assertSuccess(H3_EXPORT(getNumCells)(15, &max));
        // 13780510 will produce values above max
        for (int k = 13780510 - 100; k < 13780510 + 100; k++) {
            t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &sz));
            t_assert(sz <= max,
                     "maxGridDiskSize does not produce estimates above the "
                     "number of grid cells");
            t_assert(prev <= sz, "maxGridDiskSize is monotonically increasing");
            prev = sz;
        }

        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(INT32_MAX, &sz));
        t_assert(sz == max,
                 "maxGridDiskSize of INT32_MAX produces valid result");
    }
}
