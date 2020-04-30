/*
 * Copyright 2017-2019 Uber Technologies, Inc.
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
#include "geoCoord.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

// Tests for specific polyfill examples

SUITE(polyfill_reported) {
    // https://github.com/uber/h3-js/issues/76#issuecomment-561204505
    TEST(entireWorld) {
        // TODO: Fails for a single worldwide polygon
        GeoCoord worldVerts[] = {
            {-M_PI_2, -M_PI}, {M_PI_2, -M_PI}, {M_PI_2, 0}, {-M_PI_2, 0}};
        Geofence worldGeofence = {.numVerts = 4, .verts = worldVerts};
        GeoPolygon worldGeoPolygon = {.geofence = worldGeofence, .numHoles = 0};
        GeoCoord worldVerts2[] = {
            {-M_PI_2, 0}, {M_PI_2, 0}, {M_PI_2, M_PI}, {-M_PI_2, M_PI}};
        Geofence worldGeofence2 = {.numVerts = 4, .verts = worldVerts2};
        GeoPolygon worldGeoPolygon2 = {.geofence = worldGeofence2,
                                       .numHoles = 0};

        for (int res = 0; res < 3; res++) {
            int polyfillSize =
                H3_EXPORT(maxPolyfillSize)(&worldGeoPolygon, res);
            H3Index* polyfillOut = calloc(polyfillSize, sizeof(H3Index));

            H3_EXPORT(polyfill)(&worldGeoPolygon, res, polyfillOut);
            int actualNumHexagons =
                countActualHexagons(polyfillOut, polyfillSize);

            int polyfillSize2 =
                H3_EXPORT(maxPolyfillSize)(&worldGeoPolygon2, res);
            H3Index* polyfillOut2 = calloc(polyfillSize2, sizeof(H3Index));

            H3_EXPORT(polyfill)(&worldGeoPolygon2, res, polyfillOut2);
            int actualNumHexagons2 =
                countActualHexagons(polyfillOut2, polyfillSize2);

            t_assert(actualNumHexagons + actualNumHexagons2 ==
                         H3_EXPORT(numHexagons)(res),
                     "got expected polyfill size (entire world)");

            // Sets should be disjoint
            for (int i = 0; i < polyfillSize; i++) {
                if (polyfillOut[i] == 0) continue;

                bool found = false;
                for (int j = 0; j < polyfillSize2; j++) {
                    if (polyfillOut[i] == polyfillOut2[j]) {
                        found = true;
                        break;
                    }
                }
                t_assert(!found,
                         "Index found more than once when polyfilling the "
                         "entire world");
            }

            free(polyfillOut);
            free(polyfillOut2);
        }
    }

    // https://github.com/uber/h3-js/issues/67
    TEST(h3js_67) {
        double east = H3_EXPORT(degsToRads)(-56.25);
        double north = H3_EXPORT(degsToRads)(-33.13755119234615);
        double south = H3_EXPORT(degsToRads)(-34.30714385628804);
        double west = H3_EXPORT(degsToRads)(-57.65625);

        GeoCoord testVerts[] = {
            {north, east}, {south, east}, {south, west}, {north, west}};
        Geofence testGeofence = {.numVerts = 4, .verts = testVerts};
        GeoPolygon testPolygon;
        testPolygon.geofence = testGeofence;
        testPolygon.numHoles = 0;

        int res = 7;
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&testPolygon, res);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&testPolygon, res, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 4499,
                 "got expected polyfill size (h3-js#67)");
        free(hexagons);
    }

    // 2nd test case from h3-js#67
    TEST(h3js_67_2nd) {
        double east = H3_EXPORT(degsToRads)(-57.65625);
        double north = H3_EXPORT(degsToRads)(-34.30714385628804);
        double south = H3_EXPORT(degsToRads)(-35.4606699514953);
        double west = H3_EXPORT(degsToRads)(-59.0625);

        GeoCoord testVerts[] = {
            {north, east}, {south, east}, {south, west}, {north, west}};
        Geofence testGeofence = {.numVerts = 4, .verts = testVerts};
        GeoPolygon testPolygon;
        testPolygon.geofence = testGeofence;
        testPolygon.numHoles = 0;

        int res = 7;
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&testPolygon, res);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&testPolygon, res, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 4609,
                 "got expected polyfill size (h3-js#67, 2nd case)");
        free(hexagons);
    }

    // https://github.com/uber/h3/issues/136
    TEST(h3_136) {
        GeoCoord testVerts[] = {{0.10068990369902957, 0.8920772174196191},
                                {0.10032914690616246, 0.8915914753447348},
                                {0.10033349237998787, 0.8915860128746426},
                                {0.10069496685903621, 0.8920742194546231}};
        Geofence testGeofence = {.numVerts = 4, .verts = testVerts};
        GeoPolygon testPolygon;
        testPolygon.geofence = testGeofence;
        testPolygon.numHoles = 0;

        int res = 13;
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&testPolygon, res);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&testPolygon, res, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 4353, "got expected polyfill size");
        free(hexagons);
    }
}
