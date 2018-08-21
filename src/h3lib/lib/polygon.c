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
/** @file polygon.c
 * @brief Polygon algorithms
 */

#include "polygon.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include "bbox.h"
#include "constants.h"
#include "geoCoord.h"
#include "h3api.h"

// Functions created via the include file:

/**
 * Take a given Geofence data structure and check if it
 * contains a given geo coordinate.
 * @name pointInsideGeofence
 *
 * @param loop          The geofence
 * @param bbox          The bbox for the loop
 * @param coord         The coordinate to check
 * @return              Whether the point is contained
 */

/**
 * Take a given LinkedGeoLoop data structure and check if it
 * contains a given geo coordinate.
 * @name pointInsideLinkedGeoLoop
 *
 * @param loop          The linked loop
 * @param bbox          The bbox for the loop
 * @param coord         The coordinate to check
 * @return              Whether the point is contained
 */

/**
 * Create a bounding box from a Geofence
 * @name bboxFromGeofence
 * @param geofence Input Geofence
 * @param bbox     Output bbox
 */

/**
 * Create a bounding box from a LinkedGeoLoop
 * @name bboxFromLinkedGeoLoop
 * @param geofence Input Geofence
 * @param bbox     Output bbox
 */

/**
 * Whether the winding order of a given Geofence is clockwise
 * @name isClockwiseGeofence
 * @param loop  The loop to check
 * @return      Whether the loop is clockwise
 */

/**
 * Whether the winding order of a given LinkedGeoLoop is clockwise
 * @name isClockwiseLinkedGeoLoop
 * @param loop  The loop to check
 * @return      Whether the loop is clockwise
 */

// Define macros used in polygon algos for Geofence
#define TYPE Geofence
#define INIT_ITERATION INIT_ITERATION_GEOFENCE
#define ITERATE ITERATE_GEOFENCE
#define IS_EMPTY IS_EMPTY_GEOFENCE

#include "polygonAlgos.h"

/**
 * Create a bounding box from a GeoPolygon
 * @param polygon Input GeoPolygon
 * @param bboxes  Output bboxes, one for the outer loop and one for each hole
 */
void bboxesFromGeoPolygon(const GeoPolygon* polygon, BBox* bboxes) {
    bboxFromGeofence(&polygon->geofence, &bboxes[0]);
    for (int i = 0; i < polygon->numHoles; i++) {
        bboxFromGeofence(&polygon->holes[i], &bboxes[i + 1]);
    }
}

/**
 * pointInsidePolygon takes a given GeoPolygon data structure and
 * checks if it contains a given geo coordinate.
 *
 * @param geoPolygon The geofence and holes defining the relevant area
 * @param bboxes     The bboxes for the main geofence and each of its holes
 * @param coord      The coordinate to check
 * @return           Whether the point is contained
 */
bool pointInsidePolygon(const GeoPolygon* geoPolygon, const BBox* bboxes,
                        const GeoCoord* coord) {
    // Start with contains state of primary geofence
    bool contains =
        pointInsideGeofence(&(geoPolygon->geofence), &bboxes[0], coord);

    // If the point is contained in the primary geofence, but there are holes in
    // the geofence iterate through all holes and return false if the point is
    // contained in any hole
    if (contains && geoPolygon->numHoles > 0) {
        for (int i = 0; i < geoPolygon->numHoles; i++) {
            if (pointInsideGeofence(&(geoPolygon->holes[i]), &bboxes[i + 1],
                                    coord)) {
                return false;
            }
        }
    }

    return contains;
}

#undef TYPE
#undef IS_EMPTY
#undef INIT_ITERATION
#undef ITERATE

// Define macros used in polygon algos for LinkedGeoLoop
#define TYPE LinkedGeoLoop
#define INIT_ITERATION INIT_ITERATION_LINKED_LOOP
#define ITERATE ITERATE_LINKED_LOOP
#define IS_EMPTY IS_EMPTY_LINKED_LOOP

#include "polygonAlgos.h"

#undef TYPE
#undef IS_EMPTY
#undef INIT_ITERATION
#undef ITERATE
