/*
 * Copyright 2017-2018 Uber Technologies, Inc.
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

// Fixtures
static GeoCoord sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};
static Geofence sfGeofence = {.numVerts = 6, .verts = sfVerts};
static GeoPolygon sfGeoPolygon;

static GeoCoord holeVerts[] = {{0.6595072188743, -2.1371053983433},
                               {0.6591482046471, -2.1373141048153},
                               {0.6592295020837, -2.1365222838402}};
static Geofence holeGeofence = {.numVerts = 3, .verts = holeVerts};
static GeoPolygon holeGeoPolygon;

static GeoCoord emptyVerts[] = {{0.659966917655, -2.1364398519394},
                                {0.659966917655, -2.1364398519395},
                                {0.659966917655, -2.1364398519396}};
static Geofence emptyGeofence = {.numVerts = 3, .verts = emptyVerts};
static GeoPolygon emptyGeoPolygon;

/**
 * Return true if the cell crosses the meridian.
 */
static bool isTransmeridianCell(H3Index h) {
    GeoBoundary bndry;
    H3_EXPORT(h3ToGeoBoundary)(h, &bndry);

    double minLon = M_PI, maxLon = -M_PI;
    for (int i = 0; i < bndry.numVerts; i++) {
        if (bndry.verts[i].lon < minLon) minLon = bndry.verts[i].lon;
        if (bndry.verts[i].lon > maxLon) maxLon = bndry.verts[i].lon;
    }

    return maxLon - minLon > M_PI - (M_PI / 4);
}

