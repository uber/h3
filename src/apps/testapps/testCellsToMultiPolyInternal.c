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

#include "cellsToMultiPoly.h"
#include "h3api.h"
#include "test.h"

SUITE(cellsToMultiPolyInternal) {
    TEST(destroyArcSet_with_arcs) {
        // Test that destroyArcSet frees memory and sets pointers to NULL
        ArcSet arcset;
        arcset.numArcs = 10;
        arcset.numBuckets = 100;
        arcset.arcs = malloc(arcset.numArcs * sizeof(Arc));
        arcset.buckets = calloc(arcset.numBuckets, sizeof(Arc *));

        t_assert(arcset.arcs != NULL, "arcs should be allocated");
        t_assert(arcset.buckets != NULL, "buckets should be allocated");

        destroyArcSet(&arcset);

        t_assert(arcset.arcs == NULL, "arcs should be NULL after destroy");
        t_assert(arcset.buckets == NULL,
                 "buckets should be NULL after destroy");

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

    TEST(checkCellsToMultiPolyOverflow_safe) {
        int hashMultiplier = HASH_TABLE_MULTIPLIER;

        // Test with reasonable number of cells (should succeed)
        H3Error err = checkCellsToMultiPolyOverflow(1000000, hashMultiplier);
        t_assert(err == E_SUCCESS, "Should succeed for reasonable numCells");

        // Test with zero cells (should succeed)
        err = checkCellsToMultiPolyOverflow(0, hashMultiplier);
        t_assert(err == E_SUCCESS, "Should succeed for zero cells");

        // Test with negative cells (should succeed - validated elsewhere)
        err = checkCellsToMultiPolyOverflow(-1, hashMultiplier);
        t_assert(err == E_SUCCESS,
                 "Should succeed for negative (check doesn't apply)");

        // Test with small and large hash multipliers.
        // Largest allocated array will change, depending on multiplier.
        t_assertSuccess(checkCellsToMultiPolyOverflow(1000000, 1));
        t_assertSuccess(checkCellsToMultiPolyOverflow(1000000, 100));
    }

    TEST(checkCellsToMultiPolyOverflow_wouldOverflow) {
        int hashMultiplier = HASH_TABLE_MULTIPLIER;

        // Test with numCells that would cause overflow
        // Calculate: INT64_MAX / maxBytesPerCell + 1
        int64_t maxBytesPerCell =
            6 * HASH_TABLE_MULTIPLIER * sizeof(Arc *);  // Currently 480
        int64_t maxSafeNumCells = INT64_MAX / maxBytesPerCell;
        int64_t overflowNumCells = maxSafeNumCells + 1;

        H3Error err =
            checkCellsToMultiPolyOverflow(overflowNumCells, hashMultiplier);
        t_assert(err == E_MEMORY_BOUNDS,
                 "Should return E_MEMORY_BOUNDS when overflow would occur");

        // Also test INT64_MAX directly
        err = checkCellsToMultiPolyOverflow(INT64_MAX, hashMultiplier);
        t_assert(err == E_MEMORY_BOUNDS,
                 "Should return E_MEMORY_BOUNDS for INT64_MAX");
    }
}
