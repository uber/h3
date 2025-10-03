/*
 * Copyright 2016-2017, 2020-2021 Uber Technologies, Inc.
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
/** @file bbox.h
 * @brief   Geographic bounding box functions
 */

#ifndef BBOX_H
#define BBOX_H

#include <stdbool.h>

#include "h3api.h"
#include "latLng.h"
#include "vec3d.h"

/** @struct BBox
 *  @brief  Geographic bounding box with coordinates defined in radians
 */
typedef struct {
    double north;  ///< north latitude
    double south;  ///< south latitude
    double east;   ///< east longitude
    double west;   ///< west longitude
} BBox;

/** @struct AABB
 *  @brief Axis-aligned bounding box expressed in Cartesian space on the unit
 *         sphere.
 */
typedef struct {
    Vec3d min;  ///< Minimum corner of the box
    Vec3d max;  ///< Maximum corner of the box
} AABB;

/** @struct SphereCap
 *  @brief Bounding cap described by its center vector and cosine radius.
 */
typedef struct {
    Vec3d center;      ///< Unit vector pointing to the cap center
    double cosRadius;  ///< Cosine of the angular radius of the cap
} SphereCap;

double bboxWidthRads(const BBox *bbox);
double bboxHeightRads(const BBox *bbox);
bool bboxIsTransmeridian(const BBox *bbox);
void bboxCenter(const BBox *bbox, LatLng *center);
bool bboxContains(const BBox *bbox, const LatLng *point);
bool bboxContainsBBox(const BBox *a, const BBox *b);
bool bboxOverlapsBBox(const BBox *a, const BBox *b);
bool bboxEquals(const BBox *b1, const BBox *b2);
CellBoundary bboxToCellBoundary(const BBox *bbox);
H3Error bboxHexEstimate(const BBox *bbox, int res, int64_t *out);
H3Error lineHexEstimate(const LatLng *origin, const LatLng *destination,
                        int res, int64_t *out);
void scaleBBox(BBox *bbox, double scale);
void bboxNormalization(const BBox *a, const BBox *b,
                       LongitudeNormalization *aNormalization,
                       LongitudeNormalization *bNormalization);

/**
 * Expand an AABB with extrema from the great-circle arc connecting two points.
 *
 * @param aabb Axis-aligned bounding box to expand.
 * @param v1 First endpoint of the arc.
 * @param v2 Second endpoint of the arc.
 * @param n Normal vector of the great circle defined by the arc.
 */
void aabbUpdateWithArcExtrema(AABB *aabb, const Vec3d *v1, const Vec3d *v2,
                              const Vec3d *n);

/** Reset an AABB to an empty inverted state (min > max on every axis). */
void aabbEmptyInverted(AABB *box);

/** Reset an AABB to cover the entire unit cube. */
void aabbEmptyFull(AABB *box);

/** Expand an AABB with a single Cartesian point. */
void aabbUpdateWithVec3d(AABB *aabb, const Vec3d *v);

/**
 * Compute a bounding sphere cap for a cell index.
 *
 * @param cell Cell to bound.
 * @param out Output bounding cap.
 * @return `E_SUCCESS` on success, or another error code on failure.
 */
H3Error cellToSphereCap(H3Index cell, SphereCap *out);

#endif
