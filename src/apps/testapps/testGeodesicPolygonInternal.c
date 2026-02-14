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
#include <stdlib.h>

#include "geodesicCellBoundary.h"
#include "geodesicPolygonInternal.h"
#include "h3Index.h"
#include "polygon.h"
#include "test.h"
#include "vec3d.h"

#define DEG_TO_RAD (M_PI / 180.0)

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
        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));
        t_assert(poly != NULL, "triangle polygon builds geodesic structure");
        t_assert(poly->numHoles == 0, "no holes copied");
        geodesicPolygonDestroy(poly);
    }

    TEST(invalidInputs) {
        GeodesicPolygon *poly = NULL;
        t_assert(geodesicPolygonCreate(&zeroLoopPolygon, &poly) == E_DOMAIN,
                 "zero-vertex polygon rejected");

        GeoLoop invalidHole = {.numVerts = 0, .verts = NULL};
        GeoPolygon polyWithInvalidHole = {
            .geoloop = triangleLoop, .numHoles = 1, .holes = &invalidHole};
        t_assert(geodesicPolygonCreate(&polyWithInvalidHole, &poly) == E_DOMAIN,
                 "hole with zero vertices rejected");
    }

    TEST(containsPoint) {
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        LatLng insideLl = {.lat = 0.5 * DEG_TO_RAD, .lng = 0.5 * DEG_TO_RAD};
        LatLng outsideLl = {.lat = 3.0 * DEG_TO_RAD, .lng = 3.0 * DEG_TO_RAD};

        Vec3d insideVec;
        latLngToVec3(&insideLl, &insideVec);
        Vec3d outsideVec;
        latLngToVec3(&outsideLl, &outsideVec);

        t_assert(geodesicPolygonContainsPoint(poly, &insideVec),
                 "point inside polygon detected");
        t_assert(!geodesicPolygonContainsPoint(poly, &outsideVec),
                 "point outside polygon rejected");

        geodesicPolygonDestroy(poly);
    }

    TEST(capIntersection) {
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

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
        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        GeodesicCellBoundary boundary = {.numVerts = triangleLoop.numVerts};
        for (int i = 0; i < triangleLoop.numVerts; i++) {
            latLngToVec3(&triangleLoop.verts[i], &boundary.verts[i]);
        }

        SphereCap permissiveCap = {.cosRadius = -1.0};
        latLngToVec3(&triangleLoop.verts[0], &permissiveCap.center);
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
            latLngToVec3(&squareLl[i], &farBoundary.verts[i]);
        }
        SphereCap farCap = {.cosRadius = cos(2.0 * DEG_TO_RAD)};
        latLngToVec3(&squareLl[0], &farCap.center);
        t_assert(
            !geodesicPolygonBoundaryIntersects(poly, &farBoundary, &farCap),
            "far boundary does not intersect");

        geodesicPolygonDestroy(poly);
    }

    TEST(polygonWithHolesNullPointer) {
        GeoPolygon polyWithNullHoles = {
            .geoloop = triangleLoop, .numHoles = 1, .holes = NULL};
        GeodesicPolygon *poly = NULL;
        t_assert(geodesicPolygonCreate(&polyWithNullHoles, &poly) == E_DOMAIN,
                 "polygon with holes > 0 but NULL holes pointer rejected");
    }

    TEST(destroyNullPolygon) {
        // This should not crash
        geodesicPolygonDestroy(NULL);
        t_assert(true, "destroying NULL polygon is safe");
    }

    TEST(colinearEdgeIntersection) {
        // Test edge case with nearly colinear edges that require swapping
        // projections in _edgeIntersectsEdge
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        // Test with overlapping boundary that may trigger swap paths
        GeodesicCellBoundary boundary = {.numVerts = 3};
        LatLng edgeLl[] = {{.lat = 0.5 * DEG_TO_RAD, .lng = 0.0},
                           {.lat = 1.0 * DEG_TO_RAD, .lng = 0.0},
                           {.lat = 1.5 * DEG_TO_RAD, .lng = 0.0}};
        for (int i = 0; i < boundary.numVerts; i++) {
            latLngToVec3(&edgeLl[i], &boundary.verts[i]);
        }

        SphereCap cap = {.cosRadius = cos(2.0 * DEG_TO_RAD)};
        latLngToVec3(&edgeLl[0], &cap.center);
        t_assert(geodesicPolygonBoundaryIntersects(poly, &boundary, &cap),
                 "colinear overlapping segment intersects polygon boundary");

        geodesicPolygonDestroy(poly);
    }

    TEST(polygonWithAntipodal) {
        // Test polygon with edges that may create edge cases in intersection
        LatLng antipodeVerts[] = {{.lat = 0.0, .lng = 0.0},
                                  {.lat = 0.0, .lng = M_PI},
                                  {.lat = 1.0 * DEG_TO_RAD, .lng = 0.0}};
        GeoLoop antipodeLoop = {.numVerts = 3, .verts = antipodeVerts};
        GeoPolygon polygon = {
            .geoloop = antipodeLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        // Opposite-hemisphere point should be rejected quickly.
        LatLng oppositePt = {.lat = 0.0, .lng = M_PI};
        Vec3d oppositeVec;
        latLngToVec3(&oppositePt, &oppositeVec);
        t_assert(!geodesicPolygonContainsPoint(poly, &oppositeVec),
                 "opposite-hemisphere point is outside antipodal polygon");

        geodesicPolygonDestroy(poly);
    }

    TEST(largePolygonContainsCardinalPoints) {
        // Test a large polygon that contains cardinal axis points
        // to exercise cardinal-axis probing in _geodesicLoopToAABB.
        LatLng largeVerts[] = {
            {.lat = 60.0 * DEG_TO_RAD, .lng = 0.0},
            {.lat = 60.0 * DEG_TO_RAD, .lng = 90.0 * DEG_TO_RAD},
            {.lat = 60.0 * DEG_TO_RAD, .lng = 180.0 * DEG_TO_RAD},
            {.lat = 60.0 * DEG_TO_RAD, .lng = -90.0 * DEG_TO_RAD}};
        GeoLoop largeLoop = {.numVerts = 4, .verts = largeVerts};
        GeoPolygon largePolygon = {
            .geoloop = largeLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&largePolygon, &poly));

        // Test with north pole which should be inside
        LatLng northPole = {.lat = M_PI_2, .lng = 0.0};
        Vec3d northVec;
        latLngToVec3(&northPole, &northVec);
        t_assert(geodesicPolygonContainsPoint(poly, &northVec),
                 "north pole is inside high-latitude polygon");

        geodesicPolygonDestroy(poly);
    }

    TEST(verySmallPolygon) {
        // Test a very small polygon to potentially trigger AABB edge cases
        LatLng tinyVerts[] = {
            {.lat = 0.00001 * DEG_TO_RAD, .lng = 0.0},
            {.lat = 0.00001 * DEG_TO_RAD, .lng = 0.00001 * DEG_TO_RAD},
            {.lat = 0.0, .lng = 0.00001 * DEG_TO_RAD}};
        GeoLoop tinyLoop = {.numVerts = 3, .verts = tinyVerts};
        GeoPolygon tinyPolygon = {
            .geoloop = tinyLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&tinyPolygon, &poly));

        LatLng farPoint = {.lat = 45.0 * DEG_TO_RAD, .lng = 45.0 * DEG_TO_RAD};
        Vec3d farVec;
        latLngToVec3(&farPoint, &farVec);
        t_assert(!geodesicPolygonContainsPoint(poly, &farVec),
                 "distant point is outside tiny polygon");

        // Also test cap intersection with very distant cap
        H3Index cell;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&farPoint, 5, &cell));
        SphereCap cap;
        t_assertSuccess(cellToSphereCap(cell, &cap));
        t_assert(!geodesicPolygonCapIntersects(poly, &cap),
                 "distant cap does not intersect tiny polygon AABB");

        geodesicPolygonDestroy(poly);
    }

    TEST(nullArgumentGuards) {
        LatLng testLl = {.lat = 1.0 * DEG_TO_RAD, .lng = 1.0 * DEG_TO_RAD};
        Vec3d testVec;
        latLngToVec3(&testLl, &testVec);
        GeodesicPolygon dummy = {0};

        SphereCap cap = {.center = testVec, .cosRadius = -1.0};
        GeodesicCellBoundary boundary = {.numVerts = 1, .verts = {testVec}};

        t_assert(!geodesicPolygonCapIntersects(NULL, &cap),
                 "null polygon rejected for cap test");
        t_assert(!geodesicPolygonCapIntersects(&dummy, NULL),
                 "null cap rejected");

        t_assert(!geodesicPolygonBoundaryIntersects(NULL, &boundary, &cap),
                 "null polygon rejected for boundary test");
        t_assert(!geodesicPolygonBoundaryIntersects(&dummy, NULL, &cap),
                 "null boundary rejected");
        t_assert(!geodesicPolygonBoundaryIntersects(&dummy, &boundary, NULL),
                 "null cap rejected for boundary test");

        t_assert(!geodesicPolygonContainsPoint(NULL, &testVec),
                 "null polygon rejected for containment test");
        t_assert(!geodesicPolygonContainsPoint(&dummy, NULL),
                 "null point rejected for containment test");
    }

    TEST(capRejectsAabbOutsideUnitSphere) {
        GeodesicPolygon *poly = calloc(1, sizeof(GeodesicPolygon));
        t_assert(poly != NULL, "allocated synthetic polygon");

        poly->aabb.min = (Vec3d){2.0, 2.0, 2.0};
        poly->aabb.max = (Vec3d){3.0, 3.0, 3.0};

        LatLng centerLl = {.lat = 0.0, .lng = 0.0};
        SphereCap cap = {.cosRadius = -1.0};
        latLngToVec3(&centerLl, &cap.center);

        t_assert(!geodesicPolygonCapIntersects(poly, &cap),
                 "AABB outside unit sphere rejected");

        geodesicPolygonDestroy(poly);
    }

    TEST(degenerateBoundarySegmentRejected) {
        GeoPolygon polygon = {
            .geoloop = triangleLoop, .numHoles = 0, .holes = NULL};
        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));

        // One repeated boundary vertex creates a zero-length segment.
        GeodesicCellBoundary boundary = {.numVerts = 1};
        boundary.verts[0] = poly->geoloop.edges[0].vert;

        SphereCap cap = {.center = boundary.verts[0], .cosRadius = -1.0};
        t_assert(!geodesicPolygonBoundaryIntersects(poly, &boundary, &cap),
                 "degenerate boundary segment does not intersect");

        geodesicPolygonDestroy(poly);
    }
}
