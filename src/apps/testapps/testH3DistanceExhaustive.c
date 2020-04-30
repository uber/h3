/*
 * Copyright 2019 Uber Technologies, Inc.
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
 *  usage: `testH3DistanceExhaustive`
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

static void h3Distance_identity_assertions(H3Index h3) {
    t_assert(H3_EXPORT(h3Distance)(h3, h3) == 0, "distance to self is 0");
}

static void h3Distance_kRing_assertions(H3Index h3) {
    int r = H3_GET_RESOLUTION(h3);
    t_assert(r <= 5, "resolution supported by test function (kRing)");
    int maxK = MAX_DISTANCES[r];

    int sz = H3_EXPORT(maxKringSize)(maxK);
    H3Index *neighbors = calloc(sz, sizeof(H3Index));
    int *distances = calloc(sz, sizeof(int));

    H3_EXPORT(kRingDistances)(h3, maxK, neighbors, distances);

    for (int i = 0; i < sz; i++) {
        if (neighbors[i] == 0) {
            continue;
        }

        int calculatedDistance = H3_EXPORT(h3Distance)(h3, neighbors[i]);

        // Don't consider indexes where h3Distance reports failure to
        // generate
        t_assert(calculatedDistance == distances[i] || calculatedDistance == -1,
                 "kRingDistances matches h3Distance");
    }

    free(distances);
    free(neighbors);
}

SUITE(h3Distance) {
    TEST(h3Distance_identity) {
        iterateAllIndexesAtRes(0, h3Distance_identity_assertions);
        iterateAllIndexesAtRes(1, h3Distance_identity_assertions);
        iterateAllIndexesAtRes(2, h3Distance_identity_assertions);
    }

    TEST(h3Distance_kRing) {
        iterateAllIndexesAtRes(0, h3Distance_kRing_assertions);
        iterateAllIndexesAtRes(1, h3Distance_kRing_assertions);
        iterateAllIndexesAtRes(2, h3Distance_kRing_assertions);
        // Don't iterate all of res 3, to save time
        iterateAllIndexesAtResPartial(3, h3Distance_kRing_assertions, 27);
        // Further resolutions aren't tested to save time.
    }
}
