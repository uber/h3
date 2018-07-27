/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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

#include <math.h>
#include <stdlib.h>
#include "bbox.h"
#include "constants.h"
#include "geoCoord.h"
#include "test.h"

/**
 * Whether two bounding boxes are strictly equal
 * @param  b1 Bounding box 1
 * @param  b2 Bounding box 2
 * @return    Whether the boxes are equal
 */
bool bboxEquals(const BBox* b1, const BBox* b2) {
    return b1->north == b2->north && b1->south == b2->south &&
           b1->east == b2->east && b1->west == b2->west;
}

void assertBBox(const GeoCoord* verts, const BBox* expected,
                const GeoCoord* inside, const GeoCoord* outside) {
    BBox result;
    bboxFromVertices(verts, 4, &result);

    t_assert(bboxEquals(&result, expected), "Got expected bbox");
    t_assert(bboxContains(&result, inside), "Contains expected inside point");
    t_assert(!bboxContains(&result, outside),
             "Does not contain expected outside point");
}

BEGIN_TESTS(BBox);

TEST(posLatPosLon) {
    const GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
    const BBox expected = {1.1, 0.7, 0.7, 0.2};
    const GeoCoord inside = {0.9, 0.4};
    const GeoCoord outside = {0.0, 0.0};
    assertBBox(verts, &expected, &inside, &outside);
}

TEST(negLatPosLon) {
    const GeoCoord verts[] = {
        {-0.3, 0.6}, {-0.4, 0.9}, {-0.2, 0.8}, {-0.1, 0.6}};
    const BBox expected = {-0.1, -0.4, 0.9, 0.6};
    const GeoCoord inside = {-0.3, 0.8};
    const GeoCoord outside = {0.0, 0.0};
    assertBBox(verts, &expected, &inside, &outside);
}

TEST(posLatNegLon) {
    const GeoCoord verts[] = {
        {0.7, -1.4}, {0.8, -0.9}, {1.0, -0.8}, {1.1, -1.3}};
    const BBox expected = {1.1, 0.7, -0.8, -1.4};
    const GeoCoord inside = {0.9, -1.0};
    const GeoCoord outside = {0.0, 0.0};
    assertBBox(verts, &expected, &inside, &outside);
}

TEST(negLatNegLon) {
    const GeoCoord verts[] = {
        {-0.4, -1.4}, {-0.3, -1.1}, {-0.1, -1.2}, {-0.2, -1.4}};
    const BBox expected = {-0.1, -0.4, -1.1, -1.4};
    const GeoCoord inside = {-0.3, -1.2};
    const GeoCoord outside = {0.0, 0.0};
    assertBBox(verts, &expected, &inside, &outside);
}

TEST(aroundZeroZero) {
    const GeoCoord verts[] = {
        {0.4, -0.4}, {0.4, 0.4}, {-0.4, 0.4}, {-0.4, -0.4}};
    const BBox expected = {0.4, -0.4, 0.4, -0.4};
    const GeoCoord inside = {-0.1, -0.1};
    const GeoCoord outside = {1.0, -1.0};
    assertBBox(verts, &expected, &inside, &outside);
}

TEST(transmeridian) {
    const GeoCoord verts[] = {{0.4, M_PI - 0.1},
                              {0.4, -M_PI + 0.1},
                              {-0.4, -M_PI + 0.1},
                              {-0.4, M_PI - 0.1}};
    const BBox expected = {0.4, -0.4, -M_PI + 0.1, M_PI - 0.1};
    const GeoCoord inside = {-0.1, M_PI};
    const GeoCoord outside = {1.0, M_PI - 0.5};
    assertBBox(verts, &expected, &inside, &outside);

    BBox result;
    bboxFromVertices(verts, 4, &result);

    const GeoCoord westOutside = {0.1, M_PI - 0.5};
    t_assert(!bboxContains(&result, &westOutside),
             "Does not contain expected west outside point");
    const GeoCoord eastOutside = {0.1, -M_PI + 0.5};
    t_assert(!bboxContains(&result, &eastOutside),
             "Does not contain expected east outside point");
}

TEST(edgeOnNorthPole) {
    const GeoCoord verts[] = {
        {M_PI_2 - 0.1, 0.1}, {M_PI_2 - 0.1, 0.8}, {M_PI_2, 0.8}, {M_PI_2, 0.1}};
    const BBox expected = {M_PI_2, M_PI_2 - 0.1, 0.8, 0.1};
    const GeoCoord inside = {M_PI_2 - 0.01, 0.4};
    const GeoCoord outside = {M_PI_2, 0.9};
    assertBBox(verts, &expected, &inside, &outside);
}