static void fillIndex_assertions(H3Index h) {
    if (isTransmeridianCell(h)) {
        // TODO: these do not work correctly
        return;
    }

    int currentRes = H3_EXPORT(h3GetResolution)(h);
    // TODO: Not testing more than one depth because the assertions fail.
    for (int nextRes = currentRes; nextRes <= currentRes + 1; nextRes++) {
        GeoBoundary bndry;
        H3_EXPORT(h3ToGeoBoundary)(h, &bndry);
        GeoPolygon polygon = {
            .geofence = {.numVerts = bndry.numVerts, .verts = bndry.verts},
            .numHoles = 0,
            .holes = 0};

        int polyfillSize = H3_EXPORT(maxPolyfillSize)(&polygon, nextRes);
        H3Index* polyfillOut = calloc(polyfillSize, sizeof(H3Index));
        H3_EXPORT(polyfill)(&polygon, nextRes, polyfillOut);

        int polyfillCount = countActualHexagons(polyfillOut, polyfillSize);

        int childrenSize = H3_EXPORT(maxH3ToChildrenSize)(h, nextRes);
        H3Index* children = calloc(childrenSize, sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(h, nextRes, children);

        int h3ToChildrenCount = countActualHexagons(children, childrenSize);

        t_assert(polyfillCount == h3ToChildrenCount,
                 "Polyfill count matches h3ToChildren count");

        for (int i = 0; i < childrenSize; i++) {
            bool found = false;
            if (children[i] == H3_NULL) continue;
            for (int j = 0; j < polyfillSize; j++) {
                if (polyfillOut[j] == children[i]) {
                    found = true;
                    break;
                }
            }
            t_assert(found,
                     "All indexes match between polyfill and h3ToChildren");
        }

        free(polyfillOut);
        free(children);
    }
}

SUITE(polyfill) {
    sfGeoPolygon.geofence = sfGeofence;
    sfGeoPolygon.numHoles = 0;

    holeGeoPolygon.geofence = sfGeofence;
    holeGeoPolygon.numHoles = 1;
    holeGeoPolygon.holes = &holeGeofence;

    emptyGeoPolygon.geofence = emptyGeofence;
    emptyGeoPolygon.numHoles = 0;

    TEST(maxPolyfillSize) {
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&sfGeoPolygon, 9);
        t_assert(numHexagons == 5613, "got expected max polyfill size");

        numHexagons = H3_EXPORT(maxPolyfillSize)(&holeGeoPolygon, 9);
        t_assert(numHexagons == 5613, "got expected max polyfill size (hole)");

        numHexagons = H3_EXPORT(maxPolyfillSize)(&emptyGeoPolygon, 9);
        t_assert(numHexagons == 15, "got expected max polyfill size (empty)");
    }

    TEST(polyfill) {
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&sfGeoPolygon, 9);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&sfGeoPolygon, 9, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 1253, "got expected polyfill size");
        free(hexagons);
    }

    TEST(polyfillHole) {
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&holeGeoPolygon, 9);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&holeGeoPolygon, 9, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 1214,
                 "got expected polyfill size (hole)");
        free(hexagons);
    }

    TEST(polyfillEmpty) {
        int numHexagons = H3_EXPORT(maxPolyfillSize)(&emptyGeoPolygon, 9);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&emptyGeoPolygon, 9, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 0, "got expected polyfill size (empty)");
        free(hexagons);
    }

    TEST(polyfillExact) {
        GeoCoord somewhere = {1, 2};
        H3Index origin = H3_EXPORT(geoToH3)(&somewhere, 9);
        GeoBoundary boundary;
        H3_EXPORT(h3ToGeoBoundary)(origin, &boundary);

        GeoCoord* verts = calloc(boundary.numVerts + 1, sizeof(GeoCoord));
        for (int i = 0; i < boundary.numVerts; i++) {
            verts[i] = boundary.verts[i];
        }
        verts[boundary.numVerts] = boundary.verts[0];

        Geofence someGeofence;
        someGeofence.numVerts = boundary.numVerts + 1;
        someGeofence.verts = verts;
        GeoPolygon someHexagon;
        someHexagon.geofence = someGeofence;
        someHexagon.numHoles = 0;

        int numHexagons = H3_EXPORT(maxPolyfillSize)(&someHexagon, 9);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&someHexagon, 9, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 1, "got expected polyfill size (1)");
        free(hexagons);
        free(verts);
    }

    TEST(polyfillTransmeridian) {
        GeoCoord primeMeridianVerts[] = {
            {0.01, 0.01}, {0.01, -0.01}, {-0.01, -0.01}, {-0.01, 0.01}};
        Geofence primeMeridianGeofence = {.numVerts = 4,
                                          .verts = primeMeridianVerts};
        GeoPolygon primeMeridianGeoPolygon = {.geofence = primeMeridianGeofence,
                                              .numHoles = 0};

        GeoCoord transMeridianVerts[] = {{0.01, -M_PI + 0.01},
                                         {0.01, M_PI - 0.01},
                                         {-0.01, M_PI - 0.01},
                                         {-0.01, -M_PI + 0.01}};
        Geofence transMeridianGeofence = {.numVerts = 4,
                                          .verts = transMeridianVerts};
        GeoPolygon transMeridianGeoPolygon = {.geofence = transMeridianGeofence,
                                              .numHoles = 0};

        GeoCoord transMeridianHoleVerts[] = {{0.005, -M_PI + 0.005},
                                             {0.005, M_PI - 0.005},
                                             {-0.005, M_PI - 0.005},
                                             {-0.005, -M_PI + 0.005}};
        Geofence transMeridianHoleGeofence = {.numVerts = 4,
                                              .verts = transMeridianHoleVerts};
        GeoPolygon transMeridianHoleGeoPolygon = {
            .geofence = transMeridianGeofence,
            .numHoles = 1,
            .holes = &transMeridianHoleGeofence};
        GeoPolygon transMeridianFilledHoleGeoPolygon = {
            .geofence = transMeridianHoleGeofence, .numHoles = 0};

        int expectedSize;

        // Prime meridian case
        expectedSize = 4228;
        int numHexagons =
            H3_EXPORT(maxPolyfillSize)(&primeMeridianGeoPolygon, 7);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&primeMeridianGeoPolygon, 7, hexagons);
        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == expectedSize,
                 "got expected polyfill size (prime meridian)");

        // Transmeridian case
        // This doesn't exactly match the prime meridian count because of slight
        // differences in hex size and grid offset between the two cases
        expectedSize = 4238;
        numHexagons = H3_EXPORT(maxPolyfillSize)(&transMeridianGeoPolygon, 7);
        H3Index* hexagonsTM = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&transMeridianGeoPolygon, 7, hexagonsTM);
        actualNumHexagons = countActualHexagons(hexagonsTM, numHexagons);

        t_assert(actualNumHexagons == expectedSize,
                 "got expected polyfill size (transmeridian)");

        // Transmeridian filled hole case -- only needed for calculating hole
        // size
        numHexagons =
            H3_EXPORT(maxPolyfillSize)(&transMeridianFilledHoleGeoPolygon, 7);
        H3Index* hexagonsTMFH = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)
        (&transMeridianFilledHoleGeoPolygon, 7, hexagonsTMFH);
        int actualNumHoleHexagons =
            countActualHexagons(hexagonsTMFH, numHexagons);

        // Transmeridian hole case
        numHexagons =
            H3_EXPORT(maxPolyfillSize)(&transMeridianHoleGeoPolygon, 7);
        H3Index* hexagonsTMH = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&transMeridianHoleGeoPolygon, 7, hexagonsTMH);
        actualNumHexagons = countActualHexagons(hexagonsTMH, numHexagons);

        t_assert(actualNumHexagons == expectedSize - actualNumHoleHexagons,
                 "got expected polyfill size (transmeridian hole)");

        free(hexagons);
        free(hexagonsTM);
        free(hexagonsTMFH);
        free(hexagonsTMH);
    }

    TEST(polyfillTransmeridianComplex) {
        // This polygon is "complex" in that it has > 4 vertices - this
        // tests for a bug that was taking the max and min longitude as
        // the bounds for transmeridian polygons
        GeoCoord verts[] = {{0.1, -M_PI + 0.00001},  {0.1, M_PI - 0.00001},
                            {0.05, M_PI - 0.2},      {-0.1, M_PI - 0.00001},
                            {-0.1, -M_PI + 0.00001}, {-0.05, -M_PI + 0.2}};
        Geofence geofence = {.numVerts = 6, .verts = verts};
        GeoPolygon polygon = {.geofence = geofence, .numHoles = 0};

        int numHexagons = H3_EXPORT(maxPolyfillSize)(&polygon, 4);

        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));
        H3_EXPORT(polyfill)(&polygon, 4, hexagons);

        int actualNumHexagons = countActualHexagons(hexagons, numHexagons);

        t_assert(actualNumHexagons == 1204,
                 "got expected polyfill size (complex transmeridian)");

        free(hexagons);
    }

    TEST(polyfillPentagon) {
        H3Index pentagon;
        setH3Index(&pentagon, 9, 24, 0);
        GeoCoord coord;
        H3_EXPORT(h3ToGeo)(pentagon, &coord);

        // Length of half an edge of the polygon, in radians
        double edgeLength2 = H3_EXPORT(degsToRads)(0.001);

        GeoCoord boundingTopRigt = coord;
        boundingTopRigt.lat += edgeLength2;
        boundingTopRigt.lon += edgeLength2;

        GeoCoord boundingTopLeft = coord;
        boundingTopLeft.lat += edgeLength2;
        boundingTopLeft.lon -= edgeLength2;

        GeoCoord boundingBottomRight = coord;
        boundingBottomRight.lat -= edgeLength2;
        boundingBottomRight.lon += edgeLength2;

        GeoCoord boundingBottomLeft = coord;
        boundingBottomLeft.lat -= edgeLength2;
        boundingBottomLeft.lon -= edgeLength2;

        GeoCoord verts[] = {boundingBottomLeft, boundingTopLeft,
                            boundingTopRigt, boundingBottomRight};

        Geofence geofence;
        geofence.verts = verts;
        geofence.numVerts = 4;

        GeoPolygon polygon;
        polygon.geofence = geofence;
        polygon.numHoles = 0;

        int numHexagons = H3_EXPORT(maxPolyfillSize)(&polygon, 9);
        H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

        H3_EXPORT(polyfill)(&polygon, 9, hexagons);

        int found = 0;
        int numPentagons = 0;
        for (int i = 0; i < numHexagons; i++) {
            if (hexagons[i] != 0) {
                found++;
            }
            if (H3_EXPORT(h3IsPentagon)(hexagons[i])) {
                numPentagons++;
            }
        }
        t_assert(found == 1, "one index found");
        t_assert(numPentagons == 1, "one pentagon found");
        free(hexagons);
    }

    TEST(fillIndex) {
        iterateAllIndexesAtRes(0, fillIndex_assertions);
        iterateAllIndexesAtRes(1, fillIndex_assertions);
        iterateAllIndexesAtRes(2, fillIndex_assertions);
    }
}
