/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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
/** @file vec2d.c
 * @brief   2D floating point vector functions.
 */

#include "vec2d.h"

#include <math.h>
#include <stdio.h>

/**
 * Calculates the magnitude of a 2D cartesian vector.
 * @param v The 2D cartesian vector.
 * @return The magnitude of the vector.
 */
double _v2dMag(const Vec2d* v) { return sqrt(v->x * v->x + v->y * v->y); }

/**
 * Finds the intersection between two lines. Assumes that the lines intersect
 * and that the intersection is not at an endpoint of either line.
 * @param p0 The first endpoint of the first line.
 * @param p1 The second endpoint of the first line.
 * @param p2 The first endpoint of the second line.
 * @param p3 The second endpoint of the second line.
 * @param inter The intersection point.
 */
void _v2dIntersect(const Vec2d* p0, const Vec2d* p1, const Vec2d* p2,
                   const Vec2d* p3, Vec2d* inter) {
    Vec2d s1, s2;
    s1.x = p1->x - p0->x;
    s1.y = p1->y - p0->y;
    s2.x = p3->x - p2->x;
    s2.y = p3->y - p2->y;

    float t;
    t = (s2.x * (p0->y - p2->y) - s2.y * (p0->x - p2->x)) /
        (-s2.x * s1.y + s1.x * s2.y);

    inter->x = p0->x + (t * s1.x);
    inter->y = p0->y + (t * s1.y);
}

/**
 * Whether two 2D vectors are equal. Does not consider possible false
 * negatives due to floating-point errors.
 * @param v1 First vector to compare
 * @param v2 Second vector to compare
 * @return Whether the vectors are equal
 */
bool _v2dEquals(const Vec2d* v1, const Vec2d* v2) {
    return v1->x == v2->x && v1->y == v2->y;
}
