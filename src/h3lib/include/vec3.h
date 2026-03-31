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
/** @file vec3.h
 * @brief   3D floating point vector functions.
 */

#ifndef VEC3_H
#define VEC3_H

#include <math.h>

#include "h3api.h"
#include "latLng.h"

/** @struct Vec3
 *  @brief 3D floating point structure
 *
 *  For geodesic calulations represents a point on the surface of the Earth
 *  as a unit vector in 3D Cartesian space (ECEF-like coordinates).
 */
typedef struct {
    double x;  /// towards 0deg lat, 0deg lon
    double y;  /// towards 0deg lat, 90deg lon
    double z;  /// towards north pole
} Vec3;

/** Convert latitude and longitude to a unit Vec3 on the sphere. */
static inline Vec3 latLngToVec3(LatLng geo) {
    double r = cos(geo.lat);
    return (Vec3){
        cos(geo.lng) * r,
        sin(geo.lng) * r,
        sin(geo.lat),
    };
}

static inline LatLng vec3ToLatLng(Vec3 v) {
    return (LatLng){
        asin(v.z),
        atan2(v.y, v.x),
    };
}

static inline Vec3 vec3LinComb(double s1, Vec3 a, double s2, Vec3 b) {
    return (Vec3){
        s1 * a.x + s2 * b.x,
        s1 * a.y + s2 * b.y,
        s1 * a.z + s2 * b.z,
    };
}

static inline Vec3 vec3Cross(Vec3 v1, Vec3 v2) {
    return (Vec3){
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x,
    };
}

static inline double vec3Dot(Vec3 v1, Vec3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

static inline double vec3NormSq(Vec3 v) { return vec3Dot(v, v); }

static inline double vec3Norm(Vec3 v) { return sqrt(vec3NormSq(v)); }

static inline void vec3Normalize(Vec3 *v) {
    double norm = vec3Norm(*v);
    if (norm == 0.0) return;
    double inv = 1.0 / norm;
    v->x *= inv;
    v->y *= inv;
    v->z *= inv;
}

static inline double vec3DistSq(Vec3 v1, Vec3 v2) {
    Vec3 d = vec3LinComb(1.0, v1, -1.0, v2);
    return vec3NormSq(d);
}

#endif
