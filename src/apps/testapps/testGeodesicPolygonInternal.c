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

/** @file testGeodesicPolygonInternal.c
 * @brief Tests the internal geodesic polygon acceleration structures.
 */

/*
 * Focused tests for the geodesic polygon acceleration structures backing the
 * geodesic polygonToCellsExperimental flag.
 */

#include <math.h>

#include "geodesic_polygon_internal.h"
#include "h3Index.h"
#include "test.h"
#include "vec3d.h"

static const double DEG_TO_RAD = M_PI / 180.0;

static LatLng triangleVerts[] = {{.lat = 0.0, .lng = 0.0},
                                 {.lat = 0.0, .lng = 2.0 * DEG_TO_RAD},
                                 {.lat = 2.0 * DEG_TO_RAD, .lng = 0.0}};
static GeoLoop triangleLoop = {.numVerts = 3, .verts = triangleVerts};

static GeoPolygon zeroLoopPolygon = {
    .geoloop = {.numVerts = 0, .verts = NULL}, .numHoles = 0, .holes = NULL};

SUITE(GeodesicPolygonInternal) {
    TEST(createAndDestroy) {
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = geodesicPolygonCreate(&polygon);
        t_assert(poly != NULL, "triangle polygon builds geodesic structure");
        t_assert(poly->numHoles == 0, "no holes copied");
        geodesicPolygonDestroy(poly);
    }

    TEST(invalidInputs) {
        t_assert(geodesicPolygonCreate(&zeroLoopPolygon) == NULL,
                 "zero-vertex polygon rejected");

        GeoLoop invalidHole = {.numVerts = 0, .verts = NULL};
        GeoPolygon polyWithInvalidHole = {
            .geoloop = triangleLoop, .numHoles = 1, .holes = &invalidHole};
        t_assert(geodesicPolygonCreate(&polyWithInvalidHole) == NULL,
                 "hole with zero vertices rejected");
    }

    TEST(containsPoint) {
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = geodesicPolygonCreate(&polygon);
        t_assert(poly != NULL, "triangle polygon created");

        LatLng insideLl = {.lat = 0.5 * DEG_TO_RAD, .lng = 0.5 * DEG_TO_RAD};
        LatLng outsideLl = {.lat = 3.0 * DEG_TO_RAD, .lng = 3.0 * DEG_TO_RAD};

        Vec3d insideVec = latLngToVec3(&insideLl);
        Vec3d outsideVec = latLngToVec3(&outsideLl);

        t_assert(geodesicPolygonContainsPoint(poly, &insideVec),
                 "point inside polygon detected");
        t_assert(!geodesicPolygonContainsPoint(poly, &outsideVec),
                 "point outside polygon rejected");

        geodesicPolygonDestroy(poly);
    }

    TEST(capIntersection) {
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = geodesicPolygonCreate(&polygon);
        t_assert(poly != NULL, "triangle polygon created");

        LatLng centerLl = {.lat = 0.5 * DEG_TO_RAD, .lng = 0.5 * DEG_TO_RAD};
        H3Index cell;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&centerLl, 1, &cell));

        SphereCap cap;
        t_assertSuccess(cellToSphereCap(cell, &cap));
        t_assert(geodesicPolygonCapIntersects(poly, &cap),
                 "cap overlapping polygon detected");

        LatLng farLl = {.lat = 30.0 * DEG_TO_RAD, .lng = -50.0 * DEG_TO_RAD};
        t_assertSuccess(H3_EXPORT(latLngToCell)(&farLl, 1, &cell));
        t_assertSuccess(cellToSphereCap(cell, &cap));
        t_assert(!geodesicPolygonCapIntersects(poly, &cap),
                 "distant caps rejected");

        geodesicPolygonDestroy(poly);
    }

    TEST(boundaryIntersection) {
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = geodesicPolygonCreate(&polygon);
        t_assert(poly != NULL, "triangle polygon created");

        GeodesicCellBoundary boundary = {.numVerts = triangleLoop.numVerts};
        for (int i = 0; i < triangleLoop.numVerts; i++) {
            boundary.verts[i] = latLngToVec3(&triangleLoop.verts[i]);
        }

        SphereCap permissiveCap = {
            .center = latLngToVec3(&triangleLoop.verts[0]), .cosRadius = -1.0};
        t_assert(
            geodesicPolygonBoundaryIntersects(poly, &boundary, &permissiveCap),
            "coincident boundaries reported as intersecting");

        GeodesicCellBoundary farBoundary = {.numVerts = 4};
        LatLng squareLl[] = {
            {.lat = 10.0 * DEG_TO_RAD, .lng = 10.0 * DEG_TO_RAD},
            {.lat = 10.0 * DEG_TO_RAD, .lng = 15.0 * DEG_TO_RAD},
            {.lat = 15.0 * DEG_TO_RAD, .lng = 15.0 * DEG_TO_RAD},
            {.lat = 15.0 * DEG_TO_RAD, .lng = 10.0 * DEG_TO_RAD}};
        for (int i = 0; i < farBoundary.numVerts; i++) {
            farBoundary.verts[i] = latLngToVec3(&squareLl[i]);
        }
        SphereCap farCap = {.center = latLngToVec3(&squareLl[0]),
                            .cosRadius = cos(2.0 * DEG_TO_RAD)};
        t_assert(
            !geodesicPolygonBoundaryIntersects(poly, &farBoundary, &farCap),
            "far boundary does not intersect");

        geodesicPolygonDestroy(poly);
    }
}