TEST(edgeOnSouthPole) {
    const GeoCoord verts[] = {{-M_PI_2 + 0.1, 0.1},
                              {-M_PI_2 + 0.1, 0.8},
                              {-M_PI_2, 0.8},
                              {-M_PI_2, 0.1}};
    const BBox expected = {-M_PI_2 + 0.1, -M_PI_2, 0.8, 0.1};
    const GeoCoord inside = {-M_PI_2 + 0.01, 0.4};
    const GeoCoord outside = {-M_PI_2, 0.9};
    assertBBox(verts, &expected, &inside, &outside);
}

TEST(containsEdges) {
    const BBox bbox = {0.1, -0.1, 0.2, -0.2};
    GeoCoord points[] = {
        {0.1, 0.2},  {0.1, 0.0},  {0.1, -0.2},  {0.0, 0.2},
        {-0.1, 0.2}, {-0.1, 0.0}, {-0.1, -0.2}, {0.0, -0.2},
    };
    const int numPoints = 8;

    for (int i = 0; i < numPoints; i++) {
        t_assert(bboxContains(&bbox, &points[i]), "Contains edge point");
    }
}

TEST(containsEdgesTransmeridian) {
    const BBox bbox = {0.1, -0.1, -M_PI + 0.2, M_PI - 0.2};
    GeoCoord points[] = {
        {0.1, -M_PI + 0.2}, {0.1, M_PI},         {0.1, M_PI - 0.2},
        {0.0, -M_PI + 0.2}, {-0.1, -M_PI + 0.2}, {-0.1, M_PI},
        {-0.1, M_PI - 0.2}, {0.0, M_PI - 0.2},
    };
    const int numPoints = 8;

    for (int i = 0; i < numPoints; i++) {
        t_assert(bboxContains(&bbox, &points[i]),
                 "Contains transmeridian edge point");
    }
}

TEST(noVertices) {
    const BBox expected = {0.0, 0.0, 0.0, 0.0};

    BBox result;
    bboxFromVertices(NULL, 0, &result);

    t_assert(bboxEquals(&result, &expected), "Got expected bbox");
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

TEST(bboxCenterBasicQuandrants) {
    GeoCoord center;

    BBox bbox1 = {1.0, 0.8, 1.0, 0.8};
    GeoCoord expected1 = {0.9, 0.9};
    bboxCenter(&bbox1, &center);
    t_assert(geoAlmostEqual(&center, &expected1), "pos/pos as expected");

    BBox bbox2 = {-0.8, -1.0, 1.0, 0.8};
    GeoCoord expected2 = {-0.9, 0.9};
    bboxCenter(&bbox2, &center);
    t_assert(geoAlmostEqual(&center, &expected2), "neg/pos as expected");

    BBox bbox3 = {1.0, 0.8, -0.8, -1.0};
    GeoCoord expected3 = {0.9, -0.9};
    bboxCenter(&bbox3, &center);
    t_assert(geoAlmostEqual(&center, &expected3), "pos/neg as expected");

    BBox bbox4 = {-0.8, -1.0, -0.8, -1.0};
    GeoCoord expected4 = {-0.9, -0.9};
    bboxCenter(&bbox4, &center);
    t_assert(geoAlmostEqual(&center, &expected4), "neg/neg as expected");

    BBox bbox5 = {0.8, -0.8, 1.0, -1.0};
    GeoCoord expected5 = {0.0, 0.0};
    bboxCenter(&bbox5, &center);
    t_assert(geoAlmostEqual(&center, &expected5), "around origin as expected");
}

TEST(bboxCenterTransmeridian) {
    GeoCoord center;

    BBox bbox1 = {1.0, 0.8, -M_PI + 0.3, M_PI - 0.1};
    GeoCoord expected1 = {0.9, -M_PI + 0.1};
    bboxCenter(&bbox1, &center);

    t_assert(geoAlmostEqual(&center, &expected1), "skew east as expected");

    BBox bbox2 = {1.0, 0.8, -M_PI + 0.1, M_PI - 0.3};
    GeoCoord expected2 = {0.9, M_PI - 0.1};
    bboxCenter(&bbox2, &center);
    t_assert(geoAlmostEqual(&center, &expected2), "skew west as expected");

    BBox bbox3 = {1.0, 0.8, -M_PI + 0.1, M_PI - 0.1};
    GeoCoord expected3 = {0.9, M_PI};
    bboxCenter(&bbox3, &center);
    t_assert(geoAlmostEqual(&center, &expected3),
             "on antimeridian as expected");
}

TEST(bboxIsTransmeridian) {
    BBox bboxNormal = {1.0, 0.8, 1.0, 0.8};
    t_assert(!bboxIsTransmeridian(&bboxNormal),
             "Normal bbox not transmeridian");

    BBox bboxTransmeridian = {1.0, 0.8, -M_PI + 0.3, M_PI - 0.1};
    t_assert(bboxIsTransmeridian(&bboxTransmeridian),
             "Transmeridian bbox is transmeridian");
}

END_TESTS();
