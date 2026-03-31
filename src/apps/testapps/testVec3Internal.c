/*
 * Copyright 2018, 2020-2021, 2026 Uber Technologies, Inc.
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

#include <math.h>

#include "test.h"
#include "vec3.h"

SUITE(Vec3Internal) {
    TEST(latLngToVec3) {
        Vec3 origin = {0};

        LatLng c1 = {0, 0};
        Vec3 p1 = latLngToVec3(c1);
        t_assert(fabs(vec3DistSq(origin, p1) - 1) < EPSILON_RAD,
                 "Geo point is on the unit sphere");

        LatLng c2 = {M_PI_2, 0};
        Vec3 p2 = latLngToVec3(c2);
        t_assert(fabs(vec3DistSq(p1, p2) - 2) < EPSILON_RAD,
                 "Geo point is on another axis");

        LatLng c3 = {M_PI, 0};
        Vec3 p3 = latLngToVec3(c3);
        t_assert(fabs(vec3DistSq(p1, p3) - 4) < EPSILON_RAD,
                 "Geo point is the other side of the sphere");
    }
}
