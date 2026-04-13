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

/** @file testVec3.c
 * @brief Tests the Vec3d helpers used by the geodesic polyfill path.
 */

#include <float.h>
#include <math.h>

#include "h3Index.h"
#include "test.h"
#include "vec3d.h"

SUITE(Vec3d) {
    TEST(dotProduct) {
        Vec3d a = {.x = 1.0, .y = 0.0, .z = 0.0};
        Vec3d b = {.x = -1.0, .y = 0.0, .z = 0.0};
        t_assert(vec3Dot(a, b) == -1.0, "dot product matches expected value");
    }

    TEST(crossProductOrthogonality) {
        Vec3d i = {.x = 1.0, .y = 0.0, .z = 0.0};
        Vec3d j = {.x = 0.0, .y = 1.0, .z = 0.0};
        Vec3d k = vec3Cross(i, j);
        t_assert(fabs(k.x - 0.0) < DBL_EPSILON, "x component zero");
        t_assert(fabs(k.y - 0.0) < DBL_EPSILON, "y component zero");
        t_assert(fabs(k.z - 1.0) < DBL_EPSILON, "z component one");
        t_assert(fabs(vec3Dot(k, i)) < DBL_EPSILON, "cross is orthogonal to i");
        t_assert(fabs(vec3Dot(k, j)) < DBL_EPSILON, "cross is orthogonal to j");
    }

    TEST(normalizeAndMagnitude) {
        Vec3d v = {.x = 3.0, .y = -4.0, .z = 12.0};
        double magSq = vec3NormSq(v);
        t_assert(fabs(magSq - 169.0) < DBL_EPSILON,
                 "magnitude squared matches");
        t_assert(fabs(vec3Norm(v) - 13.0) < DBL_EPSILON, "magnitude matches");

        vec3Normalize(&v);
        t_assert(fabs(vec3Norm(v) - 1.0) < DBL_EPSILON,
                 "normalized vector is unit");

        Vec3d zero = {.x = 0.0, .y = 0.0, .z = 0.0};
        vec3Normalize(&zero);
        t_assert(zero.x == 0.0 && zero.y == 0.0 && zero.z == 0.0,
                 "zero vector remains unchanged when normalizing");
    }

    TEST(distance) {
        Vec3d a = {.x = 0.0, .y = 0.0, .z = 0.0};
        Vec3d b = {.x = 1.0, .y = 2.0, .z = 2.0};
        t_assert(fabs(vec3DistSq(a, b) - 9.0) < DBL_EPSILON,
                 "distance squared matches");
    }

    TEST(latLngToVec3_unitSphere) {
        LatLng geo = {.lat = 0.5, .lng = -1.3};
        Vec3d v = latLngToVec3(geo);
        t_assert(fabs(vec3Norm(v) - 1.0) < DBL_EPSILON,
                 "converted vector lives on the unit sphere");
    }

    TEST(vec3ToCell_invalidRes) {
        Vec3d v = {.x = 1.0, .y = 0.0, .z = 0.0};
        H3Index out;
        t_assert(vec3ToCell(&v, -1, &out) == E_RES_DOMAIN,
                 "negative resolution is rejected");
        t_assert(vec3ToCell(&v, 16, &out) == E_RES_DOMAIN,
                 "resolution above max is rejected");
    }

    TEST(cellToVec3_unitSphere) {
        // cellToVec3 should return a point on the unit sphere.
        LatLng p = {.lat = 0.6, .lng = -1.2};
        H3Index h;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&p, 5, &h));

        Vec3d v;
        t_assertSuccess(cellToVec3(h, &v));
        t_assert(fabs(vec3Norm(v) - 1.0) < DBL_EPSILON,
                 "cellToVec3 result is on the unit sphere");
    }

    TEST(cellToVec3_matchesCellToLatLng) {
        // vec3ToLatLng(cellToVec3(cell)) should agree with cellToLatLng.
        LatLng p = {.lat = 0.3, .lng = 2.1};
        H3Index h;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&p, 7, &h));

        Vec3d v;
        t_assertSuccess(cellToVec3(h, &v));
        LatLng fromVec3 = vec3ToLatLng(v);

        LatLng fromCell;
        t_assertSuccess(H3_EXPORT(cellToLatLng)(h, &fromCell));

        t_assert(fabs(fromVec3.lat - fromCell.lat) < DBL_EPSILON,
                 "lat matches cellToLatLng");
        t_assert(fabs(fromVec3.lng - fromCell.lng) < DBL_EPSILON,
                 "lng matches cellToLatLng");
    }

    TEST(cellToVec3_roundTrip) {
        // vec3ToCell(cellToVec3(cell)) should return the same cell.
        LatLng p = {.lat = -0.4, .lng = 0.8};
        H3Index h;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&p, 9, &h));

        Vec3d v;
        t_assertSuccess(cellToVec3(h, &v));

        H3Index h2;
        t_assertSuccess(vec3ToCell(&v, 9, &h2));
        t_assert(h2 == h, "round-trip through Vec3d returns same cell");
    }

    TEST(cellToVec3_invalidCell) {
        Vec3d v;
        t_assert(cellToVec3(0x7fffffffffffffff, &v) == E_CELL_INVALID,
                 "invalid cell gives E_CELL_INVALID");
    }

    TEST(vec3ToCell_nonFinite) {
        H3Index out;
        Vec3d nanX = {.x = NAN, .y = 0.0, .z = 0.0};
        t_assert(vec3ToCell(&nanX, 0, &out) == E_DOMAIN, "NaN x is rejected");
        Vec3d infY = {.x = 0.0, .y = INFINITY, .z = 0.0};
        t_assert(vec3ToCell(&infY, 0, &out) == E_DOMAIN,
                 "infinite y is rejected");
        Vec3d infZ = {.x = 0.0, .y = 0.0, .z = -INFINITY};
        t_assert(vec3ToCell(&infZ, 0, &out) == E_DOMAIN,
                 "infinite z is rejected");
    }
}
