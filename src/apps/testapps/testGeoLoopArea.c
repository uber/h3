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
#include "utility.h"

// GeoMultiPolygon alloc_global_mpoly() {
//     const int numPolygons = 8;
//     const int numVerts = 3;
//     const LatLng verts[numPolygons][numVerts] = {
//         {{M_PI_2, 0.0}, {0.0, 0.0}, {0.0, M_PI_2}},
//         {{M_PI_2, 0.0}, {0.0, M_PI_2}, {0.0, M_PI}},
//         {{M_PI_2, 0.0}, {0.0, M_PI}, {0.0, -M_PI_2}},
//         {{M_PI_2, 0.0}, {0.0, -M_PI_2}, {0.0, 0.0}},
//         {{-M_PI_2, 0.0}, {0.0, 0.0}, {0.0, -M_PI_2}},
//         {{-M_PI_2, 0.0}, {0.0, -M_PI_2}, {0.0, -M_PI}},
//         {{-M_PI_2, 0.0}, {0.0, -M_PI}, {0.0, M_PI_2}},
//         {{-M_PI_2, 0.0}, {0.0, M_PI_2}, {0.0, 0.0}},
//     };

//     GeoMultiPolygon mpoly = {
//         .numPolygons = numPolygons,
//         .polygons = H3_MEMORY(malloc)(sizeof(GeoPolygon) * numPolygons),
//     };

//     for (int i = 0; i < numPolygons; i++) {
//         GeoPolygon *poly = &mpoly.polygons[i];
//         poly->numHoles = 0;
//         poly->holes = NULL;
//         poly->geoloop.numVerts = numVerts;
//         poly->geoloop.verts = H3_MEMORY(malloc)(sizeof(LatLng) * numVerts);

//         for (int j = 0; j < numVerts; j++) {
//             poly->geoloop.verts[j] = verts[i][j];
//         }
//     }

//     qsort(mpoly.polygons, numPolygons, sizeof(GeoPolygon), cmp_poly_area);

//     return mpoly;
// }

#define TOL 1e-14

static void helper(LatLng *verts, int numVerts, double target_area) {
    GeoLoop loop = {.verts = verts, .numVerts = numVerts};

    double out;
    t_assertSuccess(H3_EXPORT(geoLoopArea)(loop, &out));

    t_assert(fabs(out - target_area) < TOL, "area should match");
}

SUITE(geoLoopArea) {
    TEST(triangle) {
        // GeoLoop representing a triangle covering 1/8 of the globe, with
        // points ordered according to right-hand rule (counter-clockwise).
        // Expected area: pi/2
        LatLng verts[] = {{M_PI_2, 0.0}, {0.0, 0.0}, {0.0, M_PI_2}};
        GeoLoop loop = {.verts = verts, .numVerts = ARRAY_SIZE(verts)};

        double out;
        t_assertSuccess(H3_EXPORT(geoLoopArea)(loop, &out));

        t_assert(fabs(out - M_PI / 2.0) < 1e-14, "area should match");
    }
    TEST(reverse_triangle) {
        // Reversed the order of the points, so they are in clockwise order.
        // This GeoLoop represents the whole globe minus the triangle above.
        // Expected area: 7*pi/2
        LatLng verts[] = {{0.0, M_PI_2}, {0.0, 0.0}, {M_PI_2, 0.0}};
        GeoLoop loop = {.verts = verts, .numVerts = ARRAY_SIZE(verts)};

        double out;
        t_assertSuccess(H3_EXPORT(geoLoopArea)(loop, &out));

        t_assert(fabs(out - 7 * M_PI / 2.0) < 1e-14, "area should match");
    }

    TEST(slice) {
        // 1/4 slice of the globe, from north to south pole
        // Expected area: pi
        LatLng verts[] = {{M_PI_2, 0.0},
                          {0.0, 0.0},
                          {-M_PI_2, 0.0},
                          {0.0, M_PI_2},
                          {M_PI_2, 0.0}};
        GeoLoop loop = {.verts = verts, .numVerts = ARRAY_SIZE(verts)};

        double out;
        t_assertSuccess(H3_EXPORT(geoLoopArea)(loop, &out));

        t_assert(fabs(out - M_PI) < 1e-14, "area should match");
    }

    TEST(reverse_slice) {
        // 3/4 slice of the globe, from north to south pole, formed by reversing
        // order of points from example above.
        // Expected area: 3*pi
        LatLng verts[] = {{M_PI_2, 0.0},
                          {0.0, M_PI_2},
                          {-M_PI_2, 0.0},
                          {0.0, 0.0},
                          {M_PI_2, 0.0}};
        GeoLoop loop = {.verts = verts, .numVerts = ARRAY_SIZE(verts)};

        double out;
        t_assertSuccess(H3_EXPORT(geoLoopArea)(loop, &out));

        t_assert(fabs(out - 3 * M_PI) < 1e-14, "area should match");
    }

    TEST(hemisphere_east) {
        // one hemisphere of globe. western or eastern?
        // Expected area: 2*pi
        LatLng verts[] = {
            {M_PI_2, 0.0},   // north pole
            {0.0, -M_PI_2},  // equator
            {-M_PI_2, 0.0},  // south pole
            {0.0, M_PI_2},   // equator
        };
        GeoLoop loop = {.verts = verts, .numVerts = ARRAY_SIZE(verts)};

        double out;
        t_assertSuccess(H3_EXPORT(geoLoopArea)(loop, &out));

        t_assert(fabs(out - 2 * M_PI) < 1e-14, "area should match");
    }

    TEST(hemisphere_north) {
        // one hemisphere of globe. northern
        // Expected area: 2*pi
        LatLng verts[] = {
            {0.0, -M_PI},    // equator
            {0.0, -M_PI_2},  // equator
            {0.0, 0.0},      // equator
            {0.0, M_PI_2},   // equator
            // {0.0, M_PI},     // equator don't need last one here. last
            // point
            // unnecessary, but doesn't harm anything.
        };

        helper(verts, ARRAY_SIZE(verts), 2 * M_PI);
    }
}
