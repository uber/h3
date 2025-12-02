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

    TEST(triforceArea) {
        LatLng _outer[] = {
            {M_PI_2, 0},
            {0, 0},
            {0, M_PI_2},
        };
        LatLng _hole[] = {
            {M_PI_2, 0},
            {0, M_PI_2},
            {0, 0},
        };

        GeoLoop outer = {
            .numVerts = 3,
            .verts = _outer,
        };
        GeoLoop hole = {
            .numVerts = 3,
            .verts = _hole,
        };

        GeoPolygon poly = {
            .geoloop = outer,
            .numHoles = 1,
            .holes = &hole,
        };

        GeoMultiPolygon mpoly = {
            .numPolygons = 1,
            .polygons = &poly,
        };

        double tol = 1e-14;
        double out;
        t_assertSuccess(geoMultiPolygonAreaRads2(mpoly, &out));
        t_assert(fabs(out - 0) < tol, "area should match");
    }
}
