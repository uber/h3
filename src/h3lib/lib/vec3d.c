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
/** @file vec3d.c
 * @brief   3D floating point vector functions.
 */

#include "vec3d.h"

#include <math.h>

/**
 * Square of a number
 *
 * @param x The input number.
 * @return The square of the input number.
 */
double _square(double x) { return x * x; }

/**
 * Calculate the square of the distance between two 3D coordinates.
 *
 * @param v1 The first 3D coordinate.
 * @param v2 The second 3D coordinate.
 * @return The square of the distance between the given points.
 */
double _pointSquareDist(const Vec3d* v1, const Vec3d* v2) {
    return _square(v1->x - v2->x) + _square(v1->y - v2->y) +
           _square(v1->z - v2->z);
}

/**
 * Calculate the 3D coordinate on unit sphere from the latitude and longitude.
 *
 * @param geo The latitude and longitude of the point.
 * @param v The 3D coordinate of the point.
 */
void _geoToVec3d(const GeoCoord* geo, Vec3d* v) {
    double r = cos(geo->lat);

    v->z = sin(geo->lat);
    v->x = cos(geo->lon) * r;
    v->y = sin(geo->lon) * r;
}
