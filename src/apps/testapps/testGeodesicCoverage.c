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

/** @file testGeodesicCoverage.c
 * @brief Additional tests to improve coverage of geodesic functions
 */

#include <math.h>

#include "geodesicPolygonInternal.h"
#include "latLng.h"
#include "polygon.h"
#include "test.h"

#define DEG_TO_RAD (M_PI / 180.0)

SUITE(GeodesicCoverage) {
    TEST(parallelEdges) {
        // Create a polygon with nearly parallel edges to trigger
        // edge intersection swap code paths
        LatLng verts[] = {{.lat = 0.0, .lng = 0.0},
                          {.lat = 0.0, .lng = 1.0 * DEG_TO_RAD},
                          {.lat = 0.01 * DEG_TO_RAD, .lng = 1.0 * DEG_TO_RAD},
                          {.lat = 0.01 * DEG_TO_RAD, .lng = 0.0}};
        GeoLoop loop = {.numVerts = 4, .verts = verts};
        GeoPolygon polygon = {.geoloop = loop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        // Test boundary intersection with parallel edges
        GeodesicCellBoundary boundary = {.numVerts = 2};
        LatLng edgePts[] = {
            {.lat = 0.005 * DEG_TO_RAD, .lng = 0.0},
            {.lat = 0.005 * DEG_TO_RAD, .lng = 1.0 * DEG_TO_RAD}};
        for (int i = 0; i < 2; i++) {
            latLngToVec3(&edgePts[i], &boundary.verts[i]);
        }
        SphereCap cap = {.cosRadius = cos(2.0 * DEG_TO_RAD)};
        latLngToVec3(&edgePts[0], &cap.center);
        t_assert(geodesicPolygonBoundaryIntersects(poly, &boundary, &cap),
                 "boundary crossing near-parallel edges intersects");

        geodesicPolygonDestroy(poly);
    }

    TEST(reversedEdgeProjections) {
        // Create edges where projections need to be swapped
        // to exercise the projection-order swap path.
        LatLng verts[] = {{.lat = 10.0 * DEG_TO_RAD, .lng = 0.0},
                          {.lat = 0.0, .lng = 0.0},
                          {.lat = 0.0, .lng = 10.0 * DEG_TO_RAD}};
        GeoLoop loop = {.numVerts = 3, .verts = verts};
        GeoPolygon polygon = {.geoloop = loop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        // Create a boundary that might have reversed projections
        GeodesicCellBoundary boundary = {.numVerts = 3};
        LatLng boundaryPts[] = {
            {.lat = 5.0 * DEG_TO_RAD, .lng = 5.0 * DEG_TO_RAD},
            {.lat = 0.0, .lng = 5.0 * DEG_TO_RAD},
            {.lat = 2.5 * DEG_TO_RAD, .lng = 2.5 * DEG_TO_RAD}};
        for (int i = 0; i < 3; i++) {
            latLngToVec3(&boundaryPts[i], &boundary.verts[i]);
        }
        SphereCap cap = {.cosRadius = -0.5};
        latLngToVec3(&boundaryPts[0], &cap.center);
        t_assert(geodesicPolygonBoundaryIntersects(poly, &boundary, &cap),
                 "reversed projection boundary intersects");

        geodesicPolygonDestroy(poly);
    }

    TEST(distantAABBSphere) {
        // Create a small polygon that doesn't intersect with origin sphere
        // to exercise the fast-reject sphere/AABB overlap path.
        LatLng verts[] = {{.lat = 89.0 * DEG_TO_RAD, .lng = 0.0},
                          {.lat = 89.0 * DEG_TO_RAD, .lng = 1.0 * DEG_TO_RAD},
                          {.lat = 89.5 * DEG_TO_RAD, .lng = 0.5 * DEG_TO_RAD}};
        GeoLoop loop = {.numVerts = 3, .verts = verts};
        GeoPolygon polygon = {.geoloop = loop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        // Test with a distant point
        LatLng farPt = {.lat = -89.0 * DEG_TO_RAD, .lng = 180.0 * DEG_TO_RAD};
        Vec3d farVec;
        latLngToVec3(&farPt, &farVec);
        t_assert(!geodesicPolygonContainsPoint(poly, &farVec),
                 "south-pole distant point is outside north-pole polygon");

        geodesicPolygonDestroy(poly);
    }
}
