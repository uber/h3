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

// https://github.com/uber/h3/issues/136

static GeoCoord testVerts[] = {{0.10068990369902957, 0.8920772174196191},
                               {0.10032914690616246, 0.8915914753447348},
                               {0.10033349237998787, 0.8915860128746426},
                               {0.10069496685903621, 0.8920742194546231}};
static Geofence testGeofence = {.numVerts = 4, .verts = testVerts};
static GeoPolygon testPolygon;

static int countActualHexagons(H3Index* hexagons, int numHexagons) {
    int actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagons[i] != 0) {
            actualNumHexagons++;
        }
    }
    return actualNumHexagons;
}

SUITE(polyfill_gh136) {
    testPolygon.geofence = testGeofence;
    testPolygon.numHoles = 0;

    TEST(gh136) {
        int res = 13;
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&testPolygon, res);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&testPolygon, res, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 4353, "got expected polyfill size");
        free(hexagons);
    }
}
