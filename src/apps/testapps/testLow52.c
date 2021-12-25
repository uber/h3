/*
 * Copyright 2020-2021 Uber Technologies, Inc.
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
 * @brief tests H3 cell area functions on a few specific cases
 *
 *  usage: `testH3CellArea`
 */

#include <math.h>

#include "h3Index.h"
#include "test.h"

static const H3Index h = 0x89283082e73ffff;

// add empty input tests

SUITE(low52tests) {
    TEST(basic_low52) {
        int k = 100;
        int N = maxGridDiskSize(k);
        int64_t numAfter;

        // create disk
        H3Index *out = calloc(N, sizeof(H3Index));
        gridDisk(h, k, out);

        // low 52 tests
        t_assert(!isLow52Sorted(out, N), "Shouldn't be sorted yet");
        t_assertSuccess(low52Sort(out, N));
        t_assert(isLow52Sorted(out, N), "Should be sorted now!");

        // canonical tests
        t_assert(isCanonicalCells(out, N),
                 "No duplicates, so should already be canon.");
        t_assertSuccess(canonicalizeCells(out, N, &numAfter));
        t_assert(N == numAfter, "Expect no change");

        // binary search
        t_assert(canonSearch(out, N, h), "Needs to be in there!");
        t_assert(!canonSearch(out, 0, h), "h can't be in empty set.");

        // intersection
        t_assert(intersectTheyDo(out, N, out, N),
                 "Set should intersect itself.");
        t_assert(!intersectTheyDo(out, 0, out, N), "A is empty.");
        t_assert(!intersectTheyDo(out, N, out, 0), "B is empty.");
        t_assert(!intersectTheyDo(out, 0, out, 0), "Both empty.");

        free(out);
    }

    TEST(handling_zeroes) {
        int k = 100;
        int N = maxGridDiskSize(k);
        int64_t numAfter;

        // create disk
        H3Index *out = calloc(N, sizeof(H3Index));
        gridDisk(h, k, out);
        t_assertSuccess(canonicalizeCells(out, N, &numAfter));
        t_assert(N == numAfter, "Expect no change");

        t_assert(isLow52Sorted(out, N), "");
        t_assert(isCanonicalCells(out, N), "");

        // insert zero at start of array
        // isLow52Sorted is OK with zeros/H3_NULL, but isCanonicalCells is not
        out[0] = 0;

        t_assert(isLow52Sorted(out, N), "");
        t_assert(!isCanonicalCells(out, N),
                 "Should not be canon, due to zero.");

        // canonicalizing should remove the zero!
        t_assertSuccess(canonicalizeCells(out, N, &numAfter));
        t_assert(numAfter == N - 1, "Lose one cell.");

        t_assert(isCanonicalCells(out, numAfter), "");

        free(out);
    }
}
