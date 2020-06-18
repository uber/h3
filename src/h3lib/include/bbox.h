/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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

#include "geoCoord.h"

/** @struct BBox
 *  @brief  Geographic bounding box with coordinates defined in radians
 */
typedef struct {
    double north;  ///< north latitude
    double south;  ///< south latitude
    double east;   ///< east longitude
    double west;   ///< west longitude
} BBox;

bool bboxIsTransmeridian(const BBox* bbox);
void bboxCenter(const BBox* bbox, GeoCoord* center);
bool bboxContains(const BBox* bbox, const GeoCoord* point);
bool bboxEquals(const BBox* b1, const BBox* b2);
int bboxHexEstimate(const BBox* bbox, int res);
int lineHexEstimate(const GeoCoord* origin, const GeoCoord* destination,
                    int res);

#endif
