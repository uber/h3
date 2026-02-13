/*
 * Copyright 2018, 2020-2021 Uber Technologies, Inc.
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

#include "constants.h"

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
double _pointSquareDist(const Vec3d *v1, const Vec3d *v2) {
    return _square(v1->x - v2->x) + _square(v1->y - v2->y) +
           _square(v1->z - v2->z);
}

/**
 * Calculate the 3D coordinate on unit sphere from the latitude and longitude.
 *
 * @param geo The latitude and longitude of the point.
 * @param v The 3D coordinate of the point.
 */
void _geoToVec3d(const LatLng *geo, Vec3d *v) {
    double r = cos(geo->lat);

    v->z = sin(geo->lat);
    v->x = cos(geo->lng) * r;
    v->y = sin(geo->lng) * r;
}

double vec3Dot(const Vec3d *v1, const Vec3d *v2) {
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

void vec3Cross(const Vec3d *v1, const Vec3d *v2, Vec3d *out) {
    out->x = v1->y * v2->z - v1->z * v2->y;
    out->y = v1->z * v2->x - v1->x * v2->z;
    out->z = v1->x * v2->y - v1->y * v2->x;
}

void vec3Normalize(Vec3d *v) {
    double mag = vec3Mag(v);
    // Check for zero-length vector to avoid division by zero.
    // Using a small epsilon for robustness.
    if (mag > EPSILON) {
        double invMag = 1.0 / mag;
        v->x *= invMag;
        v->y *= invMag;
        v->z *= invMag;
    }
}

double vec3MagSq(const Vec3d *v) { return vec3Dot(v, v); }

double vec3Mag(const Vec3d *v) { return sqrt(vec3Dot(v, v)); }

double vec3DistSq(const Vec3d *v1, const Vec3d *v2) {
    double dx = v1->x - v2->x;
    double dy = v1->y - v2->y;
    double dz = v1->z - v2->z;
    return dx * dx + dy * dy + dz * dz;
}

void latLngToVec3(const LatLng *geo, Vec3d *v) { _geoToVec3d(geo, v); }
