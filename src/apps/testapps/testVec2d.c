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
#include "vec2d.h"

SUITE(Vec2d) {
    TEST(_v2dMag) {
        Vec2d v = {3.0, 4.0};
        double expected = 5.0;
        double mag = _v2dMag(&v);
        t_assert(fabs(mag - expected) < DBL_EPSILON, "magnitude as expected");
    }

    TEST(_v2dIntersect) {
        Vec2d p0 = {2.0, 2.0};
        Vec2d p1 = {6.0, 6.0};
        Vec2d p2 = {0.0, 4.0};
        Vec2d p3 = {10.0, 4.0};
        Vec2d intersection = {0.0, 0.0};

        _v2dIntersect(&p0, &p1, &p2, &p3, &intersection);

        double expectedX = 4.0;
        double expectedY = 4.0;

        t_assert(fabs(intersection.x - expectedX) < DBL_EPSILON,
                 "X coord as expected");
        t_assert(fabs(intersection.y - expectedY) < DBL_EPSILON,
                 "Y coord as expected");
    }

    TEST(_v2dEquals) {
        Vec2d v1 = {3.0, 4.0};
        Vec2d v2 = {3.0, 4.0};
        Vec2d v3 = {3.5, 4.0};
        Vec2d v4 = {3.0, 4.5};

        t_assert(_v2dEquals(&v1, &v2), "true for equal vectors");
        t_assert(!_v2dEquals(&v1, &v3), "false for different x");
        t_assert(!_v2dEquals(&v1, &v4), "false for different y");
    }
}
