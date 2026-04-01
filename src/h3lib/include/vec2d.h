/*
 * Copyright 2016-2017, 2026 Uber Technologies, Inc.
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
/** @file vec2.h
 * @brief   2D floating point vector functions.
 */

#ifndef VEC2D_H
#define VEC2D_H

#include <stdbool.h>

/** @struct Vec2
 *  @brief 2D floating-point vector
 *
 *  Represents a point in the face-local Vec2 coordinate system:
 *  an orthogonal 2D plane centered on an icosahedron face, with the
 *  x-axis aligned to the face's i-axis and y perpendicular to it.
 */
typedef struct {
    double x;  /// aligned with face i-axis
    double y;  /// perpendicular to face i-axis
} Vec2;

// Internal functions

double _vec2Norm(const Vec2 *v);
void _vec2Intersect(const Vec2 *p0, const Vec2 *p1, const Vec2 *p2,
                    const Vec2 *p3, Vec2 *inter);
bool _vec2AlmostEquals(const Vec2 *p0, const Vec2 *p1);

#endif
