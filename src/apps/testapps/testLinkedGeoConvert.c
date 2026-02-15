/*
 * Copyright 2026 Uber Technologies, Inc.
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

#include "h3api.h"
#include "linkedGeo.h"
#include "test.h"
#include "utility.h"

static void assertSameLatLng(const LatLng a, const LatLng b) {
    t_assert(a.lat == b.lat, "lat matches");
    t_assert(a.lng == b.lng, "lng matches");
}

// Expects vertices in the same order in both loops
static void assertSameLoop(const LinkedGeoLoop *ll, const GeoLoop *gl) {
    t_assert(countLinkedCoords(ll) == gl->numVerts, "vert count matches");
    LinkedLatLng *coord = ll->first;
    for (int i = 0; i < gl->numVerts; i++) {
        t_assert(coord != NULL, "coord exists");
        assertSameLatLng(coord->vertex, gl->verts[i]);
        coord = coord->next;
    }
}

// Assumes outer loop and holes in same order
static void assertSamePoly(const LinkedGeoPolygon *lp, const GeoPolygon *gp) {
    int expectedLoops = 1 + gp->numHoles;
    t_assert(countLinkedLoops(lp) == expectedLoops, "loop count matches");

    assertSameLoop(lp->first, &gp->geoloop);

    const LinkedGeoLoop *ll = lp->first->next;
    for (int h = 0; h < gp->numHoles; h++) {
        assertSameLoop(ll, &gp->holes[h]);
        ll = ll->next;
    }
}

// Assumes polygons listed in same order
static void assertSameMultiPoly(const LinkedGeoPolygon *linked,
                                const GeoMultiPolygon *mpoly) {
    t_assert(countLinkedPolygons(linked) == mpoly->numPolygons,
             "polygon count matches");

    const LinkedGeoPolygon *lp = linked;
    for (int p = 0; p < mpoly->numPolygons; p++) {
        assertSamePoly(lp, &mpoly->polygons[p]);
        lp = lp->next;
    }
}

SUITE(linkedGeoConvert) {
    TEST(geoMultiPolygonToLinkedAndBack) {
        // Two polygons: one with 1 hole, and one with no holes.
        H3Index cells[] = {
            0x8027fffffffffff, 0x802bfffffffffff, 0x804dfffffffffff,
            0x8067fffffffffff, 0x806dfffffffffff, 0x8049fffffffffff,
            0x8055fffffffffff,
        };
        GeoMultiPolygon mpoly, mpoly2;
        LinkedGeoPolygon lpoly;

        t_assertSuccess(
            H3_EXPORT(cellsToMultiPolygon)(cells, ARRAY_SIZE(cells), &mpoly));
        t_assert(mpoly.numPolygons == 2, "has two polygons");

        t_assertSuccess(
            H3_EXPORT(geoMultiPolygonToLinkedGeoPolygon)(&mpoly, &lpoly));
        t_assertSuccess(
            H3_EXPORT(linkedGeoPolygonToGeoMultiPolygon)(&lpoly, &mpoly2));

        assertSameMultiPoly(&lpoly, &mpoly);
        assertSameMultiPoly(&lpoly, &mpoly2);

        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);
        H3_EXPORT(destroyGeoMultiPolygon)(&mpoly2);
        H3_EXPORT(destroyLinkedMultiPolygon)(&lpoly);
    }

    TEST(linkedToGeoMultiPolygonRejectsTooFewVerts) {
        LatLng v1 = {0, 0};
        LatLng v2 = {1, 0};

        // A loop with only 2 vertices should be rejected
        LinkedGeoPolygon poly1 = {0};
        LinkedGeoLoop *loop1 = addNewLinkedLoop(&poly1);
        addLinkedCoord(loop1, &v1);
        addLinkedCoord(loop1, &v2);

        GeoMultiPolygon mpoly;
        t_assert(H3_EXPORT(linkedGeoPolygonToGeoMultiPolygon)(&poly1, &mpoly) ==
                     E_FAILED,
                 "rejects loop with < 3 verts");

        H3_EXPORT(destroyLinkedMultiPolygon)(&poly1);
    }

    TEST(linkedToGeoMultiPolygonRejectsEmptyPolygon) {
        // A polygon node with no loops is rejected (not the same as
        // the empty-chain case, which has no next pointer either)
        LinkedGeoPolygon poly1 = {0};
        addNewLinkedPolygon(&poly1);  // empty node with no loops

        GeoMultiPolygon mpoly;
        t_assert(H3_EXPORT(linkedGeoPolygonToGeoMultiPolygon)(&poly1, &mpoly) ==
                     E_FAILED,
                 "rejects empty polygon node");

        H3_EXPORT(destroyLinkedMultiPolygon)(&poly1);
    }

    TEST(geoToLinkedMultiPolygonRejectsTooFewVerts) {
        LatLng verts[] = {{0, 0}, {1, 0}};
        GeoPolygon poly = {.geoloop = {.numVerts = 2, .verts = verts}};
        GeoMultiPolygon mpoly = {.numPolygons = 1, .polygons = &poly};

        LinkedGeoPolygon out;
        t_assert(H3_EXPORT(geoMultiPolygonToLinkedGeoPolygon)(&mpoly, &out) ==
                     E_FAILED,
                 "rejects geoloop with < 3 verts");
    }

    TEST(linkedToGeoMultiPolygonEmpty) {
        LinkedGeoPolygon empty = {0};
        GeoMultiPolygon mpoly;
        t_assertSuccess(
            H3_EXPORT(linkedGeoPolygonToGeoMultiPolygon)(&empty, &mpoly));
        t_assert(mpoly.numPolygons == 0, "0 polygons for empty input");
        t_assert(mpoly.polygons == NULL, "NULL polygons for empty input");
    }

    TEST(geoToLinkedMultiPolygonEmpty) {
        GeoMultiPolygon mpoly = {.numPolygons = 0, .polygons = NULL};
        LinkedGeoPolygon out;
        t_assertSuccess(
            H3_EXPORT(geoMultiPolygonToLinkedGeoPolygon)(&mpoly, &out));
        t_assert(out.first == NULL, "empty linked polygon");
        t_assert(out.next == NULL, "no next polygon");
    }
}
