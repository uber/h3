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
/** @file polygon.h
 * @brief Polygon algorithms
 */

#ifndef POLYGON_H
#define POLYGON_H

#include <stdbool.h>

#include "bbox.h"
#include "h3api.h"
#include "latLng.h"
#include "linkedGeo.h"

// Macros for use with polygonAlgos.h
/** Macro: Init iteration vars for GeoLoop */
#define INIT_ITERATION_GEOFENCE int loopIndex = -1

/** Macro: Increment GeoLoop loop iteration, or break if done. */
#define ITERATE_GEOFENCE(geoloop, vertexA, vertexB) \
    if (++loopIndex >= geoloop->numVerts) break;    \
    vertexA = geoloop->verts[loopIndex];            \
    vertexB = geoloop->verts[(loopIndex + 1) % geoloop->numVerts]

/** Macro: Whether a GeoLoop is empty */
#define IS_EMPTY_GEOFENCE(geoloop) geoloop->numVerts == 0

// Defined directly in polygon.c:
void bboxesFromGeoPolygon(const GeoPolygon *polygon, BBox *bboxes);
bool pointInsidePolygon(const GeoPolygon *geoPolygon, const BBox *bboxes,
                        const LatLng *coord);

// The following functions are created via macro in polygonAlgos.h,
// so their signatures are documented here:

/**
 * Create a bounding box from a GeoLoop
 * @param geoloop Input GeoLoop
 * @param bbox     Output bbox
 */
void bboxFromGeoLoop(const GeoLoop *loop, BBox *bbox);

/**
 * Take a given GeoLoop data structure and check if it
 * contains a given geo coordinate.
 * @param loop          The geoloop
 * @param bbox          The bbox for the loop
 * @param coord         The coordinate to check
 * @return              Whether the point is contained
 */
bool pointInsideGeoLoop(const GeoLoop *loop, const BBox *bbox,
                        const LatLng *coord);

/**
 * Whether the winding order of a given GeoLoop is clockwise
 * @param loop  The loop to check
 * @return      Whether the loop is clockwise
 */
bool isClockwiseGeoLoop(const GeoLoop *geoloop);

#endif
