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

typedef struct {
    const H3Index *cells;
    int64_t N;
} CellArray;

CellArray getDisk(H3Index h, int k) {
    CellArray arr;
    arr.N = maxGridDiskSize(k);
    arr.cells = calloc(arr.N, sizeof(H3Index));
    gridDisk(h, k, arr.cells);

    return arr;
}

// add empty input tests
// uncompact of a canonical set should give you a canonical set

SUITE(low52tests) {
    TEST(basic_low52) {
        H3Index h = 0x89283082e73ffff;
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
        t_assert(intersectTheyDo(out, N, out, N), "");
        t_assert(!intersectTheyDo(out, 0, out, N), "A is empty.");
        t_assert(!intersectTheyDo(out, N, out, 0), "B is empty.");
        t_assert(!intersectTheyDo(out, 0, out, 0), "Both empty.");

        free(out);
    }

    TEST(handling_zeroes) {
        H3Index h = 0x89283082e73ffff;
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

    TEST(compact_low52) {
        H3Index h = 0x89283082e73ffff;
        int res = 9;
        int k = 100;
        int64_t numU = maxGridDiskSize(k);

        H3Index *cellsU = calloc(numU, sizeof(H3Index));
        gridDisk(h, k, cellsU);
        canonicalizeCells(cellsU, numU, &numU);

        int64_t numC = numU;

        H3Index *cellsC = calloc(numC, sizeof(H3Index));

        compactCells(cellsU, cellsC, numU);
        canonicalizeCells(cellsC, numC, &numC);

        t_assert(isCanonicalCells(cellsU, numU), "");
        t_assert(isCanonicalCells(cellsC, numC), "");

        t_assert(canonSearch(cellsC, numC, h), "");
        t_assert(intersectTheyDo(cellsC, numC, cellsC, numC), "");

        // TODO: macro or function here would be clearer?
        // test that uncompact keeps things canonical
        H3Index *newUncompacted = calloc(numU, sizeof(H3Index));
        t_assertSuccess(
            uncompactCells(cellsC, numC, newUncompacted, numU, res));
        t_assert(isCanonicalCells(newUncompacted, numU), "");

        free(cellsU);
        free(cellsC);
        free(newUncompacted);
    }
}
