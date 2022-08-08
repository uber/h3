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

#include "latLng.h"

/** @struct BBox
 *  @brief  Geographic bounding box with coordinates defined in radians
 */
typedef struct {
    double north;  ///< north latitude
    double south;  ///< south latitude
    double east;   ///< east longitude
    double west;   ///< west longitude
} BBox;

bool bboxIsTransmeridian(const BBox *bbox);
void bboxCenter(const BBox *bbox, LatLng *center);
bool bboxContains(const BBox *bbox, const LatLng *point);
bool bboxEquals(const BBox *b1, const BBox *b2);
H3Error bboxHexEstimate(const BBox *bbox, int res, int64_t *out);
H3Error lineHexEstimate(const LatLng *origin, const LatLng *destination,
                        int res, int64_t *out);

#endif
