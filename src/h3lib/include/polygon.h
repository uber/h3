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
/** @file polygon.h
 * @brief Polygon algorithms
 */

#ifndef POLYGON_H
#define POLYGON_H

#include <stdbool.h>
#include "bbox.h"
#include "geoCoord.h"
#include "h3api.h"

void bboxFromVertices(const GeoCoord* verts, int numVerts, BBox* bbox);
void bboxFromGeofence(const Geofence* geofence, BBox* bbox);
void bboxesFromGeoPolygon(const GeoPolygon* polygon, BBox* bboxes);
bool loopContainsPoint(const Geofence* geofence, const BBox* bbox,
                       const GeoCoord* coord);
bool polygonContainsPoint(const GeoPolygon* geoPolygon, const BBox* bboxes,
                          const GeoCoord* coord);

#endif
