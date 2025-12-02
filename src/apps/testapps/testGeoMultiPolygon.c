/*
 * Copyright 2025 Uber Technologies, Inc.
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
 * @brief tests for GeoMultiPolygon, GeoPolygon, and GeoLoop
 *
 *  usage: `testGeoMultiPolygon`
 */

#include <math.h>
#include <string.h>

#include "alloc.h"
#include "area.h"
#include "h3api.h"
#include "test.h"
#include "utility.h"

SUITE(geoMultiPolygon) {
    TEST(globalMultiPolygonArea) {
        double tol = 1e-14;
        double out;

        GeoMultiPolygon mpoly = createGlobeMultiPolygon();
        t_assertSuccess(geoMultiPolygonAreaRads2(mpoly, &out));
        t_assert(fabs(out - 4 * M_PI) < tol, "area should match");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }

    TEST(holeSameAsOuter) {
        /**
         * TODO: Replace with simpler test.
         *
         * I needed a test to exercize the "hole" branches of
         * `destroyGeoMultiPolygon` and `geoMultiPolygonAreaRads2`.
         *
         * This is a verbose test because we have to allocate the
         * `GeoMultiPolygon`. When we add in the `cellsToMultiPolygon` function,
         * I can replace this with a much shorter, clearer test.
         */

        // Create a polygon with a triangle outer and a hole of exactly
        // the same size, so the resulting polygon and multipolygons should
        // have 0 area.
        LatLng _outer[] = {
            // Counter-clockwise points
            {M_PI_2, 0},
            {0, 0},
            {0, M_PI_2},
        };
        LatLng _hole[] = {
            // Same as above, but clockwise points
            {M_PI_2, 0},
            {0, M_PI_2},
            {0, 0},
        };

        GeoLoop outer = {
            .numVerts = 3,
            .verts = H3_MEMORY(malloc)(3 * sizeof(LatLng)),
        };
        GeoLoop hole = {
            .numVerts = 3,
            .verts = H3_MEMORY(malloc)(3 * sizeof(LatLng)),
        };

        memcpy(outer.verts, _outer, 3 * sizeof(LatLng));
        memcpy(hole.verts, _hole, 3 * sizeof(LatLng));

        GeoPolygon poly = {
            .geoloop = outer,
            .numHoles = 1,
            .holes = H3_MEMORY(malloc)(sizeof(GeoLoop)),
        };
        poly.holes[0] = hole;

        GeoMultiPolygon mpoly = {
            .numPolygons = 1,
            .polygons = H3_MEMORY(malloc)(sizeof(GeoPolygon)),
        };
        mpoly.polygons[0] = poly;

        double out;
        t_assertSuccess(geoMultiPolygonAreaRads2(mpoly, &out));
        t_assert(fabs(out) < 1e-14, "Area should be 0");

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
    }
}
