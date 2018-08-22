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

/** Macro: Init iteration vars for Geofence */
#define INIT_ITERATION_GEOFENCE int loopIndex = -1

/** Macro: Increment Geofence loop iteration, or break if done. */
#define ITERATE_GEOFENCE(geofence, vertexA, vertexB) \
    if (++loopIndex >= geofence->numVerts) break;    \
    vertexA = geofence->verts[loopIndex];            \
    vertexB = geofence->verts[(loopIndex + 1) % geofence->numVerts]

/** Macro: Whether a Geofence is empty */
#define IS_EMPTY_GEOFENCE(geofence) geofence->numVerts == 0

/** Macro: Init iteration vars for LinkedGeoLoop */
#define INIT_ITERATION_LINKED_LOOP       \
    LinkedGeoCoord* currentCoord = NULL; \
    LinkedGeoCoord* nextCoord = NULL

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

void bboxFromGeofence(const Geofence* loop, BBox* bbox);
void bboxesFromGeoPolygon(const GeoPolygon* polygon, BBox* bboxes);
bool pointInsideGeofence(const Geofence* loop, const BBox* bbox,
                         const GeoCoord* coord);
bool pointInsidePolygon(const GeoPolygon* geoPolygon, const BBox* bboxes,
                        const GeoCoord* coord);
void isClockwiseLinkedGeofence(const Geofence* loop);
void bboxFromLinkedGeoLoop(const LinkedGeoLoop* loop, BBox* bbox);
bool pointInsideLinkedGeoLoop(const LinkedGeoLoop* loop, const BBox* bbox,
                              const GeoCoord* coord);
bool isClockwiseLinkedGeoLoop(const LinkedGeoLoop* loop);
bool isClockwiseGeofence(const Geofence* geofence);

#endif
