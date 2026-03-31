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

#include "h3api.h"
#include "latLng.h"

/** @struct Vec3
 *  @brief 3D floating point structure
 *
 *  For geodesic calulations represents a point on the surface of the Earth
 *  as a unit vector in 3D Cartesian space (ECEF-like coordinates).
 */
typedef struct {
    double x;  ///< x component (towards 0deg lat, 0deg lon)
    double y;  ///< y component (towards 0deg lat, 90deg lon)
    double z;  ///< z component (towards north pole)
} Vec3;

double vec3Dot(const Vec3 *v1, const Vec3 *v2);
void vec3Cross(const Vec3 *v1, const Vec3 *v2, Vec3 *out);
void vec3Normalize(Vec3 *v);
double vec3NormSq(const Vec3 *v);
double vec3Norm(const Vec3 *v);
double vec3DistSq(const Vec3 *v1, const Vec3 *v2);
void latLngToVec3(const LatLng *geo, Vec3 *v);
void vec3ToLatLng(const Vec3 *v, LatLng *geo);

#endif
