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
 * @brief tests "lower 52 bit" ordering, canonicalization, and spatial join
 * algorithms
 *
 *  usage: `testLow52`
 */

#include <math.h>

#include "h3Index.h"
#include "test.h"

typedef struct {
    H3Index *cells;
    int64_t N;
} CellArray;

CellArray initCellArray(int64_t N) {
    CellArray a = {.cells = calloc(N, sizeof(H3Index)), .N = N};

    return a;
}

CellArray getDisk(H3Index h, int k) {
    CellArray arr = initCellArray(maxGridDiskSize(k));
    gridDisk(h, k, arr.cells);

    return arr;
}

CellArray getRing(H3Index h, int k) {
    CellArray A = initCellArray(6 * k);
    gridRingUnsafe(h, k, A.cells);

    return A;
}

void doCanon(CellArray *arr) {
    int64_t N;
    canonicalizeCells(arr->cells, arr->N, &N);
    arr->N = N;
}

CellArray doCompact(CellArray arr) {
    CellArray packed = initCellArray(arr.N);
    compactCells(arr.cells, packed.cells, arr.N);

    return packed;
}

CellArray doUncompact(CellArray arr, int res) {
    int64_t N;
    uncompactCellsSize(arr.cells, arr.N, res, &N);

    CellArray out = initCellArray(N);
    uncompactCells(arr.cells, arr.N, out.cells, out.N, res);

    return out;
}

bool doIntersect(CellArray A, CellArray B) {
    return intersectTheyDo(A.cells, A.N, B.cells, B.N);
}

bool isLow52(CellArray A) { return isLow52Sorted(A.cells, A.N); }
bool isCanon(CellArray A) { return isCanonicalCells(A.cells, A.N); }

// add empty input tests
// uncompact of a canonical set should give you a canonical set

