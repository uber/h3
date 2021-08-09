/*
 * Copyright 2017-2018, 2020-2021 Uber Technologies, Inc.
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
/** @file linkedGeo.h
 * @brief   Linked data structure for geo data
 */

#ifndef LINKED_GEO_H
#define LINKED_GEO_H

#include <stdlib.h>

#include "bbox.h"
#include "h3api.h"
#include "latLng.h"

// Error codes for normalizeMultiPolygon
#define NORMALIZATION_SUCCESS 0
#define NORMALIZATION_ERR_MULTIPLE_POLYGONS 1
#define NORMALIZATION_ERR_UNASSIGNED_HOLES 2

// Macros for use with polygonAlgos.h
/** Macro: Init iteration vars for LinkedGeoLoop */
#define INIT_ITERATION_LINKED_LOOP     \
    LinkedLatLng *currentCoord = NULL; \
    LinkedLatLng *nextCoord = NULL

/** Macro: Get the next coord in a linked loop, wrapping if needed */
#define GET_NEXT_COORD(loop, coordToCheck) \
    coordToCheck == NULL ? loop->first : currentCoord->next

/** Macro: Increment LinkedGeoLoop iteration, or break if done. */
#define ITERATE_LINKED_LOOP(loop, vertexA, vertexB)       \
    currentCoord = GET_NEXT_COORD(loop, currentCoord);    \
    if (currentCoord == NULL) break;                      \
    vertexA = currentCoord->vertex;                       \
    nextCoord = GET_NEXT_COORD(loop, currentCoord->next); \
    vertexB = nextCoord->vertex

/** Macro: Whether a LinkedGeoLoop is empty */
#define IS_EMPTY_LINKED_LOOP(loop) loop->first == NULL

int normalizeMultiPolygon(LinkedGeoPolygon *root);
LinkedGeoPolygon *addNewLinkedPolygon(LinkedGeoPolygon *polygon);
LinkedGeoLoop *addNewLinkedLoop(LinkedGeoPolygon *polygon);
LinkedGeoLoop *addLinkedLoop(LinkedGeoPolygon *polygon, LinkedGeoLoop *loop);
LinkedLatLng *addLinkedCoord(LinkedGeoLoop *loop, const LatLng *vertex);
int countLinkedPolygons(LinkedGeoPolygon *polygon);
int countLinkedLoops(LinkedGeoPolygon *polygon);
int countLinkedCoords(LinkedGeoLoop *loop);
void destroyLinkedGeoLoop(LinkedGeoLoop *loop);

// The following functions are created via macro in polygonAlgos.h,
// so their signatures are documented here:

/**
 * Create a bounding box from a LinkedGeoLoop
 * @param geoloop Input GeoLoop
 * @param bbox     Output bbox
 */
void bboxFromLinkedGeoLoop(const LinkedGeoLoop *loop, BBox *bbox);

/**
 * Take a given LinkedGeoLoop data structure and check if it
 * contains a given geo coordinate.
 * @param loop          The linked loop
 * @param bbox          The bbox for the loop
 * @param coord         The coordinate to check
 * @return              Whether the point is contained
 */
bool pointInsideLinkedGeoLoop(const LinkedGeoLoop *loop, const BBox *bbox,
                              const LatLng *coord);

/**
 * Whether the winding order of a given LinkedGeoLoop is clockwise
 * @param loop  The loop to check
 * @return      Whether the loop is clockwise
 */
bool isClockwiseLinkedGeoLoop(const LinkedGeoLoop *loop);

#endif
