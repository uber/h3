/*
 * Copyright 2026 Uber Technologies, Inc.
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
 * @brief tests internal helper functions in cellsToMultiPoly
 *
 * This file tests internal helper functions from cellsToMultiPoly.h,
 * primarily oriented towards getting complete line and branch coverage.
 *
 * usage: `testCellsToMultiPolyInternal`
 *
 */

#include <stdlib.h>
#include <string.h>

#include "cellsToMultiPoly.h"
#include "h3api.h"
#include "test.h"

SUITE(cellsToMultiPolyInternal) {
    TEST(destroyArcSet_with_arcs) {
        // Test that destroyArcSet frees memory and sets pointers to NULL
        ArcSet arcset;
        arcset.numArcs = 10;
        arcset.arcs = malloc(arcset.numArcs * sizeof(Arc));

        t_assert(arcset.arcs != NULL, "arcs should be allocated");

        destroyArcSet(&arcset);

        t_assert(arcset.arcs == NULL, "arcs should be NULL after destroy");

        // Call again on NULL pointers (should be safe)
        destroyArcSet(&arcset);
    }

    TEST(destroySortableLoopSet_with_verts) {
        // Test with allocated loops and verts
        SortableLoopSet loopset;
        loopset.numLoops = 3;
        loopset.sloops = malloc(3 * sizeof(SortableLoop));

        // First loop has verts (exercises positive branch)
        loopset.sloops[0].loop.numVerts = 5;
        loopset.sloops[0].loop.verts = malloc(5 * sizeof(LatLng));

        // Second loop has NULL verts (exercises negative branch)
        loopset.sloops[1].loop.numVerts = 0;
        loopset.sloops[1].loop.verts = NULL;

        // Third loop has verts (exercises positive branch)
        loopset.sloops[2].loop.numVerts = 4;
        loopset.sloops[2].loop.verts = malloc(4 * sizeof(LatLng));

        destroySortableLoopSet(&loopset);

        t_assert(loopset.sloops == NULL, "sloops should be NULL after destroy");
    }

    TEST(destroySortableLoopSet_null) {
        // Test with NULL sloops (exercises negative branch of outer if)
        SortableLoopSet loopset;
        loopset.numLoops = 0;
        loopset.sloops = NULL;

        destroySortableLoopSet(&loopset);

        t_assert(loopset.sloops == NULL, "sloops should remain NULL");
    }

    TEST(destroySortablePolys_with_holes) {
        // Test with allocated polygons and holes
        SortablePoly *spolys = malloc(2 * sizeof(SortablePoly));

        // First polygon has holes (exercises positive branch)
        spolys[0].poly.numHoles = 2;
        spolys[0].poly.holes = malloc(2 * sizeof(GeoLoop));

        // Second polygon has NULL holes (exercises negative branch)
        spolys[1].poly.numHoles = 0;
        spolys[1].poly.holes = NULL;

        destroySortablePolys(spolys, 2);
        // spolys is freed, can't assert on it
    }

    TEST(destroySortablePolys_null) {
        // Test with NULL spolys (exercises negative branch of outer if)
        destroySortablePolys(NULL, 0);
        // Should not crash
    }

    TEST(destroySortablePolyVerts_with_verts) {
        // Test with allocated polygons and outer loop verts
        SortablePoly *spolys = malloc(2 * sizeof(SortablePoly));

        // First polygon has verts (exercises positive branch)
        spolys[0].poly.geoloop.numVerts = 6;
        spolys[0].poly.geoloop.verts = malloc(6 * sizeof(LatLng));

        // Second polygon has NULL verts (exercises negative branch)
        spolys[1].poly.geoloop.numVerts = 0;
        spolys[1].poly.geoloop.verts = NULL;

        destroySortablePolyVerts(spolys, 2);
        // spolys is freed, can't assert on it
    }

    TEST(destroySortablePolyVerts_null) {
        // Test with NULL spolys (exercises negative branch of outer if)
        destroySortablePolyVerts(NULL, 0);
        // Should not crash
    }

    TEST(cmp_SortablePoly_equal) {
        // Test equality branch of cmp_SortablePoly
        SortablePoly a, b;
        a.outerArea = 100.0;
        b.outerArea = 100.0;

        int result = cmp_SortablePoly(&a, &b);
        t_assert(result == 0, "Equal areas should return 0");
    }

    TEST(cmp_SortablePoly_descending) {
        // Test descending order (larger area comes first)
        SortablePoly a, b;

        // a has larger area, should come first (return -1)
        a.outerArea = 200.0;
        b.outerArea = 100.0;
        int result = cmp_SortablePoly(&a, &b);
        t_assert(result == -1, "Larger area should come first");

        // b has larger area, should come first (return 1)
        a.outerArea = 100.0;
        b.outerArea = 200.0;
        result = cmp_SortablePoly(&a, &b);
        t_assert(result == 1, "Smaller area should come after");
    }

    TEST(cmp_ArcKey) {
        ArcKey a = {.lo = 1, .hi = 5, .arc = NULL};
        ArcKey b = {.lo = 1, .hi = 5, .arc = NULL};
        t_assert(cmp_ArcKey(&a, &b) == 0, "Same cell pair is equal");

        b.hi = 6;
        t_assert(cmp_ArcKey(&a, &b) == -1, "Smaller hi comes first");
        t_assert(cmp_ArcKey(&b, &a) == 1, "Larger hi comes after");

        b.lo = 0;
        t_assert(cmp_ArcKey(&b, &a) == -1, "Smaller lo comes first");
        t_assert(cmp_ArcKey(&a, &b) == 1, "Larger lo comes after");
    }

    TEST(bitRange) {
        int low, high;

        bitRange(0, &low, &high);
        t_assert(low == 64 && high == -1, "No bits set");

        bitRange(1, &low, &high);
        t_assert(low == 0 && high == 0, "Lowest bit only");

        bitRange(0x8000000000000000ULL, &low, &high);
        t_assert(low == 63 && high == 63, "Highest bit only");

        bitRange(0x00f0000000000100ULL, &low, &high);
        t_assert(low == 8 && high == 55, "Bits in the middle");
    }

    TEST(sortArcKeys_random) {
        // Compare against qsort on keys with varying bits across both words
        int64_t n = 5000;
        ArcKey *keys = calloc(n, sizeof(ArcKey));
        ArcKey *scratch = calloc(n, sizeof(ArcKey));
        ArcKey *expected = calloc(n, sizeof(ArcKey));
        int64_t *counts = calloc(ARC_KEY_RADIX_SIZE, sizeof(int64_t));

        uint64_t state = 12345;
        for (int64_t i = 0; i < n; i++) {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            keys[i].lo = 0x0890000000000000ULL | ((state >> 16) % 300) << 20;
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            keys[i].hi = state >> 1;
        }
        memcpy(expected, keys, n * sizeof(ArcKey));
        qsort(expected, n, sizeof(ArcKey), cmp_ArcKey);

        ArcKey *sorted = sortArcKeys(keys, scratch, n, counts);
        t_assert(sorted == keys || sorted == scratch,
                 "Result is in one of the buffers");
        for (int64_t i = 0; i < n; i++) {
            t_assert(sorted[i].lo == expected[i].lo &&
                         sorted[i].hi == expected[i].hi,
                     "Same order as qsort");
        }

        free(counts);
        free(expected);
        free(scratch);
        free(keys);
    }

    TEST(sortArcKeys_constantWord) {
        // The lo word never varies, so only hi is sorted
        int64_t n = 2 * ARC_KEY_RADIX_MIN_KEYS;
        ArcKey *keys = calloc(n, sizeof(ArcKey));
        ArcKey *scratch = calloc(n, sizeof(ArcKey));
        int64_t *counts = calloc(ARC_KEY_RADIX_SIZE, sizeof(int64_t));

        for (int64_t i = 0; i < n; i++) {
            keys[i].lo = 7;
            keys[i].hi = (uint64_t)(n - i) << 40;
        }
        ArcKey *sorted = sortArcKeys(keys, scratch, n, counts);
        for (int64_t i = 0; i < n; i++) {
            t_assert(sorted[i].lo == 7, "lo unchanged");
            t_assert(sorted[i].hi == (uint64_t)(i + 1) << 40, "Sorted by hi");
        }

        // Identical keys need no passes
        for (int64_t i = 0; i < n; i++) {
            keys[i].lo = 1;
            keys[i].hi = 2;
        }
        t_assert(sortArcKeys(keys, scratch, n, counts) == keys,
                 "Identical keys stay in place");

        free(counts);
        free(scratch);
        free(keys);
    }

    TEST(sortArcKeys_small) {
        // Few keys are sorted in place with qsort
        ArcKey keys[] = {
            {.lo = 2, .hi = 3}, {.lo = 1, .hi = 9}, {.lo = 1, .hi = 4}};
        ArcKey scratch[3];
        int64_t counts[1];

        t_assert(sortArcKeys(keys, scratch, 3, counts) == keys,
                 "Sorted in place");
        t_assert(keys[0].lo == 1 && keys[0].hi == 4, "First key");
        t_assert(keys[1].lo == 1 && keys[1].hi == 9, "Second key");
        t_assert(keys[2].lo == 2 && keys[2].hi == 3, "Third key");

        sortArcKeys(keys, scratch, 0, counts);
    }

    TEST(checkCellsToMultiPolyOverflow_safe) {
        // Test with reasonable number of cells (should succeed)
        H3Error err = checkCellsToMultiPolyOverflow(1000000);
        t_assert(err == E_SUCCESS, "Should succeed for reasonable numCells");

        // Test with zero cells (should succeed)
        err = checkCellsToMultiPolyOverflow(0);
        t_assert(err == E_SUCCESS, "Should succeed for zero cells");

        // Test with negative cells (should succeed - validated elsewhere)
        err = checkCellsToMultiPolyOverflow(-1);
        t_assert(err == E_SUCCESS,
                 "Should succeed for negative (check doesn't apply)");
    }

    TEST(checkCellsToMultiPolyOverflow_wouldOverflow) {
        // Test with numCells that would cause size_t overflow
        size_t maxBytesPerCell = 6 * (sizeof(Arc) + 2 * sizeof(ArcKey));
        size_t maxSafeNumCells = SIZE_MAX / maxBytesPerCell;
        size_t overflowNumCells = maxSafeNumCells + 1;

        t_assertSuccess(checkCellsToMultiPolyOverflow(maxSafeNumCells));

        H3Error err = checkCellsToMultiPolyOverflow(overflowNumCells);
        t_assert(err == E_MEMORY_BOUNDS,
                 "Should return E_MEMORY_BOUNDS when overflow would occur");

        // Also test INT64_MAX directly
        err = checkCellsToMultiPolyOverflow(INT64_MAX);
        t_assert(err == E_MEMORY_BOUNDS,
                 "Should return E_MEMORY_BOUNDS for INT64_MAX");
    }
}
