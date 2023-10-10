/*
 * Copyright 2016-2018, 2020-2021 Uber Technologies, Inc.
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
#include "latLng.h"
#include "polygon.h"
#include "test.h"
#include "utility.h"

void assertBBoxFromGeoLoop(const GeoLoop *geoloop, const BBox *expected,
                           const LatLng *inside, const LatLng *outside) {
    BBox result;

    bboxFromGeoLoop(geoloop, &result);

    t_assert(bboxEquals(&result, expected), "Got expected bbox");
    t_assert(bboxContains(&result, inside), "Contains expected inside point");
    t_assert(!bboxContains(&result, outside),
             "Does not contain expected outside point");
}

void assertBBox(const BBox *bbox, const BBox *expected) {
    LatLng actualNE = {.lat = bbox->north, .lng = bbox->east};
    LatLng expectedNE = {.lat = expected->north, .lng = expected->east};
    t_assert(geoAlmostEqual(&actualNE, &expectedNE), "NE corner matches");

    LatLng actualSW = {.lat = bbox->south, .lng = bbox->west};
    LatLng expectedSW = {.lat = expected->south, .lng = expected->west};
    t_assert(geoAlmostEqual(&actualSW, &expectedSW), "SW corner matches");
}

SUITE(BBox) {
    TEST(posLatPosLng) {
        LatLng verts[] = {{0.8, 0.3}, {0.7, 0.6}, {1.1, 0.7}, {1.0, 0.2}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {1.1, 0.7, 0.7, 0.2};
        const LatLng inside = {0.9, 0.4};
        const LatLng outside = {0.0, 0.0};
        assertBBoxFromGeoLoop(&geoloop, &expected, &inside, &outside);
    }

    TEST(negLatPosLng) {
        LatLng verts[] = {{-0.3, 0.6}, {-0.4, 0.9}, {-0.2, 0.8}, {-0.1, 0.6}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {-0.1, -0.4, 0.9, 0.6};
        const LatLng inside = {-0.3, 0.8};
        const LatLng outside = {0.0, 0.0};
        assertBBoxFromGeoLoop(&geoloop, &expected, &inside, &outside);
    }

    TEST(posLatNegLng) {
        LatLng verts[] = {{0.7, -1.4}, {0.8, -0.9}, {1.0, -0.8}, {1.1, -1.3}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {1.1, 0.7, -0.8, -1.4};
        const LatLng inside = {0.9, -1.0};
        const LatLng outside = {0.0, 0.0};
        assertBBoxFromGeoLoop(&geoloop, &expected, &inside, &outside);
    }

    TEST(negLatNegLng) {
        LatLng verts[] = {
            {-0.4, -1.4}, {-0.3, -1.1}, {-0.1, -1.2}, {-0.2, -1.4}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {-0.1, -0.4, -1.1, -1.4};
        const LatLng inside = {-0.3, -1.2};
        const LatLng outside = {0.0, 0.0};
        assertBBoxFromGeoLoop(&geoloop, &expected, &inside, &outside);
    }

    TEST(aroundZeroZero) {
        LatLng verts[] = {{0.4, -0.4}, {0.4, 0.4}, {-0.4, 0.4}, {-0.4, -0.4}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {0.4, -0.4, 0.4, -0.4};
        const LatLng inside = {-0.1, -0.1};
        const LatLng outside = {1.0, -1.0};
        assertBBoxFromGeoLoop(&geoloop, &expected, &inside, &outside);
    }

    TEST(transmeridian) {
        LatLng verts[] = {{0.4, M_PI - 0.1},
                          {0.4, -M_PI + 0.1},
                          {-0.4, -M_PI + 0.1},
                          {-0.4, M_PI - 0.1}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {0.4, -0.4, -M_PI + 0.1, M_PI - 0.1};
        const LatLng insideOnMeridian = {-0.1, M_PI};
        const LatLng outside = {1.0, M_PI - 0.5};
        assertBBoxFromGeoLoop(&geoloop, &expected, &insideOnMeridian, &outside);

        const LatLng westInside = {0.1, M_PI - 0.05};
        t_assert(bboxContains(&expected, &westInside),
                 "Contains expected west inside point");
        const LatLng eastInside = {0.1, -M_PI + 0.05};
        t_assert(bboxContains(&expected, &eastInside),
                 "Contains expected east outside point");

        const LatLng westOutside = {0.1, M_PI - 0.5};
        t_assert(!bboxContains(&expected, &westOutside),
                 "Does not contain expected west outside point");
        const LatLng eastOutside = {0.1, -M_PI + 0.5};
        t_assert(!bboxContains(&expected, &eastOutside),
                 "Does not contain expected east outside point");
    }

    TEST(edgeOnNorthPole) {
        LatLng verts[] = {{M_PI_2 - 0.1, 0.1},
                          {M_PI_2 - 0.1, 0.8},
                          {M_PI_2, 0.8},
                          {M_PI_2, 0.1}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {M_PI_2, M_PI_2 - 0.1, 0.8, 0.1};
        const LatLng inside = {M_PI_2 - 0.01, 0.4};
        const LatLng outside = {M_PI_2, 0.9};
        assertBBoxFromGeoLoop(&geoloop, &expected, &inside, &outside);
    }

    TEST(edgeOnSouthPole) {
        LatLng verts[] = {{-M_PI_2 + 0.1, 0.1},
                          {-M_PI_2 + 0.1, 0.8},
                          {-M_PI_2, 0.8},
                          {-M_PI_2, 0.1}};
        const GeoLoop geoloop = {.numVerts = 4, .verts = verts};
        const BBox expected = {-M_PI_2 + 0.1, -M_PI_2, 0.8, 0.1};
        const LatLng inside = {-M_PI_2 + 0.01, 0.4};
        const LatLng outside = {-M_PI_2, 0.9};
        assertBBoxFromGeoLoop(&geoloop, &expected, &inside, &outside);
    }

    TEST(containsEdges) {
        const BBox bbox = {0.1, -0.1, 0.2, -0.2};
        LatLng points[] = {
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
        LatLng points[] = {
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

    TEST(bboxOverlapsBBox) {
        BBox a = {1.0, 0.0, 1.0, 0.0};

        BBox b1 = {1.0, 0.0, -1.0, -1.5};
        t_assert(!bboxOverlapsBBox(&a, &b1), "no intersection to the west");

        BBox b2 = {1.0, 0.0, 2.0, 1.5};
        t_assert(!bboxOverlapsBBox(&a, &b2), "no intersection to the east");

        BBox b3 = {-1.0, -1.5, 1.0, 0.0};
        t_assert(!bboxOverlapsBBox(&a, &b3), "no intersection to the south");

        BBox b4 = {2.0, 1.5, 1.0, 0.0};
        t_assert(!bboxOverlapsBBox(&a, &b4), "no intersection to the north");

        BBox b5 = {1.0, 0.0, 0.5, -1.5};
        t_assert(bboxOverlapsBBox(&a, &b5), "intersection to the west");

        BBox b6 = {1.0, 0.0, 2.0, 0.5};
        t_assert(bboxOverlapsBBox(&a, &b6), "intersection to the east");

        BBox b7 = {0.5, -1.5, 1.0, 0.0};
        t_assert(bboxOverlapsBBox(&a, &b7), "intersection to the south");

        BBox b8 = {2.0, 0.5, 1.0, 0.0};
        t_assert(bboxOverlapsBBox(&a, &b8), "intersection to the north");

        BBox b9 = {1.5, -0.5, 1.5, -0.5};
        t_assert(bboxOverlapsBBox(&a, &b9), "intersection, b contains a");

        BBox b10 = {0.5, 0.25, 0.5, 0.25};
        t_assert(bboxOverlapsBBox(&a, &b10), "intersection, a contains b");

        BBox b11 = {1.0, 0.0, 1.0, 0.0};
        t_assert(bboxOverlapsBBox(&a, &b11), "intersection, a equals b");
    }

    TEST(bboxOverlapsBBoxTransmeridian) {
        BBox a = {1.0, 0.0, -M_PI + 0.5, M_PI - 0.5};

        BBox b1 = {1.0, 0.0, M_PI - 0.7, M_PI - 0.9};
        t_assert(!bboxOverlapsBBox(&a, &b1), "no intersection to the west");

        BBox b2 = {1.0, 0.0, -M_PI + 0.9, -M_PI + 0.7};
        t_assert(!bboxOverlapsBBox(&a, &b2), "no intersection to the east");

        BBox b3 = {1.0, 0.0, M_PI - 0.4, M_PI - 0.9};
        t_assert(bboxOverlapsBBox(&a, &b3), "intersection to the west");
        t_assert(bboxOverlapsBBox(&b3, &a),
                 "intersection to the west, reverse");

        BBox b4 = {1.0, 0.0, -M_PI + 0.9, -M_PI + 0.4};
        t_assert(bboxOverlapsBBox(&a, &b4), "intersection to the east");
        t_assert(bboxOverlapsBBox(&b4, &a),
                 "intersection to the east, reverse");

        BBox b5 = {1.0, 0.0, -M_PI + 0.4, M_PI - 0.4};
        t_assert(bboxOverlapsBBox(&a, &b5), "intersection, a contains b");

        BBox b6 = {1.0, 0.0, -M_PI + 0.6, M_PI - 0.6};
        t_assert(bboxOverlapsBBox(&a, &b6), "intersection, b contains a");

        BBox b7 = {1.0, 0.0, -M_PI + 0.5, M_PI - 0.5};
        t_assert(bboxOverlapsBBox(&a, &b7), "intersection, a equals b");

        BBox b8 = {1.0, 0.0, -M_PI + 0.9, M_PI - 0.4};
        t_assert(bboxOverlapsBBox(&a, &b8),
                 "intersection, transmeridian to the east");
        t_assert(bboxOverlapsBBox(&b8, &a),
                 "intersection, transmeridian to the east, reverse");

        BBox b9 = {1.0, 0.0, -M_PI + 0.4, M_PI - 0.9};
        t_assert(bboxOverlapsBBox(&a, &b9),
                 "intersection, transmeridian to the west");
        t_assert(bboxOverlapsBBox(&b9, &a),
                 "intersection, transmeridian to the west, reverse");
    }

    TEST(bboxCenterBasicQuandrants) {
        LatLng center;

        BBox bbox1 = {1.0, 0.8, 1.0, 0.8};
        LatLng expected1 = {0.9, 0.9};
        bboxCenter(&bbox1, &center);
        t_assert(geoAlmostEqual(&center, &expected1), "pos/pos as expected");

        BBox bbox2 = {-0.8, -1.0, 1.0, 0.8};
        LatLng expected2 = {-0.9, 0.9};
        bboxCenter(&bbox2, &center);
        t_assert(geoAlmostEqual(&center, &expected2), "neg/pos as expected");

        BBox bbox3 = {1.0, 0.8, -0.8, -1.0};
        LatLng expected3 = {0.9, -0.9};
        bboxCenter(&bbox3, &center);
        t_assert(geoAlmostEqual(&center, &expected3), "pos/neg as expected");

        BBox bbox4 = {-0.8, -1.0, -0.8, -1.0};
        LatLng expected4 = {-0.9, -0.9};
        bboxCenter(&bbox4, &center);
        t_assert(geoAlmostEqual(&center, &expected4), "neg/neg as expected");

        BBox bbox5 = {0.8, -0.8, 1.0, -1.0};
        LatLng expected5 = {0.0, 0.0};
        bboxCenter(&bbox5, &center);
        t_assert(geoAlmostEqual(&center, &expected5),
                 "around origin as expected");
    }

    TEST(bboxCenterTransmeridian) {
        LatLng center;

        BBox bbox1 = {1.0, 0.8, -M_PI + 0.3, M_PI - 0.1};
        LatLng expected1 = {0.9, -M_PI + 0.1};
        bboxCenter(&bbox1, &center);

        t_assert(geoAlmostEqual(&center, &expected1), "skew east as expected");

        BBox bbox2 = {1.0, 0.8, -M_PI + 0.1, M_PI - 0.3};
        LatLng expected2 = {0.9, M_PI - 0.1};
        bboxCenter(&bbox2, &center);
        t_assert(geoAlmostEqual(&center, &expected2), "skew west as expected");

        BBox bbox3 = {1.0, 0.8, -M_PI + 0.1, M_PI - 0.1};
        LatLng expected3 = {0.9, M_PI};
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

    TEST(bboxHexEstimate_invalidRes) {
        int64_t numHexagons;
        BBox bbox = {1.0, 0.0, 1.0, 0.0};
        t_assert(bboxHexEstimate(&bbox, -1, &numHexagons) == E_RES_DOMAIN,
                 "bboxHexEstimate of invalid resolution fails");
    }

    TEST(bboxHexEstimate_ratio) {
        BBox bbox1 = {0.82294, 0.82273, 0.131671, 0.131668};
        BBox bbox2 = {0.131671, 0.131668, 0.82294, 0.82273};
        int64_t numHexagons1;
        int64_t numHexagons2;

        t_assertSuccess(bboxHexEstimate(&bbox1, 15, &numHexagons1));
        t_assertSuccess(bboxHexEstimate(&bbox2, 15, &numHexagons2));

        double diffPercentage = fabs(1.0 - numHexagons1 / (double)numHexagons2);

        // numHexagons1 and numHexagons2 cannot be exactly equal because the
        // diameter of the two bboxes is not exactly the same (it's calculated
        // using greatCircleDistanceKm)
        t_assert(
            diffPercentage < 0.03,
            "Should be true for bounding boxes with (almost) the same diameter "
            "and side ratio");
    }

    TEST(lineHexEstimate_invalidRes) {
        int64_t numHexagons;
        LatLng origin = {0.0, 0.0};
        LatLng destination = {1.0, 1.0};
        t_assert(lineHexEstimate(&origin, &destination, -1, &numHexagons) ==
                     E_RES_DOMAIN,
                 "lineHexEstimate of invalid resolution fails");
    }

    TEST(scaleBBox_noop) {
        BBox bbox = {1.0, 0.0, 1.0, 0.0};
        BBox expected = {1.0, 0.0, 1.0, 0.0};
        scaleBBox(&bbox, 1);
        assertBBox(&bbox, &expected);
    }

    TEST(scaleBBox_basicGrow) {
        BBox bbox = {1.0, 0.0, 1.0, 0.0};
        BBox expected = {1.5, -0.5, 1.5, -0.5};
        scaleBBox(&bbox, 2);
        assertBBox(&bbox, &expected);
    }

    TEST(scaleBBox_basicShrink) {
        BBox bbox = {1.0, 0.0, 1.0, 0.0};
        BBox expected = {0.75, 0.25, 0.75, 0.25};
        scaleBBox(&bbox, 0.5);
        assertBBox(&bbox, &expected);
    }

    TEST(scaleBBox_clampNorthSouth) {
        BBox bbox = {M_PI_2 * 0.9, -M_PI_2 * 0.9, 1.0, 0.0};
        BBox expected = {M_PI_2, -M_PI_2, 1.5, -0.5};
        scaleBBox(&bbox, 2);
        assertBBox(&bbox, &expected);
    }

    TEST(scaleBBox_clampEastPos) {
        BBox bbox = {1.0, 0.0, M_PI - 0.1, M_PI - 1.1};
        BBox expected = {1.5, -0.5, -M_PI + 0.4, M_PI - 1.6};
        scaleBBox(&bbox, 2);
        assertBBox(&bbox, &expected);
    }

    TEST(scaleBBox_clampEastNeg) {
        BBox bbox = {1.5, -0.5, -M_PI + 0.4, M_PI - 1.6};
        BBox expected = {1.0, 0.0, M_PI - 0.1, M_PI - 1.1};
        scaleBBox(&bbox, 0.5);
        assertBBox(&bbox, &expected);
    }

    TEST(scaleBBox_clampWestPos) {
        BBox bbox = {1.0, 0.0, -M_PI + 0.9, M_PI - 0.1};
        BBox expected = {0.75, 0.25, -M_PI + 0.65, -M_PI + 0.15};
        scaleBBox(&bbox, 0.5);
        assertBBox(&bbox, &expected);
    }

    TEST(scaleBBox_clampWestNeg) {
        BBox bbox = {0.75, 0.25, -M_PI + 0.65, -M_PI + 0.15};
        BBox expected = {1.0, 0.0, -M_PI + 0.9, M_PI - 0.1};
        scaleBBox(&bbox, 2);
        assertBBox(&bbox, &expected);
    }
}
