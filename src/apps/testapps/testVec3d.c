/*
 * Copyright 2018 Uber Technologies, Inc.
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

#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "test.h"
#include "vec3d.h"

SUITE(Vec3d) {
    TEST(_pointSquareDist) {
        Vec3d v1 = {0, 0, 0};
        Vec3d v2 = {1, 0, 0};
        Vec3d v3 = {0, 1, 1};
        Vec3d v4 = {1, 1, 1};
        Vec3d v5 = {1, 1, 2};

        t_assert(fabs(_pointSquareDist(&v1, &v1)) < DBL_EPSILON,
                 "distance to self is 0");
        t_assert(fabs(_pointSquareDist(&v1, &v2) - 1) < DBL_EPSILON,
                 "distance to <1,0,0> is 1");
        t_assert(fabs(_pointSquareDist(&v1, &v3) - 2) < DBL_EPSILON,
                 "distance to <0,1,1> is 2");
        t_assert(fabs(_pointSquareDist(&v1, &v4) - 3) < DBL_EPSILON,
                 "distance to <1,1,1> is 3");
        t_assert(fabs(_pointSquareDist(&v1, &v5) - 6) < DBL_EPSILON,
                 "distance to <1,1,2> is 6");
    }

    TEST(_geoToVec3d) {
        Vec3d origin = {0};

        GeoCoord c1 = {0, 0};
        Vec3d p1;
        _geoToVec3d(&c1, &p1);
        t_assert(fabs(_pointSquareDist(&origin, &p1) - 1) < EPSILON_RAD,
                 "Geo point is on the unit sphere");

        GeoCoord c2 = {M_PI_2, 0};
        Vec3d p2;
        _geoToVec3d(&c2, &p2);
        t_assert(fabs(_pointSquareDist(&p1, &p2) - 2) < EPSILON_RAD,
                 "Geo point is on another axis");

        GeoCoord c3 = {M_PI, 0};
        Vec3d p3;
        _geoToVec3d(&c3, &p3);
        t_assert(fabs(_pointSquareDist(&p1, &p3) - 4) < EPSILON_RAD,
                 "Geo point is the other side of the sphere");
    }
}