SUITE(low52tests) {
    TEST(basic_low52) {
        H3Index h = 0x89283082e73ffff;
        int k = 100;

        CellArray A = getDisk(h, k);

        // low 52 tests
        t_assert(!isLow52(A), "Shouldn't be sorted yet");
        t_assertSuccess(low52Sort(A.cells, A.N));
        t_assert(isLow52(A), "Should be sorted now!");

        // canonical tests
        t_assert(isCanon(A), "No duplicates, so should already be canon.");

        int64_t numBefore = A.N;
        doCanon(&A);
        t_assert(A.N == numBefore, "Expect no change from canonicalizing.");

        // binary search
        t_assert(canonSearch(A.cells, A.N, h), "Needs to be in there!");
        t_assert(!canonSearch(A.cells, 0, h), "h can't be in empty set.");

        // intersection
        CellArray Z = {.N = 0, .cells = NULL};  // empty cell array
        t_assert(doIntersect(A, A), "");
        t_assert(!doIntersect(Z, A), "First is empty.");
        t_assert(!doIntersect(A, Z), "Second is empty.");
        t_assert(!doIntersect(Z, Z), "Both are empty.");

        free(A.cells);
    }

    TEST(handling_zeroes) {
        H3Index h = 0x89283082e73ffff;
        int k = 100;

        CellArray A = getDisk(h, k);

        int64_t numBefore = A.N;
        doCanon(&A);
        t_assert(A.N == numBefore, "Expect no change from canonicalizing.");

        t_assert(isLow52(A), "");
        t_assert(isCanon(A), "");

        // insert zero at start of array
        // isLow52Sorted is OK with zeros/H3_NULL, but isCanonicalCells is not
        A.cells[0] = 0;
        t_assert(isLow52(A), "");
        t_assert(!isCanon(A), "Should not be canon, due to zero.");

        // canonicalizing again should remove the zero
        doCanon(&A);
        t_assert(A.N == numBefore - 1, "Lose one cell.");
        t_assert(isCanon(A), "");

        free(A.cells);
    }

    TEST(compact_canon) {
        H3Index h = 0x89283082e73ffff;
        int res = 9;
        int k = 100;

        CellArray u = getDisk(h, k);  // uncompacted set
        doCanon(&u);

        CellArray c = doCompact(u);  // compacted set
        doCanon(&c);

        t_assert(isCanon(u), "");
        t_assert(isCanon(c), "");

        t_assert(canonSearch(c.cells, c.N, h), "");
        t_assert(doIntersect(c, c), "");
        t_assert(doIntersect(c, u), "");
        t_assert(doIntersect(u, c), "");

        // test that uncompact keeps things canonical
        CellArray u2 = doUncompact(c, res);
        t_assert(isCanon(u2), "");

        free(c.cells);
        free(u.cells);
        free(u2.cells);
    }

    TEST(ring_intersect) {
        H3Index h = 0x89283082e73ffff;
        int k = 10;

        CellArray A = getRing(h, k);
        CellArray B = getRing(h, k + 1);
        doCanon(&A);
        doCanon(&B);

        t_assert(!canonSearch(A.cells, A.N, h), "");
        t_assert(!canonSearch(B.cells, B.N, h), "");

        t_assert(canonSearch(A.cells, A.N, A.cells[0]), "");

        t_assert(!doIntersect(A, B), "");
        t_assert(!doIntersect(B, A), "");

        t_assert(doIntersect(A, A), "");
        t_assert(doIntersect(B, B), "");

        // add a cell from A to B, so they now intersect
        B.cells[B.N / 2] = A.cells[A.N / 2];
        doCanon(&B);
        t_assert(doIntersect(A, B), "");
        t_assert(doIntersect(B, A), "");

        free(A.cells);
        free(B.cells);
    }

    TEST(overlap_test_not_compated) {
        H3Index a = 0x89283082e73ffff;
        H3Index b = 0x89283095063ffff;
        CellArray A, B;

        int64_t k;
        t_assertSuccess(gridDistance(a, b, &k));
        t_assert(k == 20, "");

        // not yet
        A = getDisk(a, 9);
        B = getDisk(b, 9);
        doCanon(&A);
        doCanon(&B);

        t_assert(!doIntersect(A, B), "");
        t_assert(!doIntersect(B, A), "");

        free(A.cells);
        free(B.cells);

        // overlap
        A = getDisk(a, 10);
        B = getDisk(b, 10);
        doCanon(&A);
        doCanon(&B);

        t_assert(doIntersect(A, B), "");
        t_assert(doIntersect(B, A), "");

        free(A.cells);
        free(B.cells);

        // more overlap
        A = getDisk(a, 11);
        B = getDisk(b, 11);
        doCanon(&A);
        doCanon(&B);

        t_assert(doIntersect(A, B), "");
        t_assert(doIntersect(B, A), "");

        free(A.cells);
        free(B.cells);

        // just barely disjoint
        A = getDisk(a, 9);
        B = getDisk(b, 10);
        doCanon(&A);
        doCanon(&B);

        t_assert(!doIntersect(A, B), "");
        t_assert(!doIntersect(B, A), "");

        free(A.cells);
        free(B.cells);
    }

    TEST(overlap_test_compated) {
        H3Index a = 0x89283082e73ffff;
        H3Index b = 0x89283095063ffff;
        CellArray A, B, cA, cB;

        int64_t k;
        t_assertSuccess(gridDistance(a, b, &k));
        t_assert(k == 20, "");

        // not yet
        A = getDisk(a, 9);
        B = getDisk(b, 9);
        cA = doCompact(A);
        cB = doCompact(B);
        doCanon(&cA);
        doCanon(&cB);

        t_assert(!doIntersect(cA, cB), "");
        t_assert(!doIntersect(cB, cA), "");

        free(A.cells);
        free(B.cells);
        free(cA.cells);
        free(cB.cells);

        // overlap
        A = getDisk(a, 10);
        B = getDisk(b, 10);
        cA = doCompact(A);
        cB = doCompact(B);
        doCanon(&cA);
        doCanon(&cB);

        t_assert(doIntersect(cA, cB), "");
        t_assert(doIntersect(cB, cA), "");

        free(A.cells);
        free(B.cells);
        free(cA.cells);
        free(cB.cells);

        // more overlap
        A = getDisk(a, 11);
        B = getDisk(b, 11);
        cA = doCompact(A);
        cB = doCompact(B);
        doCanon(&cA);
        doCanon(&cB);

        t_assert(doIntersect(cA, cB), "");
        t_assert(doIntersect(cB, cA), "");

        free(A.cells);
        free(B.cells);
        free(cA.cells);
        free(cB.cells);

        // just barely disjoint
        A = getDisk(a, 9);
        B = getDisk(b, 10);
        cA = doCompact(A);
        cB = doCompact(B);
        doCanon(&cA);
        doCanon(&cB);

        t_assert(!doIntersect(cA, cB), "");
        t_assert(!doIntersect(cB, cA), "");

        free(A.cells);
        free(B.cells);
        free(cA.cells);
        free(cB.cells);
    }
}
