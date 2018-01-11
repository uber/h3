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
#include "algos.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "test.h"

// Fixtures
GeoCoord sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};
Geofence sfGeofence;
GeoPolygon sfGeoPolygon;
GeoCoord holeVerts[] = {{0.6595072188743, -2.1371053983433},
                        {0.6591482046471, -2.1373141048153},
                        {0.6592295020837, -2.1365222838402}};
Geofence holeGeofence;
GeoPolygon holeGeoPolygon;
GeoCoord emptyVerts[] = {{0.659966917655, -2.1364398519394},
                         {0.659966917655, -2.1364398519395},
                         {0.659966917655, -2.1364398519396}};
Geofence emptyGeofence;
GeoPolygon emptyGeoPolygon;

GeoCoord primeMeridianVerts[] = {
    {0.01, 0.01}, {0.01, -0.01}, {-0.01, -0.01}, {-0.01, 0.01}};
Geofence primeMeridianGeofence;
GeoPolygon primeMeridianGeoPolygon;

GeoCoord transMeridianVerts[] = {{0.01, -M_PI + 0.01},
                                 {0.01, M_PI - 0.01},
                                 {-0.01, M_PI - 0.01},
                                 {-0.01, -M_PI + 0.01}};
Geofence transMeridianGeofence;
GeoPolygon transMeridianGeoPolygon;

GeoCoord transMeridianHoleVerts[] = {{0.005, -M_PI + 0.005},
                                     {0.005, M_PI - 0.005},
                                     {-0.005, M_PI - 0.005},
                                     {-0.005, -M_PI + 0.005}};
Geofence transMeridianHoleGeofence;
GeoPolygon transMeridianHoleGeoPolygon;
GeoPolygon transMeridianFilledHoleGeoPolygon;

BEGIN_TESTS(polyfill);

sfGeofence.numVerts = 6;
sfGeofence.verts = sfVerts;
sfGeoPolygon.geofence = sfGeofence;
sfGeoPolygon.numHoles = 0;

holeGeofence.numVerts = 3;
holeGeofence.verts = holeVerts;
holeGeoPolygon.geofence = sfGeofence;
holeGeoPolygon.numHoles = 1;
holeGeoPolygon.holes = &holeGeofence;

emptyGeofence.numVerts = 3;
emptyGeofence.verts = emptyVerts;
emptyGeoPolygon.geofence = emptyGeofence;
emptyGeoPolygon.numHoles = 0;

primeMeridianGeofence.numVerts = 4;
primeMeridianGeofence.verts = primeMeridianVerts;
primeMeridianGeoPolygon.geofence = primeMeridianGeofence;
primeMeridianGeoPolygon.numHoles = 0;

transMeridianGeofence.numVerts = 4;
transMeridianGeofence.verts = transMeridianVerts;
transMeridianGeoPolygon.geofence = transMeridianGeofence;
transMeridianGeoPolygon.numHoles = 0;

transMeridianHoleGeofence.numVerts = 4;
transMeridianHoleGeofence.verts = transMeridianHoleVerts;
transMeridianHoleGeoPolygon.geofence = transMeridianGeofence;
transMeridianHoleGeoPolygon.numHoles = 1;
transMeridianHoleGeoPolygon.holes = &transMeridianHoleGeofence;

transMeridianFilledHoleGeoPolygon.geofence = transMeridianHoleGeofence;
transMeridianFilledHoleGeoPolygon.numHoles = 0;

TEST(maxPolyfillSize) {
    int numHexagons = H3_EXPORT(maxPolyfillSize)(&sfGeoPolygon, 9);
    t_assert(numHexagons == 3169, "got expected max polyfill size");

    numHexagons = H3_EXPORT(maxPolyfillSize)(&holeGeoPolygon, 9);
    t_assert(numHexagons == 3169, "got expected max polyfill size (hole)");

    numHexagons = H3_EXPORT(maxPolyfillSize)(&emptyGeoPolygon, 9);
    t_assert(numHexagons == 1, "got expected max polyfill size (empty)");
}

TEST(polyfill) {
    int numHexagons = H3_EXPORT(maxPolyfillSize)(&sfGeoPolygon, 9);
    H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

    H3_EXPORT(polyfill)(&sfGeoPolygon, 9, hexagons);
    int actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagons[i] != 0) {
            actualNumHexagons++;
        }
    }

    t_assert(actualNumHexagons == 1253, "got expected polyfill size");
    free(hexagons);
}

TEST(polyfillHole) {
    int numHexagons = H3_EXPORT(maxPolyfillSize)(&holeGeoPolygon, 9);
    H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

    H3_EXPORT(polyfill)(&holeGeoPolygon, 9, hexagons);
    int actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagons[i] != 0) {
            actualNumHexagons++;
        }
    }

    t_assert(actualNumHexagons == 1214, "got expected polyfill size (hole)");
    free(hexagons);
}

