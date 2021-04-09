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
 * @brief tests grid path cells function using tests over a large number of
 * indexes.
 *
 *  usage: `testGridPathExhaustive`
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
 * Property-based testing of gridPath output
 */
static void gridPath_assertions(H3Index start, H3Index end) {
    int sz = H3_EXPORT(gridPathSize)(start, end);
    t_assert(sz > 0, "got valid size");
    H3Index *line = calloc(sz, sizeof(H3Index));

    int err = H3_EXPORT(gridPath)(start, end, line);

    t_assert(err == 0, "no error on line");
    t_assert(line[0] == start, "line starts with start index");
    t_assert(line[sz - 1] == end, "line ends with end index");

    for (int i = 1; i < sz; i++) {
        t_assert(H3_EXPORT(isValidCell)(line[i]), "index is valid");
        t_assert(H3_EXPORT(areNeighborCells)(line[i], line[i - 1]),
                 "index is a neighbor of the previous index");
        if (i > 1) {
            t_assert(
                !H3_EXPORT(areNeighborCells)(line[i], line[i - 2]),
                "index is not a neighbor of the index before the previous");
        }
    }

    free(line);
}

/**
 * Tests for invalid gridPath input
 */
static void gridPath_invalid_assertions(H3Index start, H3Index end) {
    int sz = H3_EXPORT(gridPathSize)(start, end);
    t_assert(sz < 0, "line size marked as invalid");

    H3Index *line = {0};
    int err = H3_EXPORT(gridPath)(start, end, line);
    t_assert(err != 0, "line marked as invalid");
}

/**
 * Test for lines from an index to all neighbors within a gridDisk
 */
static void gridPath_gridDisk_assertions(H3Index h3) {
    int r = H3_GET_RESOLUTION(h3);
    t_assert(r <= 5, "resolution supported by test function (gridDisk)");
    int maxK = MAX_DISTANCES[r];

    int sz = H3_EXPORT(maxGridDiskSize)(maxK);

    if (H3_EXPORT(isPentagon)(h3)) {
        return;
    }

    H3Index *neighbors = calloc(sz, sizeof(H3Index));
    H3_EXPORT(gridDisk)(h3, maxK, neighbors);

    for (int i = 0; i < sz; i++) {
        if (neighbors[i] == 0) {
            continue;
        }
        int distance = H3_EXPORT(gridDistance)(h3, neighbors[i]);
        if (distance >= 0) {
            gridPath_assertions(h3, neighbors[i]);
        } else {
            gridPath_invalid_assertions(h3, neighbors[i]);
        }
    }

    free(neighbors);
}

SUITE(gridPath) {
    TEST(gridPath_gridDisk) {
        iterateAllIndexesAtRes(0, gridPath_gridDisk_assertions);
        iterateAllIndexesAtRes(1, gridPath_gridDisk_assertions);
        iterateAllIndexesAtRes(2, gridPath_gridDisk_assertions);
        // Don't iterate all of res 3, to save time
        iterateAllIndexesAtResPartial(3, gridPath_gridDisk_assertions, 6);
        // Further resolutions aren't tested to save time.
    }
}
