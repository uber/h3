/*
 * Copyright 2018, 2026 Uber Technologies, Inc.
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
#include "vec2.h"

SUITE(Vec2Internal) {
    TEST(_vec2Norm) {
        Vec2 v = {3.0, 4.0};
        double expected = 5.0;
        double mag = _vec2Norm(&v);
        t_assert(fabs(mag - expected) < DBL_EPSILON, "magnitude as expected");
    }

    TEST(_vec2Intersect) {
        Vec2 p0 = {2.0, 2.0};
        Vec2 p1 = {6.0, 6.0};
        Vec2 p2 = {0.0, 4.0};
        Vec2 p3 = {10.0, 4.0};
        Vec2 intersection = {0.0, 0.0};

        _vec2Intersect(&p0, &p1, &p2, &p3, &intersection);

        double expectedX = 4.0;
        double expectedY = 4.0;

        t_assert(fabs(intersection.x - expectedX) < DBL_EPSILON,
                 "X coord as expected");
        t_assert(fabs(intersection.y - expectedY) < DBL_EPSILON,
                 "Y coord as expected");
    }

    TEST(_vec2AlmostEquals) {
        Vec2 v1 = {3.0, 4.0};
        Vec2 v2 = {3.0, 4.0};
        Vec2 v3 = {3.5, 4.0};
        Vec2 v4 = {3.0, 4.5};
        Vec2 v5 = {3.0 + DBL_EPSILON, 4.0 - DBL_EPSILON};

        t_assert(_vec2AlmostEquals(&v1, &v2), "true for equal vectors");
        t_assert(!_vec2AlmostEquals(&v1, &v3), "false for different x");
        t_assert(!_vec2AlmostEquals(&v1, &v4), "false for different y");
        t_assert(_vec2AlmostEquals(&v1, &v5), "true for almost equal");
    }
}