TEST(polyfillEmpty) {
    int numHexagons = H3_EXPORT(maxPolyfillSize)(&emptyGeoPolygon, 9);
    H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

    H3_EXPORT(polyfill)(&emptyGeoPolygon, 9, hexagons);
    int actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagons[i] != 0) {
            actualNumHexagons++;
        }
    }

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
    int actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagons[i] != 0) {
            t_assert(hexagons[i] == origin, "Got origin back");
            actualNumHexagons++;
        }
    }

    t_assert(actualNumHexagons == 1, "got expected polyfill size (1)");
    free(hexagons);
    free(verts);
}

TEST(polyfillTransmeridian) {
    int expectedSize;

    // Prime meridian case
    expectedSize = 4228;
    int numHexagons = H3_EXPORT(maxPolyfillSize)(&primeMeridianGeoPolygon, 7);
    H3Index* hexagons = calloc(numHexagons, sizeof(H3Index));

    H3_EXPORT(polyfill)(&primeMeridianGeoPolygon, 7, hexagons);
    int actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagons[i] != 0) {
            actualNumHexagons++;
        }
    }

    t_assert(actualNumHexagons == expectedSize,
             "got expected polyfill size (prime meridian)");

    // Transmeridian case
    // This doesn't exactly match the prime meridian count because of slight
    // differences in hex size and grid offset between the two cases
    expectedSize = 4238;
    numHexagons = H3_EXPORT(maxPolyfillSize)(&transMeridianGeoPolygon, 7);
    H3Index* hexagonsTM = calloc(numHexagons, sizeof(H3Index));

    H3_EXPORT(polyfill)(&transMeridianGeoPolygon, 7, hexagonsTM);
    actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagonsTM[i] != 0) {
            actualNumHexagons++;
        }
    }

    t_assert(actualNumHexagons == expectedSize,
             "got expected polyfill size (transmeridian)");

    // Transmeridian filled hole case -- only needed for calculating hole size
    numHexagons =
        H3_EXPORT(maxPolyfillSize)(&transMeridianFilledHoleGeoPolygon, 7);
    H3Index* hexagonsTMFH = calloc(numHexagons, sizeof(H3Index));

    H3_EXPORT(polyfill)(&transMeridianFilledHoleGeoPolygon, 7, hexagonsTMFH);
    int actualNumHoleHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagonsTMFH[i] != 0) {
            actualNumHoleHexagons++;
        }
    }

    // Transmeridian hole case
    numHexagons = H3_EXPORT(maxPolyfillSize)(&transMeridianHoleGeoPolygon, 7);
    H3Index* hexagonsTMH = calloc(numHexagons, sizeof(H3Index));

    H3_EXPORT(polyfill)(&transMeridianHoleGeoPolygon, 7, hexagonsTMH);
    actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagonsTMH[i] != 0) {
            actualNumHexagons++;
        }
    }

    t_assert(actualNumHexagons == expectedSize - actualNumHoleHexagons,
             "got expected polyfill size (transmeridian hole)");

    free(hexagons);
    free(hexagonsTM);
    free(hexagonsTMFH);
    free(hexagonsTMH);
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

    GeoCoord verts[] = {boundingBottomLeft, boundingTopLeft, boundingTopRigt,
                        boundingBottomRight};

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

TEST(_pointInPolyContainsLoop) {
    GeoCoord somewhere = {1, 2};

    BBox bbox;
    bboxFromGeofence(&sfGeofence, &bbox);

    t_assert(_pointInPolyContainsLoop(&sfGeofence, &bbox, &sfVerts[0]) == false,
             "contains exact");
    t_assert(_pointInPolyContainsLoop(&sfGeofence, &bbox, &sfVerts[4]) == true,
             "contains exact4");
    t_assert(_pointInPolyContainsLoop(&sfGeofence, &bbox, &somewhere) == false,
             "contains somewhere else");
}

TEST(_pointInPolyContainsLoopTransmeridian) {
    GeoCoord eastPoint = {0.001, -M_PI + 0.001};
    GeoCoord eastPointOutside = {0.001, -M_PI + 0.1};
    GeoCoord westPoint = {0.001, M_PI - 0.001};
    GeoCoord westPointOutside = {0.001, M_PI - 0.1};

    BBox bbox;
    bboxFromGeofence(&transMeridianGeofence, &bbox);

    t_assert(_pointInPolyContainsLoop(&transMeridianGeofence, &bbox,
                                      &westPoint) == true,
             "contains point to the west of the antimeridian");
    t_assert(_pointInPolyContainsLoop(&transMeridianGeofence, &bbox,
                                      &eastPoint) == true,
             "contains point to the east of the antimeridian");
    t_assert(_pointInPolyContainsLoop(&transMeridianGeofence, &bbox,
                                      &westPointOutside) == false,
             "does not contain outside point to the west of the antimeridian");
    t_assert(_pointInPolyContainsLoop(&transMeridianGeofence, &bbox,
                                      &eastPointOutside) == false,
             "does not contain outside point to the east of the antimeridian");
}

END_TESTS();
