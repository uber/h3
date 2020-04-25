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
#include "linkedGeo.h"

// Macros for use with polygonAlgos.h
/** Macro: Init iteration vars for Geofence */
#define INIT_ITERATION_GEOFENCE int loopIndex = -1

/** Macro: Increment Geofence loop iteration, or break if done. */
#define ITERATE_GEOFENCE(geofence, vertexA, vertexB) \
    if (++loopIndex >= geofence->numVerts) break;    \
    vertexA = geofence->verts[loopIndex];            \
    vertexB = geofence->verts[(loopIndex + 1) % geofence->numVerts]

/** Macro: Whether a Geofence is empty */
#define IS_EMPTY_GEOFENCE(geofence) geofence->numVerts == 0

// Defined directly in polygon.c:
void bboxesFromGeoPolygon(const GeoPolygon* polygon, BBox* bboxes);
bool pointInsidePolygon(const GeoPolygon* geoPolygon, const BBox* bboxes,
                        const GeoCoord* coord);

// The following functions are created via macro in polygonAlgos.h,
// so their signatures are documented here:

/**
 * Create a bounding box from a Geofence
 * @param geofence Input Geofence
 * @param bbox     Output bbox
 */
void bboxFromGeofence(const Geofence* loop, BBox* bbox);

/**
 * Take a given Geofence data structure and check if it
 * contains a given geo coordinate.
 * @param loop          The geofence
 * @param bbox          The bbox for the loop
 * @param coord         The coordinate to check
 * @return              Whether the point is contained
 */
bool pointInsideGeofence(const Geofence* loop, const BBox* bbox,
                         const GeoCoord* coord);

/**
 * Whether the winding order of a given Geofence is clockwise
 * @param loop  The loop to check
 * @return      Whether the loop is clockwise
 */
bool isClockwiseGeofence(const Geofence* geofence);

#endif
