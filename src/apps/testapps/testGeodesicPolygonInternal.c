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

    TEST(greatCircleBoundaryLoopRejected) {
        // All vertices lie on the equator (a single great circle), yielding a
        // zero-area boundary loop. Reject to avoid ambiguous containment.
        LatLng equatorLineVerts[] = {{.lat = 0.0, .lng = -60.0 * DEG_TO_RAD},
                                     {.lat = 0.0, .lng = -20.0 * DEG_TO_RAD},
                                     {.lat = 0.0, .lng = 20.0 * DEG_TO_RAD},
                                     {.lat = 0.0, .lng = 60.0 * DEG_TO_RAD}};
        GeoLoop equatorLineLoop = {.numVerts = 4, .verts = equatorLineVerts};
        GeoPolygon polygon = {
            .geoloop = equatorLineLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assert(geodesicPolygonCreate(&polygon, &poly) == E_DOMAIN,
                 "great-circle boundary loop rejected in geodesic mode");
        geodesicPolygonDestroy(poly);
    }

    TEST(nearGreatCircleLoopAccepted) {
        // Similar to the great-circle boundary case, but one vertex is a few
        // degrees off the equator so the loop encloses non-zero area.
        LatLng nearEquatorVerts[] = {
            {.lat = 0.0, .lng = -60.0 * DEG_TO_RAD},
            {.lat = 0.0, .lng = -20.0 * DEG_TO_RAD},
            {.lat = 3.0 * DEG_TO_RAD, .lng = 20.0 * DEG_TO_RAD},
            {.lat = 0.0, .lng = 60.0 * DEG_TO_RAD}};
        GeoLoop nearEquatorLoop = {.numVerts = 4, .verts = nearEquatorVerts};
        GeoPolygon polygon = {
            .geoloop = nearEquatorLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));
        t_assert(poly != NULL, "near-great-circle loop remains valid");
        geodesicPolygonDestroy(poly);
    }

    TEST(polarCapSmallCircleLoopAccepted) {
        // Vertices share a high latitude (small circle, not a great circle).
        // This is a valid polygon and should be accepted.
        LatLng polarCapVerts[] = {
            {.lat = 80.0 * DEG_TO_RAD, .lng = -170.0 * DEG_TO_RAD},
            {.lat = 80.0 * DEG_TO_RAD, .lng = -60.0 * DEG_TO_RAD},
            {.lat = 80.0 * DEG_TO_RAD, .lng = 60.0 * DEG_TO_RAD},
            {.lat = 80.0 * DEG_TO_RAD, .lng = 170.0 * DEG_TO_RAD}};
        GeoLoop polarCapLoop = {.numVerts = 4, .verts = polarCapVerts};
        GeoPolygon polygon = {
            .geoloop = polarCapLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));
        t_assert(poly != NULL, "polar-cap small-circle loop is accepted");

        LatLng northPole = {.lat = M_PI_2, .lng = 0.0};
        Vec3d northVec;
        latLngToVec3(&northPole, &northVec);
        t_assert(geodesicPolygonContainsPoint(poly, &northVec),
                 "north pole is inside polar-cap polygon");

        geodesicPolygonDestroy(poly);
    }

    TEST(hemispherePoleFallsBackToVertexCandidate) {
        // This loop is crafted so the centroid candidate is rejected, while
        // one vertex is a valid containing-hemisphere pole.
        LatLng verts[] = {{.lat = 29.512732152343 * DEG_TO_RAD,
                           .lng = -159.155546799103 * DEG_TO_RAD},
                          {.lat = -11.073667682726 * DEG_TO_RAD,
                           .lng = -154.999100885587 * DEG_TO_RAD},
                          {.lat = 73.688809451961 * DEG_TO_RAD,
                           .lng = -161.574247136150 * DEG_TO_RAD},
                          {.lat = -79.396090284140 * DEG_TO_RAD,
                           .lng = 63.249426974972 * DEG_TO_RAD}};
        GeoLoop loop = {.numVerts = 4, .verts = verts};
        GeoPolygon polygon = {.geoloop = loop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));
        t_assert(poly->geoloop.hasHemispherePole, "hemisphere pole is stored");

        Vec3d expectedPole;
        latLngToVec3(&verts[1], &expectedPole);
        t_assert(vec3Dot(&poly->geoloop.hemispherePole, &expectedPole) > 0.9999,
                 "fallback pole matches the vertex candidate");

        geodesicPolygonDestroy(poly);
    }

    TEST(hemispherePoleFallsBackToOppositeCrossCandidate) {
        // This loop is crafted so centroid and vertex candidates are rejected,
        // and the accepted pole comes from -cross(v0, v1).
        LatLng verts[] = {{.lat = -39.222488834958 * DEG_TO_RAD,
                           .lng = 17.278671407850 * DEG_TO_RAD},
                          {.lat = 77.709767848239 * DEG_TO_RAD,
                           .lng = -177.944713397859 * DEG_TO_RAD},
                          {.lat = 48.221389544616 * DEG_TO_RAD,
                           .lng = 115.374928293173 * DEG_TO_RAD},
                          {.lat = 65.650528740421 * DEG_TO_RAD,
                           .lng = 86.581228259951 * DEG_TO_RAD}};
        GeoLoop loop = {.numVerts = 4, .verts = verts};
        GeoPolygon polygon = {.geoloop = loop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&polygon, &poly));
        t_assert(poly->geoloop.hasHemispherePole, "hemisphere pole is stored");

        Vec3d v0, v1, expectedPole;
        latLngToVec3(&verts[0], &v0);
        latLngToVec3(&verts[1], &v1);
        vec3Cross(&v0, &v1, &expectedPole);
        expectedPole.x = -expectedPole.x;
        expectedPole.y = -expectedPole.y;
        expectedPole.z = -expectedPole.z;
        vec3Normalize(&expectedPole);

        t_assert(vec3Dot(&poly->geoloop.hemispherePole, &expectedPole) > 0.9999,
                 "fallback pole matches opposite cross-product candidate");

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

    TEST(largePolygonGlobalCoverage) {
        // Polar cap at 30 deg N with 8 equally spaced vertices.
        // This covers approximately 25% of the sphere (the northern cap above
        // the geodesic boundary).  With approximately uniform sampling of the
        // whole globe, expect ~25% of points inside.  Assert <=50% to catch
        // the case where the algorithm misidentifies the complement region
        // (75%) as the interior.
        LatLng capVerts[] = {
            {.lat = 30.0 * DEG_TO_RAD, .lng = 0.0},
            {.lat = 30.0 * DEG_TO_RAD, .lng = 45.0 * DEG_TO_RAD},
            {.lat = 30.0 * DEG_TO_RAD, .lng = 90.0 * DEG_TO_RAD},
            {.lat = 30.0 * DEG_TO_RAD, .lng = 135.0 * DEG_TO_RAD},
            {.lat = 30.0 * DEG_TO_RAD, .lng = 180.0 * DEG_TO_RAD},
            {.lat = 30.0 * DEG_TO_RAD, .lng = -135.0 * DEG_TO_RAD},
            {.lat = 30.0 * DEG_TO_RAD, .lng = -90.0 * DEG_TO_RAD},
            {.lat = 30.0 * DEG_TO_RAD, .lng = -45.0 * DEG_TO_RAD}};
        GeoLoop capLoop = {.numVerts = 8, .verts = capVerts};
        GeoPolygon capPolygon = {
            .geoloop = capLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&capPolygon, &poly));

        // Sanity: north pole must be inside, south pole outside.
        LatLng northPole = {.lat = M_PI_2, .lng = 0.0};
        Vec3d northVec;
        latLngToVec3(&northPole, &northVec);
        t_assert(geodesicPolygonContainsPoint(poly, &northVec),
                 "north pole is inside cap polygon");

        LatLng southPole = {.lat = -M_PI_2, .lng = 0.0};
        Vec3d southVec;
        latLngToVec3(&southPole, &southVec);
        t_assert(!geodesicPolygonContainsPoint(poly, &southVec),
                 "south pole is outside cap polygon");

        // Sample points approximately uniformly on the sphere.
        // Rows are uniform in sin(lat) for area-preserving distribution.
        int insideCount = 0;
        int totalCount = 0;
        const int nRows = 90;
        const int nCols = 180;
        for (int i = 0; i <= nRows; i++) {
            double sinLat = -1.0 + 2.0 * i / nRows;
            double lat = asin(sinLat);
            for (int j = 0; j < nCols; j++) {
                double lng = -M_PI + 2.0 * M_PI * j / nCols;
                LatLng ll = {.lat = lat, .lng = lng};
                Vec3d vec;
                latLngToVec3(&ll, &vec);
                if (geodesicPolygonContainsPoint(poly, &vec)) {
                    insideCount++;
                }
                totalCount++;
            }
        }

        // Polygon covers ~25% of the sphere.  Use generous <=50% bound.
        double fraction = (double)insideCount / totalCount;
        t_assert(fraction <= 0.30,
                 "cap polygon contains at most 30%% of globe sample points");
        // Ensure some meaningful fraction is inside (not all-reject).
        t_assert(fraction > 0.20,
                 "cap polygon contains at least 20%% of globe sample points");

        geodesicPolygonDestroy(poly);
    }

    TEST(nearHemisphericPolygonGlobalCoverage) {
        // Polar cap at 10 deg N with 12 equally spaced vertices.
        // Covers approximately 41% of the sphere
        LatLng capVerts[12];
        for (int i = 0; i < 12; i++) {
            capVerts[i].lat = 10.0 * DEG_TO_RAD;
            capVerts[i].lng = (i * 30.0) * DEG_TO_RAD;
        }
        GeoLoop capLoop = {.numVerts = 12, .verts = capVerts};
        GeoPolygon capPolygon = {
            .geoloop = capLoop, .numHoles = 0, .holes = NULL};

        GeodesicPolygon *poly = NULL;
        t_assertSuccess(geodesicPolygonCreate(&capPolygon, &poly));

        // Sanity: north pole inside, equatorial point outside.
        LatLng northPole = {.lat = M_PI_2, .lng = 0.0};
        Vec3d northVec;
        latLngToVec3(&northPole, &northVec);
        t_assert(geodesicPolygonContainsPoint(poly, &northVec),
                 "north pole is inside near-hemispheric cap");

        LatLng equatorPt = {.lat = 0.0, .lng = 0.0};
        Vec3d equatorVec;
        latLngToVec3(&equatorPt, &equatorVec);
        t_assert(!geodesicPolygonContainsPoint(poly, &equatorVec),
                 "equator point is outside near-hemispheric cap");

        // Uniform spherical sampling
        int insideCount = 0;
        int totalCount = 0;
        const int nRows = 90;
        const int nCols = 180;
        for (int i = 0; i <= nRows; i++) {
            double sinLat = -1.0 + 2.0 * i / nRows;
            double lat = asin(sinLat);
            for (int j = 0; j < nCols; j++) {
                double lng = -M_PI + 2.0 * M_PI * j / nCols;
                LatLng ll = {.lat = lat, .lng = lng};
                Vec3d vec;
                latLngToVec3(&ll, &vec);
                if (geodesicPolygonContainsPoint(poly, &vec)) {
                    insideCount++;
                }
                totalCount++;
            }
        }

        double fraction = (double)insideCount / totalCount;
        t_assert(fraction <= 0.45,
                 "near-hemispheric cap contains at most 45%% of globe sample "
                 "points");
        t_assert(fraction > 0.35,
                 "near-hemispheric cap contains at least 35%% of globe sample "
                 "points");

        geodesicPolygonDestroy(poly);
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
