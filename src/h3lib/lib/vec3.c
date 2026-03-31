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
/** @file vec3.c
 * @brief   3D floating point vector functions.
 */

#include "vec3.h"

#include <math.h>

#include "constants.h"

/**
 * Calculate the 3D coordinate on unit sphere from the latitude and longitude.
 *
 * @param geo The latitude and longitude of the point.
 * @param v The 3D coordinate of the point.
 */
Vec3 latLngToVec3(const LatLng *geo) {
    double r = cos(geo->lat);
    return (Vec3){cos(geo->lng) * r, sin(geo->lng) * r, sin(geo->lat)};
}

LatLng vec3ToLatLng(const Vec3 *v) {
    return (LatLng){asin(v->z), atan2(v->y, v->x)};
}

Vec3 vec3LinComb(double s1, const Vec3 *a, double s2, const Vec3 *b) {
    return (Vec3){
        s1 * a->x + s2 * b->x,
        s1 * a->y + s2 * b->y,
        s1 * a->z + s2 * b->z,
    };
}

Vec3 vec3Cross(const Vec3 *v1, const Vec3 *v2) {
    return (Vec3){v1->y * v2->z - v1->z * v2->y, v1->z * v2->x - v1->x * v2->z,
                  v1->x * v2->y - v1->y * v2->x};
}

double vec3Dot(const Vec3 *v1, const Vec3 *v2) {
    return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}

double vec3NormSq(const Vec3 *v) { return vec3Dot(v, v); }

double vec3Norm(const Vec3 *v) { return sqrt(vec3NormSq(v)); }

void vec3Normalize(Vec3 *v) {
    double norm = vec3Norm(v);
    if (norm == 0.0) return;

    double inv = 1.0 / norm;
    v->x *= inv;
    v->y *= inv;
    v->z *= inv;
}

double vec3DistSq(const Vec3 *v1, const Vec3 *v2) {
    Vec3 d = vec3LinComb(1.0, v1, -1.0, v2);
    return vec3NormSq(&d);
}
