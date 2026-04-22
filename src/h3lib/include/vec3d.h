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
/** @file vec3d.h
 * @brief   3D floating point vector functions.
 *
 * Header-only (static inline) so callers in other translation units
 * can inline these without requiring LTO.
 */

#ifndef VEC3D_H
#define VEC3D_H

#include <math.h>

#include "h3api.h"
#include "latLng.h"

/** @struct Vec3d
 *  @brief 3D floating point structure
 *
 *  For geodesic calculations represents a point on the surface of the Earth
 *  as a unit vector in 3D Cartesian space (ECEF-like coordinates).
 */
typedef struct {
    double x;  /// towards 0deg lat, 0deg lon
    double y;  /// towards 0deg lat, 90deg lon
    double z;  /// towards north pole
} Vec3d;

/** Convert latitude and longitude to a unit Vec3d on the sphere. */
static inline Vec3d latLngToVec3(LatLng geo) {
    double r = cos(geo.lat);
    Vec3d out = {
        .x = cos(geo.lng) * r,
        .y = sin(geo.lng) * r,
        .z = sin(geo.lat),
    };
    return out;
}

static inline LatLng vec3ToLatLng(Vec3d v) {
    LatLng out = {
        .lat = asin(v.z),
        .lng = atan2(v.y, v.x),
    };
    return out;
}

static inline Vec3d vec3LinComb(double a, Vec3d v1, double b, Vec3d v2) {
    Vec3d out = {
        .x = a * v1.x + b * v2.x,
        .y = a * v1.y + b * v2.y,
        .z = a * v1.z + b * v2.z,
    };
    return out;
}

static inline Vec3d vec3Cross(Vec3d v1, Vec3d v2) {
    Vec3d out = {
        .x = v1.y * v2.z - v1.z * v2.y,
        .y = v1.z * v2.x - v1.x * v2.z,
        .z = v1.x * v2.y - v1.y * v2.x,
    };
    return out;
}

static inline double vec3Dot(Vec3d v1, Vec3d v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

static inline double vec3NormSq(Vec3d v) { return vec3Dot(v, v); }

static inline double vec3Norm(Vec3d v) { return sqrt(vec3NormSq(v)); }

static inline void vec3Normalize(Vec3d *v) {
    double norm = vec3Norm(*v);

    // Norm can be zero either from true zero vector, or from squaring
    // underflowing to zero.
    // If the norm is nonzero, we normalize v using it.
    // If the norm is zero, we set the vector to be exactly zero.
    double s = 0.0;
    if (norm > 0.0) {
        s = 1.0 / norm;
    }

    v->x *= s;
    v->y *= s;
    v->z *= s;
}

static inline double vec3DistSq(Vec3d v1, Vec3d v2) {
    Vec3d d = vec3LinComb(1.0, v1, -1.0, v2);
    return vec3NormSq(d);
}

#endif
