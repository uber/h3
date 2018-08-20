/*
 * Copyright 2017 Uber Technologies, Inc.
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

#include <stdlib.h>
#include "bbox.h"
#include "constants.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "linkedGeo.h"
#include "polygon.h"
#include "test.h"

// Fixtures
GeoCoord sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};
Geofence sfGeofence;

GeoCoord primeMeridianVerts[] = {
    {0.01, 0.01}, {0.01, -0.01}, {-0.01, -0.01}, {-0.01, 0.01}};
Geofence primeMeridianGeofence;

GeoCoord transMeridianVerts[] = {{0.01, -M_PI + 0.01},
                                 {0.01, M_PI - 0.01},
                                 {-0.01, M_PI - 0.01},
                                 {-0.01, -M_PI + 0.01}};
Geofence transMeridianGeofence;

GeoCoord transMeridianHoleVerts[] = {{0.005, -M_PI + 0.005},
                                     {0.005, M_PI - 0.005},
                                     {-0.005, M_PI - 0.005},
                                     {-0.005, -M_PI + 0.005}};
Geofence transMeridianHoleGeofence;

void destroyLinkedGeoLoop(LinkedGeoLoop* loop) {
    for (LinkedGeoCoord *currentCoord = loop->first, *nextCoord;
         currentCoord != NULL; currentCoord = nextCoord) {
        nextCoord = currentCoord->next;
        free(currentCoord);
    }
}

BEGIN_TESTS(polygon);

sfGeofence.numVerts = 6;
sfGeofence.verts = sfVerts;

primeMeridianGeofence.numVerts = 4;
primeMeridianGeofence.verts = primeMeridianVerts;

transMeridianGeofence.numVerts = 4;
transMeridianGeofence.verts = transMeridianVerts;

transMeridianHoleGeofence.numVerts = 4;
transMeridianHoleGeofence.verts = transMeridianHoleVerts;

TEST(geofenceContainsPoint) {
    GeoCoord somewhere = {1, 2};

    BBox bbox;
    bboxFromGeofence(&sfGeofence, &bbox);

    t_assert(geofenceContainsPoint(&sfGeofence, &bbox, &sfVerts[0]) == false,
             "contains exact");
    t_assert(geofenceContainsPoint(&sfGeofence, &bbox, &sfVerts[4]) == true,
             "contains exact4");
    t_assert(geofenceContainsPoint(&sfGeofence, &bbox, &somewhere) == false,
             "contains somewhere else");
}

TEST(geofenceContainsPointTransmeridian) {
    GeoCoord eastPoint = {0.001, -M_PI + 0.001};
    GeoCoord eastPointOutside = {0.001, -M_PI + 0.1};
    GeoCoord westPoint = {0.001, M_PI - 0.001};
    GeoCoord westPointOutside = {0.001, M_PI - 0.1};

    BBox bbox;
    bboxFromGeofence(&transMeridianGeofence, &bbox);

    t_assert(geofenceContainsPoint(&transMeridianGeofence, &bbox, &westPoint) ==
                 true,
             "contains point to the west of the antimeridian");
    t_assert(geofenceContainsPoint(&transMeridianGeofence, &bbox, &eastPoint) ==
                 true,
             "contains point to the east of the antimeridian");
    t_assert(geofenceContainsPoint(&transMeridianGeofence, &bbox,
                                   &westPointOutside) == false,
             "does not contain outside point to the west of the antimeridian");
    t_assert(geofenceContainsPoint(&transMeridianGeofence, &bbox,
                                   &eastPointOutside) == false,
             "does not contain outside point to the east of the antimeridian");
}

TEST(linkedGeoLoopContainsPoint) {
    GeoCoord somewhere = {1, 2};
    GeoCoord inside = {0.659, -2.136};

    LinkedGeoLoop loop;
    initLinkedLoop(&loop);

    for (int i = 0; i < 6; i++) {
        addLinkedCoord(&loop, &sfVerts[i]);
    }

    BBox bbox;
    bboxFromLinkedGeoLoop(&loop, &bbox);

    t_assert(linkedGeoLoopContainsPoint(&loop, &bbox, &inside) == true,
             "contains exact4");
    t_assert(linkedGeoLoopContainsPoint(&loop, &bbox, &somewhere) == false,
             "contains somewhere else");

    destroyLinkedGeoLoop(&loop);
}

TEST(bboxFromGeofence) {
    GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};

    Geofence geofence;
    geofence.verts = verts;
    geofence.numVerts = 4;

    const BBox expected = {1.1, 0.7, 0.7, 0.2};

    BBox result;
    bboxFromGeofence(&geofence, &result);
    t_assert(bboxEquals(&result, &expected), "Got expected bbox");
}

TEST(bboxFromGeofenceNoVertices) {
    GeoCoord verts[] = {};

    Geofence geofence;
    geofence.verts = verts;
    geofence.numVerts = 0;

    const BBox expected = {0.0, 0.0, 0.0, 0.0};

    BBox result;
    bboxFromGeofence(&geofence, &result);

    t_assert(bboxEquals(&result, &expected), "Got expected bbox");
}

TEST(bboxesFromGeoPolygon) {
    GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};

    Geofence geofence;
    geofence.verts = verts;
    geofence.numVerts = 4;

    GeoPolygon polygon;
    polygon.geofence = geofence;
    polygon.numHoles = 0;

    const BBox expected = {1.1, 0.7, 0.7, 0.2};

    BBox* result = calloc(sizeof(BBox), 1);
    bboxesFromGeoPolygon(&polygon, result);
    t_assert(bboxEquals(&result[0], &expected), "Got expected bbox");

    free(result);
}

TEST(bboxesFromGeoPolygonHole) {
    GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};

    Geofence geofence;
    geofence.verts = verts;
    geofence.numVerts = 4;

    // not a real hole, but doesn't matter for the test
    GeoCoord holeVerts[] = {{0.9, 0.3}, {0.9, 0.5}, {1.0, 0.7}, {0.9, 0.3}};

    Geofence holeGeofence;
    holeGeofence.verts = holeVerts;
    holeGeofence.numVerts = 4;

    GeoPolygon polygon;
    polygon.geofence = geofence;
    polygon.numHoles = 1;
    polygon.holes = &holeGeofence;

    const BBox expected = {1.1, 0.7, 0.7, 0.2};
    const BBox expectedHole = {1.0, 0.9, 0.7, 0.3};

    BBox* result = calloc(sizeof(BBox), 2);
    bboxesFromGeoPolygon(&polygon, result);
    t_assert(bboxEquals(&result[0], &expected), "Got expected bbox");
    t_assert(bboxEquals(&result[1], &expectedHole), "Got expected hole bbox");

    free(result);
}

TEST(bboxFromLinkedGeoLoop) {
    const GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};

    LinkedGeoLoop loop;
    initLinkedLoop(&loop);

    for (int i = 0; i < 4; i++) {
        addLinkedCoord(&loop, &verts[i]);
    }

    const BBox expected = {1.1, 0.7, 0.7, 0.2};

    BBox result;
    bboxFromLinkedGeoLoop(&loop, &result);
    t_assert(bboxEquals(&result, &expected), "Got expected bbox");

    destroyLinkedGeoLoop(&loop);
}

TEST(bboxFromLinkedGeoLoopNoVertices) {
    LinkedGeoLoop loop;
    initLinkedLoop(&loop);

    const BBox expected = {0.0, 0.0, 0.0, 0.0};

    BBox result;
    bboxFromLinkedGeoLoop(&loop, &result);

    t_assert(bboxEquals(&result, &expected), "Got expected bbox");
}

TEST(loopIsEmptyLinkedLoop) {
    LinkedGeoLoop linkedGeoLoop;
    initLinkedLoop(&linkedGeoLoop);

    IterableGeoLoop loop;
    loop.linkedGeoLoop = &linkedGeoLoop;
    loop.type = TYPE_LINKED_GEO_LOOP;

    t_assert(loopIsEmpty(&loop) == true, "LinkedGeoLoop is empty");

    const GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
    for (int i = 0; i < 4; i++) {
        addLinkedCoord(&linkedGeoLoop, &verts[i]);
    }

    t_assert(loopIsEmpty(&loop) == false, "LinkedGeoLoop is not empty");

    destroyLinkedGeoLoop(&linkedGeoLoop);
}

TEST(loopIsEmptyLinkedLoopGeofence) {
    GeoCoord noVerts[] = {};

    Geofence geofence;
    geofence.verts = noVerts;
    geofence.numVerts = 0;

    IterableGeoLoop loop;
    loop.geofence = &geofence;
    loop.type = TYPE_GEOFENCE;

    t_assert(loopIsEmpty(&loop) == true, "Geofence is empty");

    GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
    geofence.verts = verts;
    geofence.numVerts = 4;

    t_assert(loopIsEmpty(&loop) == false, "Geofence is not empty");
}

TEST(loopIsEmptyUnknown) {
    IterableGeoLoop loop;
    loop.type = 42;

    t_assert(loopIsEmpty(&loop) == true, "Unknown loop type is empty");
}

END_TESTS();
