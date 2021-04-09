/*
 * Copyright 2017-2020 Uber Technologies, Inc.
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
#include "geoPoint.h"
#include "h3Index.h"
#include "linkedGeo.h"
#include "polygon.h"
#include "test.h"

// Fixtures
static GeoPoint sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};

static void createLinkedLoop(LinkedGeoLoop* loop, GeoPoint* verts,
                             int numVerts) {
    *loop = (LinkedGeoLoop){0};
    for (int i = 0; i < numVerts; i++) {
        addLinkedCoord(loop, verts++);
    }
}

SUITE(polygon) {
    TEST(pointInsideGeoLoop) {
        GeoLoop geoloop = {.numVerts = 6, .verts = sfVerts};

        GeoPoint inside = {0.659, -2.136};
        GeoPoint somewhere = {1, 2};

        BBox bbox;
        bboxFromGeoLoop(&geoloop, &bbox);

        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &sfVerts[0]),
                 "contains exact");
        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &sfVerts[4]),
                 "contains exact 4");
        t_assert(pointInsideGeoLoop(&geoloop, &bbox, &inside),
                 "contains point inside");
        t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &somewhere),
                 "contains somewhere else");
    }

    TEST(pointInsideGeoLoopTransmeridian) {
        GeoPoint verts[] = {{0.01, -M_PI + 0.01},
                            {0.01, M_PI - 0.01},
                            {-0.01, M_PI - 0.01},
                            {-0.01, -M_PI + 0.01}};
        GeoLoop transMeridianGeoLoop = {.numVerts = 4, .verts = verts};

        GeoPoint eastPoint = {0.001, -M_PI + 0.001};
        GeoPoint eastPointOutside = {0.001, -M_PI + 0.1};
        GeoPoint westPoint = {0.001, M_PI - 0.001};
        GeoPoint westPointOutside = {0.001, M_PI - 0.1};

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
        GeoPoint somewhere = {1, 2};
        GeoPoint inside = {0.659, -2.136};

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
        GeoPoint verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        const BBox expected = {1.1, 0.7, 0.7, 0.2};

        BBox result;
        bboxFromGeoLoop(&geoloop, &result);
        t_assert(bboxEquals(&result, &expected), "Got expected bbox");
    }

    TEST(bboxFromGeoLoopTransmeridian) {
        GeoPoint verts[] = {{0.1, -M_PI + 0.1},  {0.1, M_PI - 0.1},
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
        GeoPoint verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};

        const BBox expected = {1.1, 0.7, 0.7, 0.2};

        BBox* result = calloc(sizeof(BBox), 1);
        bboxesFromGeoPolygon(&polygon, result);
        t_assert(bboxEquals(&result[0], &expected), "Got expected bbox");

        free(result);
    }

    TEST(bboxesFromGeoPolygonHole) {
        GeoPoint verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        // not a real hole, but doesn't matter for the test
        GeoPoint holeVerts[] = {{0.9, 0.3}, {0.9, 0.5}, {1.0, 0.7}, {0.9, 0.3}};
        GeoLoop holeGeoLoop = {.numVerts = 4, .verts = holeVerts};

        GeoPolygon polygon = {
            .geoloop = geoloop, .numHoles = 1, .holes = &holeGeoLoop};

        const BBox expected = {1.1, 0.7, 0.7, 0.2};
        const BBox expectedHole = {1.0, 0.9, 0.7, 0.3};

        BBox* result = calloc(sizeof(BBox), 2);
        bboxesFromGeoPolygon(&polygon, result);
        t_assert(bboxEquals(&result[0], &expected), "Got expected bbox");
        t_assert(bboxEquals(&result[1], &expectedHole),
                 "Got expected hole bbox");

        free(result);
    }

    TEST(bboxFromLinkedGeoLoop) {
        GeoPoint verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};

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
        GeoPoint verts[] = {{0, 0}, {0.1, 0.1}, {0, 0.1}};
        GeoLoop geoloop = {.numVerts = 3, .verts = verts};

        t_assert(isClockwiseGeoLoop(&geoloop),
                 "Got true for clockwise geoloop");
    }

    TEST(isClockwiseLinkedGeoLoop) {
        GeoPoint verts[] = {{0.1, 0.1}, {0.2, 0.2}, {0.1, 0.2}};
        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &verts[0], 3);

        t_assert(isClockwiseLinkedGeoLoop(&loop),
                 "Got true for clockwise loop");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(isNotClockwiseLinkedGeoLoop) {
        GeoPoint verts[] = {{0, 0}, {0, 0.4}, {0.4, 0.4}, {0.4, 0}};
        LinkedGeoLoop loop;
        createLinkedLoop(&loop, &verts[0], 4);

        t_assert(!isClockwiseLinkedGeoLoop(&loop),
                 "Got false for counter-clockwise loop");

        destroyLinkedGeoLoop(&loop);
    }

    TEST(isClockwiseGeoLoopTransmeridian) {
        GeoPoint verts[] = {{0.4, M_PI - 0.1},
                            {0.4, -M_PI + 0.1},
                            {-0.4, -M_PI + 0.1},
                            {-0.4, M_PI - 0.1}};
        GeoLoop geoloop = {.numVerts = 4, .verts = verts};

        t_assert(isClockwiseGeoLoop(&geoloop),
                 "Got true for clockwise geoloop");
    }

    TEST(isClockwiseLinkedGeoLoopTransmeridian) {
        GeoPoint verts[] = {{0.4, M_PI - 0.1},
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
        GeoPoint verts[] = {{0.4, M_PI - 0.1},
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
        GeoPoint verts[] = {{0, 0}, {0, 1}, {1, 1}};

        LinkedGeoLoop* outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_SUCCESS, "No error code returned");

        t_assert(countLinkedPolygons(&polygon) == 1, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 1, "Loop count correct");
        t_assert(polygon.first == outer, "Got expected loop");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonTwoOuterLoops) {
        GeoPoint verts1[] = {{0, 0}, {0, 1}, {1, 1}};

        LinkedGeoLoop* outer1 = malloc(sizeof(*outer1));
        assert(outer1 != NULL);
        createLinkedLoop(outer1, &verts1[0], 3);

        GeoPoint verts2[] = {{2, 2}, {2, 3}, {3, 3}};

        LinkedGeoLoop* outer2 = malloc(sizeof(*outer2));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer1);
        addLinkedLoop(&polygon, outer2);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_SUCCESS, "No error code returned");

        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 1,
                 "Loop count on first polygon correct");
        t_assert(countLinkedLoops(polygon.next) == 1,
                 "Loop count on second polygon correct");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonOneHole) {
        GeoPoint verts[] = {{0, 0}, {0, 3}, {3, 3}, {3, 0}};

        LinkedGeoLoop* outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        GeoPoint verts2[] = {{1, 1}, {2, 2}, {1, 2}};

        LinkedGeoLoop* inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outer);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_SUCCESS, "No error code returned");

        t_assert(countLinkedPolygons(&polygon) == 1, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 2,
                 "Loop count on first polygon correct");
        t_assert(polygon.first == outer, "Got expected outer loop");
        t_assert(polygon.first->next == inner, "Got expected inner loop");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonTwoHoles) {
        GeoPoint verts[] = {{0, 0}, {0, 0.4}, {0.4, 0.4}, {0.4, 0}};

        LinkedGeoLoop* outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        GeoPoint verts2[] = {{0.1, 0.1}, {0.2, 0.2}, {0.1, 0.2}};

        LinkedGeoLoop* inner1 = malloc(sizeof(*inner1));
        assert(inner1 != NULL);
        createLinkedLoop(inner1, &verts2[0], 3);

        GeoPoint verts3[] = {{0.2, 0.2}, {0.3, 0.3}, {0.2, 0.3}};

        LinkedGeoLoop* inner2 = malloc(sizeof(*inner2));
        assert(inner2 != NULL);
        createLinkedLoop(inner2, &verts3[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner2);
        addLinkedLoop(&polygon, outer);
        addLinkedLoop(&polygon, inner1);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_SUCCESS, "No error code returned");

        t_assert(countLinkedPolygons(&polygon) == 1,
                 "Polygon count correct for 2 holes");
        t_assert(polygon.first == outer, "Got expected outer loop");
        t_assert(countLinkedLoops(&polygon) == 3,
                 "Loop count on first polygon correct");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonTwoDonuts) {
        GeoPoint verts[] = {{0, 0}, {0, 3}, {3, 3}, {3, 0}};
        LinkedGeoLoop* outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        GeoPoint verts2[] = {{1, 1}, {2, 2}, {1, 2}};
        LinkedGeoLoop* inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 3);

        GeoPoint verts3[] = {{0, 0}, {0, -3}, {-3, -3}, {-3, 0}};
        LinkedGeoLoop* outer2 = malloc(sizeof(*outer));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts3[0], 4);

        GeoPoint verts4[] = {{-1, -1}, {-2, -2}, {-1, -2}};
        LinkedGeoLoop* inner2 = malloc(sizeof(*inner));
        assert(inner2 != NULL);
        createLinkedLoop(inner2, &verts4[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner2);
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outer);
        addLinkedLoop(&polygon, outer2);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_SUCCESS, "No error code returned");

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

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonNestedDonuts) {
        GeoPoint verts[] = {{0.2, 0.2}, {0.2, -0.2}, {-0.2, -0.2}, {-0.2, 0.2}};
        LinkedGeoLoop* outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        GeoPoint verts2[] = {
            {0.1, 0.1}, {-0.1, 0.1}, {-0.1, -0.1}, {0.1, -0.1}};
        LinkedGeoLoop* inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 4);

        GeoPoint verts3[] = {
            {0.6, 0.6}, {0.6, -0.6}, {-0.6, -0.6}, {-0.6, 0.6}};
        LinkedGeoLoop* outerBig = malloc(sizeof(*outerBig));
        assert(outerBig != NULL);
        createLinkedLoop(outerBig, &verts3[0], 4);

        GeoPoint verts4[] = {
            {0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}};
        LinkedGeoLoop* innerBig = malloc(sizeof(*innerBig));
        assert(innerBig != NULL);
        createLinkedLoop(innerBig, &verts4[0], 4);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outerBig);
        addLinkedLoop(&polygon, innerBig);
        addLinkedLoop(&polygon, outer);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_SUCCESS, "No error code returned");

        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 2,
                 "Loop count on first polygon correct");
        t_assert(polygon.first == outerBig, "Got expected outer loop");
        t_assert(polygon.first->next == innerBig, "Got expected inner loop");
        t_assert(countLinkedLoops(polygon.next) == 2,
                 "Loop count on second polygon correct");
        t_assert(polygon.next->first == outer, "Got expected outer loop");
        t_assert(polygon.next->first->next == inner, "Got expected inner loop");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonNoOuterLoops) {
        GeoPoint verts1[] = {{0, 0}, {1, 1}, {0, 1}};

        LinkedGeoLoop* outer1 = malloc(sizeof(*outer1));
        assert(outer1 != NULL);
        createLinkedLoop(outer1, &verts1[0], 3);

        GeoPoint verts2[] = {{2, 2}, {3, 3}, {2, 3}};

        LinkedGeoLoop* outer2 = malloc(sizeof(*outer2));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer1);
        addLinkedLoop(&polygon, outer2);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_ERR_UNASSIGNED_HOLES,
                 "Expected error code returned");

        t_assert(countLinkedPolygons(&polygon) == 1, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 0,
                 "Loop count as expected with invalid input");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygonAlreadyNormalized) {
        GeoPoint verts1[] = {{0, 0}, {0, 1}, {1, 1}};

        LinkedGeoLoop* outer1 = malloc(sizeof(*outer1));
        assert(outer1 != NULL);
        createLinkedLoop(outer1, &verts1[0], 3);

        GeoPoint verts2[] = {{2, 2}, {2, 3}, {3, 3}};

        LinkedGeoLoop* outer2 = malloc(sizeof(*outer2));
        assert(outer2 != NULL);
        createLinkedLoop(outer2, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, outer1);
        LinkedGeoPolygon* next = addNewLinkedPolygon(&polygon);
        addLinkedLoop(next, outer2);

        // Should be a no-op
        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_ERR_MULTIPLE_POLYGONS,
                 "Expected error code returned");

        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 1,
                 "Loop count on first polygon correct");
        t_assert(polygon.first == outer1, "Got expected outer loop");
        t_assert(countLinkedLoops(polygon.next) == 1,
                 "Loop count on second polygon correct");
        t_assert(polygon.next->first == outer2, "Got expected outer loop");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }

    TEST(normalizeMultiPolygon_unassignedHole) {
        GeoPoint verts[] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};

        LinkedGeoLoop* outer = malloc(sizeof(*outer));
        assert(outer != NULL);
        createLinkedLoop(outer, &verts[0], 4);

        GeoPoint verts2[] = {{2, 2}, {3, 3}, {2, 3}};

        LinkedGeoLoop* inner = malloc(sizeof(*inner));
        assert(inner != NULL);
        createLinkedLoop(inner, &verts2[0], 3);

        LinkedGeoPolygon polygon = {0};
        addLinkedLoop(&polygon, inner);
        addLinkedLoop(&polygon, outer);

        int result = normalizeMultiPolygon(&polygon);

        t_assert(result == NORMALIZATION_ERR_UNASSIGNED_HOLES,
                 "Expected error code returned");

        H3_EXPORT(destroyLinkedPolygon)(&polygon);
    }
}
