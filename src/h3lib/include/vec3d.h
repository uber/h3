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
/** @file vec3d.h
 * @brief   3D floating point vector functions.
 */

#ifndef VEC3D_H
#define VEC3D_H

#include "latLng.h"

/** @struct Vec3D
 *  @brief 3D floating point structure
 */
typedef struct {
    double x;  ///< x component
    double y;  ///< y component
    double z;  ///< z component
} Vec3d;

void _geoToVec3d(const LatLng *geo, Vec3d *point);
double _pointSquareDist(const Vec3d *p1, const Vec3d *p2);

#endif
