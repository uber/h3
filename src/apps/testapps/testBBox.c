/*
 * Copyright 2016-2018 Uber Technologies, Inc.
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
#include "polygon.h"
#include "test.h"

void assertBBox(const Geofence* geofence, const BBox* expected,
                const GeoCoord* inside, const GeoCoord* outside) {
    BBox result;

    bboxFromGeofence(geofence, &result);

    t_assert(bboxEquals(&result, expected), "Got expected bbox");
    t_assert(bboxContains(&result, inside), "Contains expected inside point");
    t_assert(!bboxContains(&result, outside),
             "Does not contain expected outside point");
}

SUITE(BBox) {
    TEST(posLatPosLon) {
        GeoCoord verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {1.1, 0.7, 0.7, 0.2};
        const GeoCoord inside = {0.9, 0.4};
        const GeoCoord outside = {0.0, 0.0};
        assertBBox(&geofence, &expected, &inside, &outside);
    }

    TEST(negLatPosLon) {
        GeoCoord verts[] = {{-0.3, 0.6}, {-0.4, 0.9}, {-0.2, 0.8}, {-0.1, 0.6}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {-0.1, -0.4, 0.9, 0.6};
        const GeoCoord inside = {-0.3, 0.8};
        const GeoCoord outside = {0.0, 0.0};
        assertBBox(&geofence, &expected, &inside, &outside);
    }

    TEST(posLatNegLon) {
        GeoCoord verts[] = {{0.7, -1.4}, {0.8, -0.9}, {1.0, -0.8}, {1.1, -1.3}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {1.1, 0.7, -0.8, -1.4};
        const GeoCoord inside = {0.9, -1.0};
        const GeoCoord outside = {0.0, 0.0};
        assertBBox(&geofence, &expected, &inside, &outside);
    }

    TEST(negLatNegLon) {
        GeoCoord verts[] = {
            {-0.4, -1.4}, {-0.3, -1.1}, {-0.1, -1.2}, {-0.2, -1.4}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {-0.1, -0.4, -1.1, -1.4};
        const GeoCoord inside = {-0.3, -1.2};
        const GeoCoord outside = {0.0, 0.0};
        assertBBox(&geofence, &expected, &inside, &outside);
    }

    TEST(aroundZeroZero) {
        GeoCoord verts[] = {{0.4, -0.4}, {0.4, 0.4}, {-0.4, 0.4}, {-0.4, -0.4}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {0.4, -0.4, 0.4, -0.4};
        const GeoCoord inside = {-0.1, -0.1};
        const GeoCoord outside = {1.0, -1.0};
        assertBBox(&geofence, &expected, &inside, &outside);
    }

    TEST(transmeridian) {
        GeoCoord verts[] = {{0.4, M_PI - 0.1},
                            {0.4, -M_PI + 0.1},
                            {-0.4, -M_PI + 0.1},
                            {-0.4, M_PI - 0.1}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {0.4, -0.4, -M_PI + 0.1, M_PI - 0.1};
        const GeoCoord insideOnMeridian = {-0.1, M_PI};
        const GeoCoord outside = {1.0, M_PI - 0.5};
        assertBBox(&geofence, &expected, &insideOnMeridian, &outside);

        const GeoCoord westInside = {0.1, M_PI - 0.05};
        t_assert(bboxContains(&expected, &westInside),
                 "Contains expected west inside point");
        const GeoCoord eastInside = {0.1, -M_PI + 0.05};
        t_assert(bboxContains(&expected, &eastInside),
                 "Contains expected east outside point");

        const GeoCoord westOutside = {0.1, M_PI - 0.5};
        t_assert(!bboxContains(&expected, &westOutside),
                 "Does not contain expected west outside point");
        const GeoCoord eastOutside = {0.1, -M_PI + 0.5};
        t_assert(!bboxContains(&expected, &eastOutside),
                 "Does not contain expected east outside point");
    }

    TEST(edgeOnNorthPole) {
        GeoCoord verts[] = {{M_PI_2 - 0.1, 0.1},
                            {M_PI_2 - 0.1, 0.8},
                            {M_PI_2, 0.8},
                            {M_PI_2, 0.1}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {M_PI_2, M_PI_2 - 0.1, 0.8, 0.1};
        const GeoCoord inside = {M_PI_2 - 0.01, 0.4};
        const GeoCoord outside = {M_PI_2, 0.9};
        assertBBox(&geofence, &expected, &inside, &outside);
    }

    TEST(edgeOnSouthPole) {
        GeoCoord verts[] = {{-M_PI_2 + 0.1, 0.1},
                            {-M_PI_2 + 0.1, 0.8},
                            {-M_PI_2, 0.8},
                            {-M_PI_2, 0.1}};
        const Geofence geofence = {.numVerts = 4, .verts = verts};
        const BBox expected = {-M_PI_2 + 0.1, -M_PI_2, 0.8, 0.1};
        const GeoCoord inside = {-M_PI_2 + 0.01, 0.4};
        const GeoCoord outside = {-M_PI_2, 0.9};
        assertBBox(&geofence, &expected, &inside, &outside);
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
        t_assert(geoAlmostEqual(&center, &expected5),
                 "around origin as expected");
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

    TEST(bboxEquals) {
        BBox bbox = {1.0, 0.0, 1.0, 0.0};
        BBox north = bbox;
        north.north += 0.1;
        BBox south = bbox;
        south.south += 0.1;
        BBox east = bbox;
        east.east += 0.1;
        BBox west = bbox;
        west.west += 0.1;

        t_assert(bboxEquals(&bbox, &bbox), "Equals self");
        t_assert(!bboxEquals(&bbox, &north), "Not equals different north");
        t_assert(!bboxEquals(&bbox, &south), "Not equals different south");
        t_assert(!bboxEquals(&bbox, &east), "Not equals different east");
        t_assert(!bboxEquals(&bbox, &west), "Not equals different west");
    }
}
