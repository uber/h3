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

#include "h3api.h"
#include "test.h"

typedef struct {
    H3Index *cells;
    int64_t N;
} CellArray;

CellArray ca_init(int64_t N) {
    CellArray a = {.cells = calloc(N, sizeof(H3Index)), .N = N};

    return a;
}

CellArray ca_disk(H3Index h, int k) {
    CellArray arr = ca_init(H3_EXPORT(maxGridDiskSize)(k));
    H3_EXPORT(gridDisk)(h, k, arr.cells);

    return arr;
}

CellArray ca_ring(H3Index h, int k) {
    CellArray A = ca_init(6 * k);
    H3_EXPORT(gridRingUnsafe)(h, k, A.cells);

    return A;
}

void ca_canon(CellArray *arr) {
    int64_t N;
    canonicalizeCells(arr->cells, arr->N, &N);
    arr->N = N;
}

CellArray ca_compact(CellArray arr) {
    CellArray packed = ca_init(arr.N);
    H3_EXPORT(compactCells)(arr.cells, packed.cells, arr.N);

    return packed;
}

CellArray ca_uncompact(CellArray arr, int res) {
    int64_t N;
    H3_EXPORT(uncompactCellsSize)(arr.cells, arr.N, res, &N);

    CellArray out = ca_init(N);
    H3_EXPORT(uncompactCells)(arr.cells, arr.N, out.cells, out.N, res);

    return out;
}

void t_intersect(CellArray A, CellArray B, bool result) {
    t_assert(result == intersectTheyDo(A.cells, A.N, B.cells, B.N), "");
}

void t_contains(CellArray A, H3Index h, bool result) {
    t_assert(result == canonSearch(A.cells, A.N, h), "");
}

void t_isLow52(CellArray A, bool result) {
    t_assert(result == isLow52Sorted(A.cells, A.N), "");
}
void t_isCanon(CellArray A, bool result) {
    t_assert(result == isCanonicalCells(A.cells, A.N), "");
}

void t_diskIntersect(H3Index a, H3Index b, int ka, int kb,
                     bool shouldIntersect) {
    CellArray A = ca_disk(a, ka);
    CellArray B = ca_disk(b, kb);

    ca_canon(&A);
    ca_canon(&B);

    t_intersect(A, B, shouldIntersect);
    t_intersect(B, A, shouldIntersect);

    free(A.cells);
    free(B.cells);
}

void t_diskIntersectCompact(H3Index a, H3Index b, int ka, int kb,
                            bool shouldIntersect) {
    CellArray A, B, cA, cB;

    A = ca_disk(a, ka);
    B = ca_disk(b, kb);

    cA = ca_compact(A);
    cB = ca_compact(B);
    ca_canon(&cA);
    ca_canon(&cB);

    t_intersect(cA, cB, shouldIntersect);
    t_intersect(cB, cA, shouldIntersect);

    free(A.cells);
    free(B.cells);
    free(cA.cells);
    free(cB.cells);
}

SUITE(low52tests) {
    TEST(basic_low52) {
        H3Index h = 0x89283082e73ffff;
        int k = 100;

        CellArray A = ca_disk(h, k);
        CellArray Z = {.N = 0, .cells = NULL};  // empty cell array

        // low 52 tests
        t_isLow52(A, false);  // shouldn't be sorted yet
        t_assertSuccess(low52Sort(A.cells, A.N));
        t_isLow52(A, true);  // should be sorted now!

        // canonical tests
        t_isCanon(A, true);  // no duplicates, so should already be canon
        int64_t numBefore = A.N;
        ca_canon(&A);
        t_assert(A.N == numBefore, "Expect no change from canonicalizing.");

        // binary search
        t_contains(A, h, true);   // Needs to be in there!
        t_contains(Z, h, false);  // h can't be in an empty set

        // intersection
        t_intersect(A, A, true);
        t_intersect(Z, A, false);  // first is empty
        t_intersect(A, Z, false);  // second is empty
        t_intersect(Z, Z, false);  // both are empty

        free(A.cells);
    }

    TEST(handling_zeroes) {
        H3Index h = 0x89283082e73ffff;
        int k = 100;

        CellArray A = ca_disk(h, k);

        int64_t numBefore = A.N;
        ca_canon(&A);
        t_assert(A.N == numBefore, "Expect no change from canonicalizing.");

        t_isLow52(A, true);
        t_isCanon(A, true);

        // insert zero at start of array
        // isLow52Sorted is OK with zeros/H3_NULL, but isCanonicalCells is not
        A.cells[0] = 0;
        t_isLow52(A, true);
        t_isCanon(A, false);

        // canonicalizing again should remove the zero
        ca_canon(&A);
        t_assert(A.N == numBefore - 1, "Lose one cell.");
        t_isCanon(A, true);

        free(A.cells);
    }

    TEST(compact_canon) {
        H3Index h = 0x89283082e73ffff;
        int res = 9;
        int k = 100;

        CellArray u = ca_disk(h, k);  // uncompacted set
        ca_canon(&u);

        CellArray c = ca_compact(u);  // compacted set
        ca_canon(&c);

        t_isCanon(u, true);
        t_isCanon(c, true);

        t_contains(c, h, true);
        t_intersect(c, c, true);
        t_intersect(c, u, true);
        t_intersect(u, c, true);

        // test that uncompact keeps things canonical
        CellArray u2 = ca_uncompact(c, res);
        t_isCanon(u2, true);

        free(c.cells);
        free(u.cells);
        free(u2.cells);
    }

    TEST(ring_intersect) {
        H3Index h = 0x89283082e73ffff;
        int k = 10;

        CellArray A = ca_ring(h, k);
        CellArray B = ca_ring(h, k + 1);
        ca_canon(&A);
        ca_canon(&B);

        t_contains(A, h, false);
        t_contains(B, h, false);
        t_contains(A, A.cells[0], true);

        t_intersect(A, B, false);
        t_intersect(B, A, false);
        t_intersect(A, A, true);
        t_intersect(B, B, true);

        // add a cell from A to B, so they now intersect
        B.cells[B.N / 2] = A.cells[A.N / 2];
        ca_canon(&B);
        t_intersect(A, B, true);
        t_intersect(A, B, true);

        free(A.cells);
        free(B.cells);
    }

    TEST(disk_overlap) {
        H3Index a = 0x89283082e73ffff;
        H3Index b = 0x89283095063ffff;

        int64_t k;
        t_assertSuccess(H3_EXPORT(gridDistance)(a, b, &k));
        t_assert(k == 20, "");

        // not compacted
        t_diskIntersect(a, b, 9, 9, false);   // not yet
        t_diskIntersect(a, b, 9, 10, false);  // just barely disjoint
        t_diskIntersect(a, b, 10, 10, true);  // overlap
        t_diskIntersect(a, b, 11, 11, true);  // more overlap

        // compacted
        t_diskIntersectCompact(a, b, 9, 9, false);   // not yet
        t_diskIntersectCompact(a, b, 9, 10, false);  // just barely disjoint
        t_diskIntersectCompact(a, b, 10, 10, true);  // overlap
        t_diskIntersectCompact(a, b, 11, 11, true);  // more overlap
    }
}
