/*
 * Copyright 2025 Uber Technologies, Inc.
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

/** @file geodesicPolygon.h
 * @brief Internal data structures for geodesic polygon acceleration.
 */

#ifndef GEODESIC_POLYGON_INTERNAL_H
#define GEODESIC_POLYGON_INTERNAL_H

#include <stdbool.h>

#include "bbox.h"
#include "geodesicCellBoundary.h"
#include "polygon.h"
#include "vec3d.h"

/** @struct GeodesicEdge
 *  @brief Cached data describing a polygon edge on the unit sphere.
 */
typedef struct {
    Vec3d vert;       ///< Vertex position on the unit sphere
    Vec3d edgeCross;  ///< Great-circle normal for the edge starting at vert
    double edgeDot;   ///< Dot product between consecutive vertices
    AABB aabb;        ///< Bounding box covering the edge arc
} GeodesicEdge;

/** @struct GeodesicLoop
 *  @brief Polygon loop represented in geodesic space.
 */
typedef struct {
    int numVerts;         ///< Number of vertices in the loop
    GeodesicEdge *edges;  ///< Edge data for the loop
    Vec3d centroid;       ///< Approximate centroid used for hemisphere tests
} GeodesicLoop;

/** @struct GeodesicPolygon
 *  @brief Geodesic acceleration structure for polygon containment tests.
 */
typedef struct {
    GeodesicLoop geoloop;  ///< Exterior boundary in geodesic space
    AABB aabb;             ///< Bounding box covering the entire polygon
    int numHoles;          ///< Number of interior loops
    GeodesicLoop *holes;   ///< Optional interior loops in geodesic space
} GeodesicPolygon;

/**
 * Build a geodesic acceleration structure from a geographic polygon.
 *
 * @param polygon Source polygon expressed in lat/lng.
 * @return Allocated geodesic polygon, or `NULL` on invalid input/allocation
 *         failure.
 */
GeodesicPolygon *geodesicPolygonCreate(const GeoPolygon *polygon);

/** Destroy a geodesic polygon created by `geodesicPolygonCreate`. */
void geodesicPolygonDestroy(GeodesicPolygon *polygon);

/**
 * Check whether a cell bounding cap intersects the polygon's outer hull.
 *
 * @param polygon Geodesic polygon to test.
 * @param cap Bounding cap derived from a candidate cell.
 * @return `true` when the cap may intersect the polygon.
 */
bool geodesicPolygonCapIntersects(const GeodesicPolygon *polygon,
                                  const SphereCap *cap);

/**
 * Test whether a cell boundary intersects any polygon loop.
 *
 * @param polygon Geodesic polygon to test.
 * @param boundary Geodesic boundary of the candidate cell.
 * @param cap Bounding cap for the candidate cell.
 * @return `true` if the cell boundary crosses the polygon.
 */
bool geodesicPolygonBoundaryIntersects(const GeodesicPolygon *polygon,
                                       const GeodesicCellBoundary *boundary,
                                       const SphereCap *cap);

/**
 * Determine whether a point on the unit sphere lies inside the polygon.
 *
 * @param polygon Geodesic polygon to test.
 * @param point Unit vector for the point of interest.
 * @return `true` when the point is inside the polygon exterior and outside all
 *         holes.
 */
bool geodesicPolygonContainsPoint(const GeodesicPolygon *polygon,
                                  const Vec3d *point);

#endif
