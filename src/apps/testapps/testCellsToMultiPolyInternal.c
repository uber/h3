/*
 * Copyright 2016-2021 Uber Technologies, Inc.
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
 * This file tests internal helper functions (comparison and cleanup functions)
 * that are defined in cellsToMultiPoly.h. These functions are normally internal
 * to the implementation but are exposed via the header as static inline
 * functions to enable unit testing and ensure proper memory cleanup in all
 * error paths.
 *
 * See cellsToMultiPoly.h for more details on why these functions are in a
 * header file.
 *
 * usage: `testCellsToMultiPolyInternal`
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
}
