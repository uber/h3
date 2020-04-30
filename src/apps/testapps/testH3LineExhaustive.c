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
 *  usage: `testH3Distance`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h3Index.h"
#include "h3api.h"
#include "localij.h"
#include "test.h"
#include "utility.h"

static const int MAX_DISTANCES[] = {1, 2, 5, 12, 19, 26};

/**
 * Property-based testing of h3Line output
 */
static void h3Line_assertions(H3Index start, H3Index end) {
    int sz = H3_EXPORT(h3LineSize)(start, end);
    t_assert(sz > 0, "got valid size");
    H3Index *line = calloc(sz, sizeof(H3Index));

    int err = H3_EXPORT(h3Line)(start, end, line);

    t_assert(err == 0, "no error on line");
    t_assert(line[0] == start, "line starts with start index");
    t_assert(line[sz - 1] == end, "line ends with end index");

    for (int i = 1; i < sz; i++) {
        t_assert(H3_EXPORT(h3IsValid)(line[i]), "index is valid");
        t_assert(H3_EXPORT(h3IndexesAreNeighbors)(line[i], line[i - 1]),
                 "index is a neighbor of the previous index");
        if (i > 1) {
            t_assert(
                !H3_EXPORT(h3IndexesAreNeighbors)(line[i], line[i - 2]),
                "index is not a neighbor of the index before the previous");
        }
    }

    free(line);
}

/**
 * Tests for invalid h3Line input
 */
static void h3Line_invalid_assertions(H3Index start, H3Index end) {
    int sz = H3_EXPORT(h3LineSize)(start, end);
    t_assert(sz < 0, "line size marked as invalid");

    H3Index *line = {0};
    int err = H3_EXPORT(h3Line)(start, end, line);
    t_assert(err != 0, "line marked as invalid");
}

/**
 * Test for lines from an index to all neighbors within a kRing
 */
static void h3Line_kRing_assertions(H3Index h3) {
    int r = H3_GET_RESOLUTION(h3);
    t_assert(r <= 5, "resolution supported by test function (kRing)");
    int maxK = MAX_DISTANCES[r];

    int sz = H3_EXPORT(maxKringSize)(maxK);

    if (H3_EXPORT(h3IsPentagon)(h3)) {
        return;
    }

    H3Index *neighbors = calloc(sz, sizeof(H3Index));
    H3_EXPORT(kRing)(h3, maxK, neighbors);

    for (int i = 0; i < sz; i++) {
        if (neighbors[i] == 0) {
            continue;
        }
        int distance = H3_EXPORT(h3Distance)(h3, neighbors[i]);
        if (distance >= 0) {
            h3Line_assertions(h3, neighbors[i]);
        } else {
            h3Line_invalid_assertions(h3, neighbors[i]);
        }
    }

    free(neighbors);
}

SUITE(h3Line) {
    TEST(h3Line_kRing) {
        iterateAllIndexesAtRes(0, h3Line_kRing_assertions);
        iterateAllIndexesAtRes(1, h3Line_kRing_assertions);
        iterateAllIndexesAtRes(2, h3Line_kRing_assertions);
        // Don't iterate all of res 3, to save time
        iterateAllIndexesAtResPartial(3, h3Line_kRing_assertions, 6);
        // Further resolutions aren't tested to save time.
    }
}
