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
 * @brief Tests the Vec3 helpers used by the geodesic polyfill path.
 */

#include <math.h>

#include "constants.h"
#include "h3Index.h"
#include "test.h"
#include "vec3d.h"

SUITE(Vec3) {
    TEST(dotProduct) {
        Vec3 a = {.x = 1.0, .y = 0.0, .z = 0.0};
        Vec3 b = {.x = -1.0, .y = 0.0, .z = 0.0};
        t_assert(vec3Dot(a, b) == -1.0, "dot product matches expected value");
    }

    TEST(crossProductOrthogonality) {
        Vec3 i = {.x = 1.0, .y = 0.0, .z = 0.0};
        Vec3 j = {.x = 0.0, .y = 1.0, .z = 0.0};
        Vec3 k = vec3Cross(i, j);
        t_assert(fabs(k.x - 0.0) < EPSILON, "x component zero");
        t_assert(fabs(k.y - 0.0) < EPSILON, "y component zero");
        t_assert(fabs(k.z - 1.0) < EPSILON, "z component one");
        t_assert(fabs(vec3Dot(k, i)) < EPSILON, "cross is orthogonal to i");
        t_assert(fabs(vec3Dot(k, j)) < EPSILON, "cross is orthogonal to j");
    }

    TEST(normalizeAndMagnitude) {
        Vec3 v = {.x = 3.0, .y = -4.0, .z = 12.0};
        double magSq = vec3NormSq(v);
        t_assert(fabs(magSq - 169.0) < EPSILON, "magnitude squared matches");
        t_assert(fabs(vec3Norm(v) - 13.0) < EPSILON, "magnitude matches");

        vec3Normalize(&v);
        t_assert(fabs(vec3Norm(v) - 1.0) < 1e-12, "normalized vector is unit");

        Vec3 zero = {.x = 0.0, .y = 0.0, .z = 0.0};
        vec3Normalize(&zero);
        t_assert(zero.x == 0.0 && zero.y == 0.0 && zero.z == 0.0,
                 "zero vector remains unchanged when normalizing");
    }

    TEST(distance) {
        Vec3 a = {.x = 0.0, .y = 0.0, .z = 0.0};
        Vec3 b = {.x = 1.0, .y = 2.0, .z = 2.0};
        t_assert(fabs(vec3DistSq(a, b) - 9.0) < EPSILON,
                 "distance squared matches");
    }

    TEST(latLngToVec3_unitSphere) {
        LatLng geo = {.lat = 0.5, .lng = -1.3};
        Vec3 v = latLngToVec3(geo);
        t_assert(fabs(vec3Norm(v) - 1.0) < 1e-12,
                 "converted vector lives on the unit sphere");
    }

    TEST(vec3ToCell_invalidRes) {
        Vec3 v = {.x = 1.0, .y = 0.0, .z = 0.0};
        H3Index out;
        t_assert(vec3ToCell(&v, -1, &out) == E_RES_DOMAIN,
                 "negative resolution is rejected");
        t_assert(vec3ToCell(&v, 16, &out) == E_RES_DOMAIN,
                 "resolution above max is rejected");
    }

    TEST(vec3ToCell_nonFinite) {
        H3Index out;
        Vec3 nan_x = {.x = NAN, .y = 0.0, .z = 0.0};
        t_assert(vec3ToCell(&nan_x, 0, &out) == E_DOMAIN, "NaN x is rejected");
        Vec3 inf_y = {.x = 0.0, .y = INFINITY, .z = 0.0};
        t_assert(vec3ToCell(&inf_y, 0, &out) == E_DOMAIN,
                 "infinite y is rejected");
        Vec3 inf_z = {.x = 0.0, .y = 0.0, .z = -INFINITY};
        t_assert(vec3ToCell(&inf_z, 0, &out) == E_DOMAIN,
                 "infinite z is rejected");
    }
}
