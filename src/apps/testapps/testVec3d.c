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

/** @file testVec3d.c
 * @brief Tests the vec3d helpers used by the geodesic polyfill path.
 */

#include <math.h>

#include "constants.h"
#include "test.h"
#include "vec3d.h"

SUITE(Vec3d) {
    TEST(dotProduct) {
        Vec3d a = {.x = 1.0, .y = 0.0, .z = 0.0};
        Vec3d b = {.x = -1.0, .y = 0.0, .z = 0.0};
        t_assert(vec3Dot(&a, &b) == -1.0, "dot product matches expected value");
    }

    TEST(crossProductOrthogonality) {
        Vec3d i = {.x = 1.0, .y = 0.0, .z = 0.0};
        Vec3d j = {.x = 0.0, .y = 1.0, .z = 0.0};
        Vec3d k;
        vec3Cross(&i, &j, &k);
        t_assert(fabs(k.x - 0.0) < EPSILON, "x component zero");
        t_assert(fabs(k.y - 0.0) < EPSILON, "y component zero");
        t_assert(fabs(k.z - 1.0) < EPSILON, "z component one");
        t_assert(fabs(vec3Dot(&k, &i)) < EPSILON, "cross is orthogonal to i");
        t_assert(fabs(vec3Dot(&k, &j)) < EPSILON, "cross is orthogonal to j");
    }

    TEST(normalizeAndMagnitude) {
        Vec3d v = {.x = 3.0, .y = -4.0, .z = 12.0};
        double magSq = vec3MagSq(&v);
        t_assert(fabs(magSq - 169.0) < EPSILON, "magnitude squared matches");
        t_assert(fabs(vec3Mag(&v) - 13.0) < EPSILON, "magnitude matches");

        vec3Normalize(&v);
        t_assert(fabs(vec3Mag(&v) - 1.0) < 1e-12, "normalized vector is unit");

        Vec3d zero = {.x = 0.0, .y = 0.0, .z = 0.0};
        vec3Normalize(&zero);
        t_assert(zero.x == 0.0 && zero.y == 0.0 && zero.z == 0.0,
                 "zero vector remains unchanged when normalizing");
    }

    TEST(distance) {
        Vec3d a = {.x = 0.0, .y = 0.0, .z = 0.0};
        Vec3d b = {.x = 1.0, .y = 2.0, .z = 2.0};
        t_assert(fabs(vec3DistSq(&a, &b) - 9.0) < EPSILON,
                 "distance squared matches");
    }

    TEST(latLngConversionConsistency) {
        LatLng geo = {.lat = 0.5, .lng = -1.3};
        Vec3d viaReturn;
        latLngToVec3(&geo, &viaReturn);
        Vec3d viaOut;
        _geoToVec3d(&geo, &viaOut);

        t_assert(fabs(viaReturn.x - viaOut.x) < EPSILON,
                 "x coordinate consistent");
        t_assert(fabs(viaReturn.y - viaOut.y) < EPSILON,
                 "y coordinate consistent");
        t_assert(fabs(viaReturn.z - viaOut.z) < EPSILON,
                 "z coordinate consistent");
        t_assert(fabs(vec3Mag(&viaReturn) - 1.0) < 1e-12,
                 "converted vector lives on the unit sphere");
    }
}
