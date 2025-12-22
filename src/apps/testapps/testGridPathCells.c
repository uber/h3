/*
 * Copyright 2018-2020 Uber Technologies, Inc.
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
 * @brief tests H3 grid path function.
 *
 *  usage: `testGridPathCells`
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h3Index.h"
#include "h3api.h"
#include "localij.h"
#include "test.h"
#include "utility.h"

static void assertPathValid(H3Index start, H3Index end, const H3Index *path,
                            int64_t size) {
    t_assert(size > 0, "path size is positive");
    t_assert(path[0] == start, "path starts with start index");
    t_assert(path[size - 1] == end, "path ends with end index");

    for (int64_t i = 1; i < size; i++) {
        int isNeighbor;
        t_assertSuccess(
            H3_EXPORT(areNeighborCells)(path[i], path[i - 1], &isNeighbor));
        t_assert(isNeighbor, "path is contiguous");
    }
}

SUITE(gridPathCells) {
    TEST(gridPathCells_acrossMultipleFaces) {
        H3Index start = 0x85285aa7fffffff;
        H3Index end = 0x851d9b1bfffffff;

        int64_t size;
        H3Error lineError = H3_EXPORT(gridPathCellsSize)(start, end, &size);
        t_assert(lineError == E_FAILED,
                 "Line not computable across multiple icosa faces");
    }

    TEST(gridPathCells_pentagonReverseInterpolation) {
        H3Index start = 0x820807fffffffff;
        H3Index end = 0x8208e7fffffffff;

        int64_t size;
        t_assertSuccess(H3_EXPORT(gridPathCellsSize)(start, end, &size));
        H3Index *path = calloc(size, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(gridPathCells)(start, end, path));
        assertPathValid(start, end, path, size);

        free(path);
    }

    TEST(gridPathCells_knownFailureNotCoveredByReverseInterpolation) {
        // Known limitation case:
        //
        // There are still pairs where `gridDistance` succeeds but interpolation
        // fails in both origin-anchored local IJK charts (anchored at `start`
        // and anchored at `end`). Since `gridPathCells` only performs these two
        // interpolation attempts, it returns an error.
        //
        // This test keeps a pinned input pair to demonstrate that the current
        // approach is not complete.
        H3Index start = 0x8411b61ffffffff;
        H3Index end = 0x84016d3ffffffff;

        int64_t size;
        t_assertSuccess(H3_EXPORT(gridPathCellsSize)(start, end, &size));
        H3Index *path = calloc(size, sizeof(H3Index));

        H3Error err = H3_EXPORT(gridPathCells)(start, end, path);
        t_assert(err != E_SUCCESS, "Expected gridPathCells to fail");

        free(path);
    }
}
