/*
 * Copyright 2019-2020 Uber Technologies, Inc.
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
 * @brief tests H3 distance function using tests over a large number of indexes.
 *
 *  usage: `testGridDistanceExhaustive`
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

static const int MAX_DISTANCES[] = {1, 2, 5, 12, 19, 26};

static void gridDistance_identity_assertions(H3Index h3) {
    int64_t distance;
    t_assertSuccess(H3_EXPORT(gridDistance)(h3, h3, &distance));
    t_assert(distance == 0, "distance to self is 0");
}

static void gridDistance_gridDisk_assertions(H3Index h3) {
    int r = H3_GET_RESOLUTION(h3);
    t_assert(r <= 5, "resolution supported by test function (gridDisk)");
    int maxK = MAX_DISTANCES[r];

    int64_t sz;
    t_assertSuccess(H3_EXPORT(maxGridDiskSize)(maxK, &sz));
    H3Index *neighbors = calloc(sz, sizeof(H3Index));
    int *distances = calloc(sz, sizeof(int));

    t_assertSuccess(
        H3_EXPORT(gridDiskDistances)(h3, maxK, neighbors, distances));

    for (int64_t i = 0; i < sz; i++) {
        if (neighbors[i] == 0) {
            continue;
        }

        int64_t calculatedDistance;
        H3Error calculatedError =
            H3_EXPORT(gridDistance)(h3, neighbors[i], &calculatedDistance);

        // Don't consider indexes where gridDistance reports failure to
        // generate
        if (calculatedError == E_SUCCESS) {
            t_assert(calculatedDistance == distances[i],
                     "gridDiskDistances matches gridDistance");
        }
    }

    free(distances);
    free(neighbors);
}

SUITE(gridDistance) {
    TEST(gridDistance_identity) {
        iterateAllIndexesAtRes(0, gridDistance_identity_assertions);
        iterateAllIndexesAtRes(1, gridDistance_identity_assertions);
        iterateAllIndexesAtRes(2, gridDistance_identity_assertions);
    }

    TEST(gridDistance_gridDisk) {
        iterateAllIndexesAtRes(0, gridDistance_gridDisk_assertions);
        iterateAllIndexesAtRes(1, gridDistance_gridDisk_assertions);
        iterateAllIndexesAtRes(2, gridDistance_gridDisk_assertions);
        // Don't iterate all of res 3, to save time
        iterateAllIndexesAtResPartial(3, gridDistance_gridDisk_assertions, 27);
        // Further resolutions aren't tested to save time.
    }
}
