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

#include <stdlib.h>

#include "algos.h"
#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

// Tests for specific polygonToCells examples

SUITE(polygonToCells_reported) {
    // https://github.com/uber/h3-js/issues/76#issuecomment-561204505
    TEST(entireWorld) {
        // TODO: Fails for a single worldwide polygon
        LatLng worldVerts[] = {
            {-M_PI_2, -M_PI}, {M_PI_2, -M_PI}, {M_PI_2, 0}, {-M_PI_2, 0}};
        GeoLoop worldGeoLoop = {.numVerts = 4, .verts = worldVerts};
        GeoPolygon worldGeoPolygon = {.geoloop = worldGeoLoop, .numHoles = 0};
        LatLng worldVerts2[] = {
            {-M_PI_2, 0}, {M_PI_2, 0}, {M_PI_2, M_PI}, {-M_PI_2, M_PI}};
        GeoLoop worldGeoLoop2 = {.numVerts = 4, .verts = worldVerts2};
        GeoPolygon worldGeoPolygon2 = {.geoloop = worldGeoLoop2, .numHoles = 0};

        for (int res = 0; res < 3; res++) {
            int64_t polygonToCellsSize;
            t_assertSuccess(H3_EXPORT(maxPolygonToCellsSize)(
                &worldGeoPolygon, res, 0, &polygonToCellsSize));
            H3Index *polygonToCellsOut =
                calloc(polygonToCellsSize, sizeof(H3Index));

            t_assertSuccess(H3_EXPORT(polygonToCells)(&worldGeoPolygon, res, 0,
                                                      polygonToCellsOut));
            int64_t actualNumIndexes =
                countNonNullIndexes(polygonToCellsOut, polygonToCellsSize);

            int64_t polygonToCellsSize2;
            t_assertSuccess(H3_EXPORT(maxPolygonToCellsSize)(
                &worldGeoPolygon2, res, 0, &polygonToCellsSize2));
            H3Index *polygonToCellsOut2 =
                calloc(polygonToCellsSize2, sizeof(H3Index));

            t_assertSuccess(H3_EXPORT(polygonToCells)(&worldGeoPolygon2, res, 0,
                                                      polygonToCellsOut2));
            int64_t actualNumIndexes2 =
                countNonNullIndexes(polygonToCellsOut2, polygonToCellsSize2);

            int64_t expectedTotalWorld;
            t_assertSuccess(H3_EXPORT(getNumCells)(res, &expectedTotalWorld));
            t_assert(actualNumIndexes + actualNumIndexes2 == expectedTotalWorld,
                     "got expected polygonToCells size (entire world)");

            // Sets should be disjoint
            for (int i = 0; i < polygonToCellsSize; i++) {
                if (polygonToCellsOut[i] == 0) continue;

                bool found = false;
                for (int j = 0; j < polygonToCellsSize2; j++) {
                    if (polygonToCellsOut[i] == polygonToCellsOut2[j]) {
                        found = true;
                        break;
                    }
                }
                t_assert(
                    !found,
                    "Index found more than once when polygonToCellsing the "
                    "entire world");
            }

            free(polygonToCellsOut);
            free(polygonToCellsOut2);
        }
    }

    // https://github.com/uber/h3-js/issues/67
    TEST(h3js_67) {
        double east = H3_EXPORT(degsToRads)(-56.25);
        double north = H3_EXPORT(degsToRads)(-33.13755119234615);
        double south = H3_EXPORT(degsToRads)(-34.30714385628804);
        double west = H3_EXPORT(degsToRads)(-57.65625);

        LatLng testVerts[] = {
            {north, east}, {south, east}, {south, west}, {north, west}};
        GeoLoop testGeoLoop = {.numVerts = 4, .verts = testVerts};
        GeoPolygon testPolygon;
        testPolygon.geoloop = testGeoLoop;
        testPolygon.numHoles = 0;

        int res = 7;
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSize)(&testPolygon, res, 0,
                                                         &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(
            H3_EXPORT(polygonToCells)(&testPolygon, res, 0, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 4499,
                 "got expected polygonToCells size (h3-js#67)");
        free(hexagons);
    }

    // 2nd test case from h3-js#67
    TEST(h3js_67_2nd) {
        double east = H3_EXPORT(degsToRads)(-57.65625);
        double north = H3_EXPORT(degsToRads)(-34.30714385628804);
        double south = H3_EXPORT(degsToRads)(-35.4606699514953);
        double west = H3_EXPORT(degsToRads)(-59.0625);

        LatLng testVerts[] = {
            {north, east}, {south, east}, {south, west}, {north, west}};
        GeoLoop testGeoLoop = {.numVerts = 4, .verts = testVerts};
        GeoPolygon testPolygon;
        testPolygon.geoloop = testGeoLoop;
        testPolygon.numHoles = 0;

        int res = 7;
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSize)(&testPolygon, res, 0,
                                                         &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(
            H3_EXPORT(polygonToCells)(&testPolygon, res, 0, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 4609,
                 "got expected polygonToCells size (h3-js#67, 2nd case)");
        free(hexagons);
    }

    // https://github.com/uber/h3/issues/136
    TEST(h3_136) {
        LatLng testVerts[] = {{0.10068990369902957, 0.8920772174196191},
                              {0.10032914690616246, 0.8915914753447348},
                              {0.10033349237998787, 0.8915860128746426},
                              {0.10069496685903621, 0.8920742194546231}};
        GeoLoop testGeoLoop = {.numVerts = 4, .verts = testVerts};
        GeoPolygon testPolygon;
        testPolygon.geoloop = testGeoLoop;
        testPolygon.numHoles = 0;

        int res = 13;
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSize)(&testPolygon, res, 0,
                                                         &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(
            H3_EXPORT(polygonToCells)(&testPolygon, res, 0, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 4353, "got expected polygonToCells size");
        free(hexagons);
    }

    // https://github.com/uber/h3/issues/595
    TEST(h3_136) {
        H3Index center = 0x85283473fffffff;
        LatLng centerLatLng;
        H3_EXPORT(cellToLatLng)(center, &centerLatLng);

        // This polygon should include the center cell. The issue here arises
        // when one of the polygon vertexes is to the east of the index center,
        // with exactly the same latitude
        LatLng testVerts[] = {{.lat = centerLatLng.lat, -2.121207808248113},
                              {0.6565301558937859, -2.1281107217935986},
                              {0.6515463604919347, -2.1345342663428695},
                              {0.6466583305904194, -2.1276313527973842}};

        GeoLoop testGeoLoop = {.numVerts = 4, .verts = testVerts};
        GeoPolygon testPolygon;
        testPolygon.geoloop = testGeoLoop;
        testPolygon.numHoles = 0;

        int res = 5;
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSize)(&testPolygon, res, 0,
                                                         &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(
            H3_EXPORT(polygonToCells)(&testPolygon, res, 0, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 8, "got expected polygonToCells size");
        free(hexagons);
    }
}
