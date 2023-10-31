/*
 * Copyright 2017-2021 Uber Technologies, Inc.
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

#include <assert.h>
#include <stdlib.h>

#include "bbox.h"
#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "linkedGeo.h"
#include "polygon.h"
#include "test.h"

// Fixtures
static LatLng sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};

static void createLinkedLoop(LinkedGeoLoop *loop, LatLng *verts, int numVerts) {
    *loop = (LinkedGeoLoop){0};
    for (int i = 0; i < numVerts; i++) {
        addLinkedCoord(loop, verts++);
    }
}

SUITE(polygonInternal) {
    TEST(pointInsideGeoLoop) {
        GeoLoop geoloop = {.numVerts = 6, .verts = sfVerts};

        LatLng inside = {0.659, -2.136};
        LatLng somewhere = {1, 2};

        BBox bbox;
        bboxFromGeoLoop(&geoloop, &bbox);

        // For exact points on the polygon, we bias west and south
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &sfVerts[0]),
                 "does not contain exact vertex 0");
        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &sfVerts[3]),
                 "contains exact vertex 3");

        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &inside),
                 "contains point inside");
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &somewhere),
                 "contains somewhere else");
    }

    TEST(pointInsideGeoLoopCornerCases) {
        LatLng verts[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        BBox bbox;
        bboxFromGeoLoop(&geoloop, &bbox);

        LatLng point = {0, 0};

        // Test corners. For exact points on the polygon, we bias west and
        // north, so only the southeast corner is contained.
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "does not contain sw corner");
        point.lat = 1;
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "does not contain nw corner ");
        point.lng = 1;
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "does not contain ne corner ");
        point.lat = 0;
        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "contains se corner ");
    }

    TEST(pointInsideGeoLoopEdgeCases) {
        LatLng verts[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        BBox bbox;
        bboxFromGeoLoop(&geoloop, &bbox);

        LatLng point;

        // Test edges. Only points on south and east edges are contained.
        point.lat = 0.5;
        point.lng = 0;
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "does not contain point on west edge");
        point.lat = 1;
        point.lng = 0.5;
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "does not contain point on north edge");
        point.lat = 0.5;
        point.lng = 1;
        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "contains point on east edge");
        point.lat = 0;
        point.lng = 0.5;
        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "contains point on south edge");
    }

    TEST(pointInsideGeoLoopExtraEdgeCase) {
        // This is a carefully crafted shape + point to hit an otherwise
        // missed branch in coverage
        LatLng verts[] = {{0, 0}, {1, 0.5}, {0, 1}};
        GeoLoop geoloop = {.numVerts = 3, .verts = verts};

        BBox bbox;
        bboxFromGeoLoop(&geoloop, &bbox);

        LatLng point = {0.5, 0.5};
        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &point),
                 "contains inside point matching longitude of a vertex");
    }

    TEST(pointInsideGeoLoopTransmeridian) {
        LatLng verts[] = {{0.01, -M_PI + 0.01},
                          {0.01, M_PI - 0.01},
                          {-0.01, M_PI - 0.01},
                          {-0.01, -M_PI + 0.01}};
        GeoLoop transMeridianGeoLoop = {.numVerts = 4, .verts = verts};

        LatLng eastPoint = {0.001, -M_PI + 0.001};
        LatLng eastPointOutside = {0.001, -M_PI + 0.1};
        LatLng westPoint = {0.001, M_PI - 0.001};
        LatLng westPointOutside = {0.001, M_PI - 0.1};

        BBox bbox;
        bboxFromGeoLoop(&transMeridianGeoLoop, &bbox);

        t_assert(pointInsideGeoLoop(&transMeridianGeoLoop, &bbox, &westPoint),
                 "contains point to the west of the antimeridian");
        t_assert(pointInsideGeoLoop(&transMeridianGeoLoop, &bbox, &eastPoint),
                 "contains point to the east of the antimeridian");
        t_assert(
            !pointInsideGeoLoop(&transMeridianGeoLoop, &bbox,
                                &westPointOutside),
            "does not contain outside point to the west of the antimeridian");
        t_assert(
            !pointInsideGeoLoop(&transMeridianGeoLoop, &bbox,
                                &eastPointOutside),
            "does not contain outside point to the east of the antimeridian");
    }

    TEST(pointInsideLinkedGeoLoop) {
        LatLng somewhere = {1, 2};
        LatLng inside = {0.659, -2.136};

        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &sfVerts[0], 6);

        BBox bbox;
        bboxFromLinkedGeoLoop(&loop, &bbox);

        t_assert(pointInsideLinkedGeoLoop(&loop, &bbox, &inside),
                 "contains exact4");
        t_assert(!pointInsideLinkedGeoLoop(&loop, &bbox, &somewhere),
                 "contains somewhere else");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(bboxFromGeoLoop) {
        LatLng verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        const BBox expected = {1.1, 0.7, 0.7, 0.2};

        BBox result;
        bboxFromGeoLoop(&geoloop, &result);
        t_assert(bboxEquals(&result, &expected), "Got expected bbox");
    }

    TEST(bboxFromGeoLoopTransmeridian) {
        LatLng verts[] = {{0.1, -M_PI + 0.1},  {0.1, M_PI - 0.1},
                          {0.05, M_PI - 0.2},  {-0.1, M_PI - 0.1},
                          {-0.1, -M_PI + 0.1}, {-0.05, -M_PI + 0.2}};
        GeoLoop geoloop = {.numVerts = 6, .verts = verts};

        const BBox expected = {0.1, -0.1, -M_PI + 0.2, M_PI - 0.2};

        BBox result;
        bboxFromGeoLoop(&geoloop, &result);
        t_assert(bboxEquals(&result, &expected),
                 "Got expected transmeridian bbox");
    }

    TEST(bboxFromGeoLoopNoVertices) {
        GeoLoop geoloop;
        geoloop.verts = NULL;
        geoloop.numVerts = 0;

        const BBox expected = {0.0, 0.0, 0.0, 0.0};

        BBox result;
        bboxFromGeoLoop(&geoloop, &result);

        t_assert(bboxEquals(&result, &expected), "Got expected bbox");
    }

    TEST(bboxesFromGeoPolygon) {
        LatLng verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};

        const BBox expected = {1.1, 0.7, 0.7, 0.2};

        BBox *result = calloc(sizeof(BBox), 1);
        bboxesFromGeoPolygon(&polygon, result);
        t_assert(bboxEquals(&result[0], &expected), "Got expected bbox");

        free(result);
    }

    TEST(bboxesFromGeoPolygonHole) {
        LatLng verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        // not a real hole, but doesn't matter for the test
        LatLng holeVerts[] = {{0.9, 0.3}, {0.9, 0.5}, {1.0, 0.7}, {0.9, 0.3}};
        GeoLoop holeGeoLoop = {.numVerts = 4, .verts = holeVerts};

        GeoPolygon polygon = {
            .geoloop = geoloop, .numHoles = 1, .holes = &holeGeoLoop};

        const BBox expected = {1.1, 0.7, 0.7, 0.2};
        const BBox expectedHole = {1.0, 0.9, 0.7, 0.3};

        BBox *result = calloc(sizeof(BBox), 2);
        bboxesFromGeoPolygon(&polygon, result);
        t_assert(bboxEquals(&result[0], &expected), "Got expected bbox");
        t_assert(bboxEquals(&result[1], &expectedHole),
                 "Got expected hole bbox");

        free(result);
    }

    TEST(bboxFromLinkedGeoLoop) {
        LatLng verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};

        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &verts[0], 4);

        const BBox expected = {1.1, 0.7, 0.7, 0.2};

        BBox result;
        bboxFromLinkedGeoLoop(&loop, &result);
        t_assert(bboxEquals(&result, &expected), "Got expected bbox");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(bboxFromLinkedGeoLoopNoVertices) {
        LinkedGeoLoop loop = {0};

        const BBox expected = {0.0, 0.0, 0.0, 0.0};

        BBox result;
        bboxFromLinkedGeoLoop(&loop, &result);

        t_assert(bboxEquals(&result, &expected), "Got expected bbox");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(isClockwiseGeoLoop) {
        LatLng verts[] = {{0, 0}, {0.1, 0.1}, {0, 0.1}};
        GeoLoop geoloop = {.numVerts = 3, .verts = verts};

        t_assert(isClockwiseGeoLoop(&geoloop),
                 "Got true for clockwise geoloop");
    }

    TEST(isClockwiseLinkedGeoLoop) {
        LatLng verts[] = {{0.1, 0.1}, {0.2, 0.2}, {0.1, 0.2}};
        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &verts[0], 3);

        t_assert(isClockwiseLinkedGeoLoop(&loop),
                 "Got true for clockwise loop");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(isNotClockwiseLinkedGeoLoop) {
        LatLng verts[] = {{0, 0}, {0, 0.4}, {0.4, 0.4}, {0.4, 0}};
        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &verts[0], 4);

        t_assert(!isClockwiseLinkedGeoLoop(&loop),
                 "Got false for counter-clockwise loop");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(isClockwiseGeoLoopTransmeridian) {
        LatLng verts[] = {{0.4, M_PI - 0.1},
                          {0.4, -M_PI + 0.1},
                          {-0.4, -M_PI + 0.1},
                          {-0.4, M_PI - 0.1}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        t_assert(isClockwiseGeoLoop(&geoloop),
                 "Got true for clockwise geoloop");
    }

    TEST(isClockwiseLinkedGeoLoopTransmeridian) {
        LatLng verts[] = {{0.4, M_PI - 0.1},
                          {0.4, -M_PI + 0.1},
                          {-0.4, -M_PI + 0.1},
                          {-0.4, M_PI - 0.1}};
        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &verts[0], 4);

        t_assert(isClockwiseLinkedGeoLoop(&loop),
                 "Got true for clockwise transmeridian loop");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(isNotClockwiseLinkedGeoLoopTransmeridian) {
        LatLng verts[] = {{0.4, M_PI - 0.1},
                          {-0.4, M_PI - 0.1},
                          {-0.4, -M_PI + 0.1},
                          {0.4, -M_PI + 0.1}};
        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &verts[0], 4);

        t_assert(!isClockwiseLinkedGeoLoop(&loop),
                 "Got false for counter-clockwise transmeridian loop");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(normalizeMultiPolygonSingle) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}};

        LinkedGeoLoop *outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer);

        t_assertSuccess(normalizeMultiPolygon(&polygon));

        t_assert(countLinkedPolygons(&polygon) == 1, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 1, "Loop count correct");
        t_assert(polygon.first == outer, "Got expected loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonTwoOuterLoops) {
        LatLng verts1[] = {{0, 0}, {0, 1}, {1, 1}};

        LinkedGeoLoop *outer1 = malloc(sizeof(*outer1));
        assert(outer1 != NULL);
        createLinkedLoop(outer1, &verts1[0], 3);

        LatLng verts2[] = {{2, 2}, {2, 3}, {3, 3}};

        LinkedGeoLoop *outer2 = malloc(sizeof(*outer2));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer1);
        addLinkedLoop(&polygon, outer2);

        t_assertSuccess(normalizeMultiPolygon(&polygon));

        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 1,
                 "Loop count on first polygon correct");
        t_assert(countLinkedLoops(polygon.next) == 1,
                 "Loop count on second polygon correct");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonOneHole) {
        LatLng verts[] = {{0, 0}, {0, 3}, {3, 3}, {3, 0}};

        LinkedGeoLoop *outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        LatLng verts2[] = {{1, 1}, {2, 2}, {1, 2}};

        LinkedGeoLoop *inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outer);

        t_assertSuccess(normalizeMultiPolygon(&polygon));

        t_assert(countLinkedPolygons(&polygon) == 1, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 2,
                 "Loop count on first polygon correct");
        t_assert(polygon.first == outer, "Got expected outer loop");
        t_assert(polygon.first->next == inner, "Got expected inner loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonTwoHoles) {
        LatLng verts[] = {{0, 0}, {0, 0.4}, {0.4, 0.4}, {0.4, 0}};

        LinkedGeoLoop *outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        LatLng verts2[] = {{0.1, 0.1}, {0.2, 0.2}, {0.1, 0.2}};

        LinkedGeoLoop *inner1 = malloc(sizeof(*inner1));
        assert(inner1 != NULL);
        createLinkedLoop(inner1, &verts2[0], 3);

        LatLng verts3[] = {{0.2, 0.2}, {0.3, 0.3}, {0.2, 0.3}};

        LinkedGeoLoop *inner2 = malloc(sizeof(*inner2));
        assert(inner2 != NULL);
        createLinkedLoop(inner2, &verts3[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner2);
        addLinkedLoop(&polygon, outer);
        addLinkedLoop(&polygon, inner1);

        t_assertSuccess(normalizeMultiPolygon(&polygon));

        t_assert(countLinkedPolygons(&polygon) == 1,
                 "Polygon count correct for 2 holes");
        t_assert(polygon.first == outer, "Got expected outer loop");
        t_assert(countLinkedLoops(&polygon) == 3,
                 "Loop count on first polygon correct");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonTwoDonuts) {
        LatLng verts[] = {{0, 0}, {0, 3}, {3, 3}, {3, 0}};
        LinkedGeoLoop *outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        LatLng verts2[] = {{1, 1}, {2, 2}, {1, 2}};
        LinkedGeoLoop *inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 3);

        LatLng verts3[] = {{0, 0}, {0, -3}, {-3, -3}, {-3, 0}};
        LinkedGeoLoop *outer2 = malloc(sizeof(*outer));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts3[0], 4);

        LatLng verts4[] = {{-1, -1}, {-2, -2}, {-1, -2}};
        LinkedGeoLoop *inner2 = malloc(sizeof(*inner));
        assert(inner2 != NULL);
        createLinkedLoop(inner2, &verts4[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner2);
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outer);
        addLinkedLoop(&polygon, outer2);

        t_assertSuccess(normalizeMultiPolygon(&polygon));

        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 2,
                 "Loop count on first polygon correct");
        t_assert(countLinkedCoords(polygon.first) == 4,
                 "Got expected outer loop");
        t_assert(countLinkedCoords(polygon.first->next) == 3,
                 "Got expected inner loop");
        t_assert(countLinkedLoops(polygon.next) == 2,
                 "Loop count on second polygon correct");
        t_assert(countLinkedCoords(polygon.next->first) == 4,
                 "Got expected outer loop");
        t_assert(countLinkedCoords(polygon.next->first->next) == 3,
                 "Got expected inner loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonNestedDonuts) {
        LatLng verts[] = {{0.2, 0.2}, {0.2, -0.2}, {-0.2, -0.2}, {-0.2, 0.2}};
        LinkedGeoLoop *outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        LatLng verts2[] = {{0.1, 0.1}, {-0.1, 0.1}, {-0.1, -0.1}, {0.1, -0.1}};
        LinkedGeoLoop *inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 4);

        LatLng verts3[] = {{0.6, 0.6}, {0.6, -0.6}, {-0.6, -0.6}, {-0.6, 0.6}};
        LinkedGeoLoop *outerBig = malloc(sizeof(*outerBig));
        assert(outerBig != NULL);
        createLinkedLoop(outerBig, &verts3[0], 4);

        LatLng verts4[] = {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}};
        LinkedGeoLoop *innerBig = malloc(sizeof(*innerBig));
        assert(innerBig != NULL);
        createLinkedLoop(innerBig, &verts4[0], 4);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outerBig);
        addLinkedLoop(&polygon, innerBig);
        addLinkedLoop(&polygon, outer);

        t_assertSuccess(normalizeMultiPolygon(&polygon));

        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 2,
                 "Loop count on first polygon correct");
        t_assert(polygon.first == outerBig, "Got expected outer loop");
        t_assert(polygon.first->next == innerBig, "Got expected inner loop");
        t_assert(countLinkedLoops(polygon.next) == 2,
                 "Loop count on second polygon correct");
        t_assert(polygon.next->first == outer, "Got expected outer loop");
        t_assert(polygon.next->first->next == inner, "Got expected inner loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonNoOuterLoops) {
        LatLng verts1[] = {{0, 0}, {1, 1}, {0, 1}};

        LinkedGeoLoop *outer1 = malloc(sizeof(*outer1));
        assert(outer1 != NULL);
        createLinkedLoop(outer1, &verts1[0], 3);

        LatLng verts2[] = {{2, 2}, {3, 3}, {2, 3}};

        LinkedGeoLoop *outer2 = malloc(sizeof(*outer2));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer1);
        addLinkedLoop(&polygon, outer2);

        t_assert(normalizeMultiPolygon(&polygon) == E_FAILED,
                 "Expected error code returned");

        t_assert(countLinkedPolygons(&polygon) == 1, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 0,
                 "Loop count as expected with invalid input");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonAlreadyNormalized) {
        LatLng verts1[] = {{0, 0}, {0, 1}, {1, 1}};

        LinkedGeoLoop *outer1 = malloc(sizeof(*outer1));
        assert(outer1 != NULL);
        createLinkedLoop(outer1, &verts1[0], 3);

        LatLng verts2[] = {{2, 2}, {2, 3}, {3, 3}};

        LinkedGeoLoop *outer2 = malloc(sizeof(*outer2));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer1);
        LinkedGeoPolygon *next = addNewLinkedPolygon(&polygon);
        addLinkedLoop(next, outer2);

        // Should be a no-op
        t_assert(normalizeMultiPolygon(&polygon) == E_FAILED,
                 "Expected error code returned");

        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 1,
                 "Loop count on first polygon correct");
        t_assert(polygon.first == outer1, "Got expected outer loop");
        t_assert(countLinkedLoops(polygon.next) == 1,
                 "Loop count on second polygon correct");
        t_assert(polygon.next->first == outer2, "Got expected outer loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygon_unassignedHole) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};

        LinkedGeoLoop *outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        LatLng verts2[] = {{2, 2}, {3, 3}, {2, 3}};

        LinkedGeoLoop *inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outer);

        t_assert(normalizeMultiPolygon(&polygon) == E_FAILED,
                 "Expected error code returned");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(lineIntersectsLine) {
        LatLng lines1[4] = {{0, 0}, {1, 1}, {0, 1}, {1, 0}};
        t_assert(
            lineIntersectsLine(&lines1[0], &lines1[1], &lines1[2], &lines1[3]),
            "diagonal intersection");

        LatLng lines2[4] = {{1, 1}, {0, 0}, {1, 0}, {0, 1}};
        t_assert(
            lineIntersectsLine(&lines2[0], &lines2[1], &lines2[2], &lines2[3]),
            "diagonal intersection, reverse vertexes");

        LatLng lines3[4] = {{0.5, 0}, {0.5, 1}, {0, 0.5}, {1, 0.5}};
        t_assert(
            lineIntersectsLine(&lines3[0], &lines3[1], &lines3[2], &lines3[3]),
            "horizontal/vertical intersection");

        LatLng lines4[4] = {{0.5, 1}, {0.5, 0}, {1, 0.5}, {0, 0.5}};
        t_assert(
            lineIntersectsLine(&lines4[0], &lines4[1], &lines4[2], &lines4[3]),
            "horizontal/vertical intersection, reverse vertexes");

        LatLng lines5[4] = {{0, 0}, {0.4, 0.4}, {0, 1}, {1, 0}};
        t_assert(
            !lineIntersectsLine(&lines5[0], &lines5[1], &lines5[2], &lines5[3]),
            "diagonal non-intersection, below");

        LatLng lines6[4] = {{0.6, 0.6}, {1, 1}, {0, 1}, {1, 0}};
        t_assert(
            !lineIntersectsLine(&lines6[0], &lines6[1], &lines6[2], &lines6[3]),
            "diagonal non-intersection, above");

        LatLng lines7[4] = {{0.5, 0}, {0.5, 1}, {0, 0.5}, {0.4, 0.5}};
        t_assert(
            !lineIntersectsLine(&lines7[0], &lines7[1], &lines7[2], &lines7[3]),
            "horizontal/vertical non-intersection, below");

        LatLng lines8[4] = {{0.5, 0}, {0.5, 1}, {0.6, 0.5}, {1, 0.5}};
        t_assert(
            !lineIntersectsLine(&lines8[0], &lines8[1], &lines8[2], &lines8[3]),
            "horizontal/vertical non-intersection, above");

        LatLng lines9[4] = {{0.5, 0}, {0.5, 0.4}, {0, 0.5}, {1, 0.5}};
        t_assert(
            !lineIntersectsLine(&lines9[0], &lines9[1], &lines9[2], &lines9[3]),
            "horizontal/vertical non-intersection, left");

        LatLng lines10[4] = {{0.5, 0.6}, {0.5, 1}, {0, 0.5}, {1, 0.5}};
        t_assert(!lineIntersectsLine(&lines10[0], &lines10[1], &lines10[2],
                                     &lines10[3]),
                 "horizontal/vertical non-intersection, right");
    }

    TEST(cellBoundaryInsidePolygon_inside) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};
        BBox *bboxes = calloc(sizeof(BBox), 1);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {
            .numVerts = 4,
            .verts = {{0.6, 0.6}, {0.6, 0.4}, {0.4, 0.4}, {0.4, 0.6}}};
        BBox boundaryBBox = {0.6, 0.4, 0.6, 0.4};

        t_assert(cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                           &boundaryBBox),
                 "simple containment is inside");
        free(bboxes);
    }

    TEST(cellBoundaryInsidePolygon_insideTransmeridianWest) {
        LatLng verts[] = {{0, M_PI - 0.5},
                          {0, -M_PI + 0.5},
                          {1, -M_PI + 0.5},
                          {1, M_PI - 0.5}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};
        BBox *bboxes = calloc(sizeof(BBox), 1);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {.numVerts = 4,
                                 .verts = {{0.6, M_PI - 0.1},
                                           {0.6, M_PI - 0.2},
                                           {0.4, M_PI - 0.2},
                                           {0.4, M_PI - 0.1}}};
        BBox boundaryBBox = {0.6, 0.4, 0.6, 0.4};

        t_assert(cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                           &boundaryBBox),
                 "simple containment is inside, west side of transmeridian");
        free(bboxes);
    }

    TEST(cellBoundaryInsidePolygon_insideTransmeridianEast) {
        LatLng verts[] = {{0, M_PI - 0.5},
                          {0, -M_PI + 0.5},
                          {1, -M_PI + 0.5},
                          {1, M_PI - 0.5}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};
        BBox *bboxes = calloc(sizeof(BBox), 1);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {.numVerts = 4,
                                 .verts = {{0.6, -M_PI + 0.4},
                                           {0.6, -M_PI + 0.2},
                                           {0.4, -M_PI + 0.2},
                                           {0.4, -M_PI + 0.4}}};
        BBox boundaryBBox = {0.6, 0.4, 0.6, 0.4};

        t_assert(cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                           &boundaryBBox),
                 "simple containment is inside, east side of transmeridian");
        free(bboxes);
    }

    TEST(cellBoundaryInsidePolygon_insideWithHole) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        LatLng holeVerts[] = {{0.3, 0.3}, {0.3, 0.1}, {0.1, 0.1}, {0.1, 0.3}};
        GeoLoop holeGeoLoop = {.numVerts = 4, .verts = holeVerts};

        GeoPolygon polygon = {
            .geoloop = geoloop, .numHoles = 1, .holes = &holeGeoLoop};

        BBox *bboxes = calloc(sizeof(BBox), 2);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {
            .numVerts = 4,
            .verts = {{0.6, 0.6}, {0.6, 0.4}, {0.4, 0.4}, {0.4, 0.6}}};
        BBox boundaryBBox = {0.6, 0.4, 0.6, 0.4};

        t_assert(cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                           &boundaryBBox),
                 "simple containment is inside, with hole");
        free(bboxes);
    }

    TEST(cellBoundaryInsidePolygon_notInside) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};
        BBox *bboxes = calloc(sizeof(BBox), 1);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {
            .numVerts = 4,
            .verts = {{1.6, 1.6}, {1.6, 1.4}, {1.4, 1.4}, {1.4, 1.6}}};
        BBox boundaryBBox = {1.6, 1.4, 1.6, 1.4};

        t_assert(!cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                            &boundaryBBox),
                 "fully outside is not inside");
        free(bboxes);
    }

    TEST(cellBoundaryInsidePolygon_notInsideIntersect) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};
        BBox *bboxes = calloc(sizeof(BBox), 1);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {
            .numVerts = 4,
            .verts = {{0.6, 0.6}, {1.6, 0.4}, {0.4, 0.4}, {0.4, 0.6}}};
        BBox boundaryBBox = {1.6, 0.4, 0.6, 0.4};

        t_assert(!cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                            &boundaryBBox),
                 "intersecting polygon is not inside");
        free(bboxes);
    }

    TEST(cellBoundaryInsidePolygon_notInsideIntersectHole) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        LatLng holeVerts[] = {{0.3, 0.3}, {0.5, 0.5}, {0.1, 0.1}, {0.1, 0.3}};
        GeoLoop holeGeoLoop = {.numVerts = 4, .verts = holeVerts};

        GeoPolygon polygon = {
            .geoloop = geoloop, .numHoles = 1, .holes = &holeGeoLoop};

        BBox *bboxes = calloc(sizeof(BBox), 2);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {
            .numVerts = 4,
            .verts = {{0.6, 0.6}, {0.6, 0.4}, {0.4, 0.4}, {0.4, 0.6}}};
        BBox boundaryBBox = {0.6, 0.4, 0.6, 0.4};

        t_assert(!cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                            &boundaryBBox),
                 "not inside with hole intersection");
        free(bboxes);
    }

    TEST(cellBoundaryInsidePolygon_notInsideWithinHole) {
        LatLng verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        LatLng holeVerts[] = {{0.9, 0.9}, {0.9, 0.1}, {0.1, 0.1}, {0.1, 0.9}};
        GeoLoop holeGeoLoop = {.numVerts = 4, .verts = holeVerts};

        GeoPolygon polygon = {
            .geoloop = geoloop, .numHoles = 1, .holes = &holeGeoLoop};

        BBox *bboxes = calloc(sizeof(BBox), 2);
        bboxesFromGeoPolygon(&polygon, bboxes);

        CellBoundary boundary = {
            .numVerts = 4,
            .verts = {{0.6, 0.6}, {0.6, 0.4}, {0.4, 0.4}, {0.4, 0.6}}};
        BBox boundaryBBox = {0.6, 0.4, 0.6, 0.4};

        t_assert(!cellBoundaryInsidePolygon(&polygon, bboxes, &boundary,
                                            &boundaryBBox),
                 "not inside when within hole");
        free(bboxes);
    }
}
