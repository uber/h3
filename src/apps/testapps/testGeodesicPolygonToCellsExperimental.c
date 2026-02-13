/*
 * Copyright 2025 Uber Technologies, Inc.
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

/** @file testGeodesicPolygonToCellsExperimental.c
 * @brief Tests the geodesic polygon-to-cells experimental API surface.
 */

/*
 * Geodesic polygonToCellsExperimental tests.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "h3Index.h"
#include "polyfill.h"
#include "polygon.h"
#include "test.h"
#include "utility.h"

// Fixtures - Large scale polygons for performance testing
// Large polygon covering western United States (approx 1000km x 1000km)
static LatLng sfVerts[] = {{0.698132, -2.094395},   // 40°N, 120°W
                           {0.698132, -1.919862},   // 40°N, 110°W
                           {0.610865, -1.919862},   // 35°N, 110°W
                           {0.610865, -2.094395},   // 35°N, 120°W
                           {0.628319, -2.181662},   // 36°N, 125°W
                           {0.663225, -2.181662},   // 38°N, 125°W
                           {0.680678, -2.138763},   // 39°N, 122.5°W
                           {0.680678, -2.050796}};  // 39°N, 117.5°W
static GeoLoop sfGeoLoop = {.numVerts = 8, .verts = sfVerts};
static GeoPolygon sfGeoPolygon;

// Large hole covering parts of Nevada/Utah (approx 300km x 200km)
static LatLng holeVerts[] = {{0.663225, -2.007129},   // 38°N, 115°W
                             {0.645772, -2.007129},   // 37°N, 115°W
                             {0.645772, -1.989676},   // 37°N, 114°W
                             {0.663225, -1.989676},   // 38°N, 114°W
                             {0.680678, -1.998403}};  // 39°N, 114.5°W
static GeoLoop holeGeoLoop = {.numVerts = 5, .verts = holeVerts};
static GeoPolygon holeGeoPolygon;

static LatLng emptyVerts[] = {{0.659966917655, -2.1364398519394},
                              {0.659966917656, -2.1364398519395},
                              {0.659966917657, -2.1364398519396}};
static GeoLoop emptyGeoLoop = {.numVerts = 3, .verts = emptyVerts};
static GeoPolygon emptyGeoPolygon;

static LatLng invalidVerts[] = {{INFINITY, INFINITY}, {-INFINITY, -INFINITY}};
static GeoLoop invalidGeoLoop = {.numVerts = 2, .verts = invalidVerts};
static GeoPolygon invalidGeoPolygon;

static LatLng outOfBoundsVert[] = {{-2000, -2000}};
static GeoLoop outOfBoundsVertGeoLoop = {.numVerts = 1,
                                         .verts = outOfBoundsVert};
static GeoPolygon outOfBoundsVertGeoPolygon;

static LatLng invalid2Verts[] = {{NAN, NAN}, {-NAN, -NAN}};
static GeoLoop invalid2GeoLoop = {.numVerts = 2, .verts = invalid2Verts};
static GeoPolygon invalid2GeoPolygon;

static GeoLoop nullGeoLoop = {.numVerts = 0};
static GeoPolygon nullGeoPolygon;

static LatLng pointVerts[] = {{0.6595072188743, -2.1371053983433}};
static GeoLoop pointGeoLoop = {.numVerts = 1, .verts = pointVerts};
static GeoPolygon pointGeoPolygon;

static LatLng lineVerts[] = {{0.6595072188743, -2.1371053983433},
                             {0.6591482046471, -2.1373141048153}};
static GeoLoop lineGeoLoop = {.numVerts = 2, .verts = lineVerts};
static GeoPolygon lineGeoPolygon;

// Large transmeridian polygon covering Pacific (approx 6000km x 4000km)
static LatLng transVerts[] = {{0.698132, -2.967060},   // 40°N, 170°W
                              {0.698132, 2.967060},    // 40°N, 170°E
                              {0.174533, 2.967060},    // 10°N, 170°E
                              {0.174533, -2.967060},   // 10°N, 170°W
                              {0.436332, -3.141593},   // 25°N, 180°W
                              {0.610865, -3.141593}};  // 35°N, 180°W
static GeoLoop transGeoLoop = {.numVerts = ARRAY_SIZE(transVerts),
                               .verts = transVerts};
static GeoPolygon transGeoPolygon;

static GeoPolygon nullHoleGeoPolygon;
static GeoPolygon pointHoleGeoPolygon;
static GeoPolygon lineHoleGeoPolygon;

static int64_t geodesicFillCount(const GeoPolygon *polygon, int res,
                                 uint32_t mode) {
    uint32_t flags = mode;
    FLAG_SET_GEODESIC(flags);

    int64_t size = 0;
    t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(polygon, res,
                                                                 flags, &size));

    H3Index *cells = size > 0 ? calloc(size, sizeof(H3Index)) : NULL;
    t_assert(size == 0 || cells != NULL, "allocated output buffer");

    t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(polygon, res, flags,
                                                          size, cells));

    int64_t count = countNonNullIndexes(cells, size);
    free(cells);
    return count;
}

static H3Error geodesicFill(const GeoPolygon *polygon, int res, uint32_t mode,
                            int64_t *sizeOut, H3Index **cellsOut) {
    uint32_t flags = mode;
    FLAG_SET_GEODESIC(flags);

    int64_t size = 0;
    H3Error err = H3_EXPORT(maxPolygonToCellsSizeExperimental)(polygon, res,
                                                               flags, &size);
    if (err != E_SUCCESS) {
        return err;
    }

    H3Index *cells = size > 0 ? calloc(size, sizeof(H3Index)) : NULL;
    if (size > 0) {
        t_assert(cells != NULL, "allocated output buffer");
    }

    err =
        H3_EXPORT(polygonToCellsExperimental)(polygon, res, flags, size, cells);
    if (err != E_SUCCESS) {
        free(cells);
        return err;
    }

    if (sizeOut) {
        *sizeOut = size;
    } else {
        t_assert(size == 0 || cells != NULL, "buffer consistent");
    }
    if (cellsOut) {
        *cellsOut = cells;
    } else if (cells) {
        free(cells);
    }
    return E_SUCCESS;
}

SUITE(geodesicPolygonToCellsExperimental) {
    sfGeoPolygon.geoloop = sfGeoLoop;
    sfGeoPolygon.numHoles = 0;

    holeGeoPolygon.geoloop = sfGeoLoop;
    holeGeoPolygon.numHoles = 1;
    holeGeoPolygon.holes = &holeGeoLoop;

    nullHoleGeoPolygon.geoloop = sfGeoLoop;
    nullHoleGeoPolygon.numHoles = 1;
    nullHoleGeoPolygon.holes = &nullGeoLoop;

    pointHoleGeoPolygon.geoloop = sfGeoLoop;
    pointHoleGeoPolygon.numHoles = 1;
    pointHoleGeoPolygon.holes = &pointGeoLoop;

    lineHoleGeoPolygon.geoloop = sfGeoLoop;
    lineHoleGeoPolygon.numHoles = 1;
    lineHoleGeoPolygon.holes = &lineGeoLoop;

    emptyGeoPolygon.geoloop = emptyGeoLoop;
    emptyGeoPolygon.numHoles = 0;

    invalidGeoPolygon.geoloop = invalidGeoLoop;
    invalidGeoPolygon.numHoles = 0;

    invalid2GeoPolygon.geoloop = invalid2GeoLoop;
    invalid2GeoPolygon.numHoles = 0;

    outOfBoundsVertGeoPolygon.geoloop = outOfBoundsVertGeoLoop;
    outOfBoundsVertGeoPolygon.numHoles = 0;

    nullGeoPolygon.geoloop = nullGeoLoop;
    nullGeoPolygon.numHoles = 0;

    pointGeoPolygon.geoloop = pointGeoLoop;
    pointGeoPolygon.numHoles = 0;

    lineGeoPolygon.geoloop = lineGeoLoop;
    lineGeoPolygon.numHoles = 0;

    transGeoPolygon.geoloop = transGeoLoop;
    transGeoPolygon.numHoles = 0;

    TEST(edgePoint) {
        static LatLng point[] = {{-0.0002458237579169511, 0.12401960784313724}};
        static GeoLoop pointLoop = {.numVerts = 1, .verts = point};
        static GeoPolygon pointPolygon;
        pointPolygon.geoloop = pointLoop;
        pointPolygon.numHoles = 0;

        int64_t count =
            geodesicFillCount(&pointPolygon, 1, CONTAINMENT_OVERLAPPING);
        t_assert(count == 1, "point should produce exactly one cell");
    }

    TEST(geodesicZeroSize) {
        uint32_t flags = CONTAINMENT_OVERLAPPING;
        FLAG_SET_GEODESIC(flags);
        t_assert(H3_EXPORT(polygonToCellsExperimental)(
                     &sfGeoPolygon, 0, flags, 0, NULL) == E_MEMORY_BOUNDS,
                 "requires capacity for geodesic polyfill");
    }

    TEST(geodesicFullContainment) {
        int64_t count = geodesicFillCount(&sfGeoPolygon, 5, CONTAINMENT_FULL);
        t_assert(count == 2198, "matches expected geodesic full count");
    }

    TEST(geodesicOverlapping) {
        int64_t count =
            geodesicFillCount(&sfGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assert(count == 2469, "matches expected geodesic overlapping count");
    }

    TEST(geodesicHoleFullContainment) {
        int64_t count = geodesicFillCount(&holeGeoPolygon, 5, CONTAINMENT_FULL);
        t_assert(count == 2123, "hole respected in full containment");
    }

    TEST(geodesicHoleOverlapping) {
        int64_t count =
            geodesicFillCount(&holeGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assert(count == 2436, "hole respected in overlapping containment");
    }

    TEST(geodesicTransmeridianFull) {
        int64_t count =
            geodesicFillCount(&transGeoPolygon, 4, CONTAINMENT_FULL);
        t_assert(count == 2246, "large transmeridian full count");
    }

    TEST(geodesicTransmeridianOverlapping) {
        int64_t count =
            geodesicFillCount(&transGeoPolygon, 4, CONTAINMENT_OVERLAPPING);
        t_assert(count == 2554, "large transmeridian overlapping count");
    }

    TEST(geodesicEmptyPolygon) {
        int64_t count =
            geodesicFillCount(&emptyGeoPolygon, 5, CONTAINMENT_FULL);
        t_assert(count == 0, "empty polygon yields no cells");
    }

    TEST(geodesicNullPolygon) {
        int64_t count =
            geodesicFillCount(&nullGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assert(count == 0, "null polygon yields no cells");
    }

    TEST(geodesicPointPolygon) {
        int64_t fullCount =
            geodesicFillCount(&pointGeoPolygon, 5, CONTAINMENT_FULL);
        int64_t overlapCount =
            geodesicFillCount(&pointGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assert(fullCount == 0, "point polygon full containment empty");
        t_assert(overlapCount == 1, "point polygon overlapping includes one");
    }

    TEST(geodesicLinePolygon) {
        int64_t fullCount =
            geodesicFillCount(&lineGeoPolygon, 5, CONTAINMENT_FULL);
        int64_t overlapCount =
            geodesicFillCount(&lineGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assert(fullCount == 0, "line polygon full containment empty");
        t_assert(overlapCount == 2, "line polygon overlapping captures edge");
    }

    TEST(geodesicInvalidLoopValues) {
        int64_t size = 0;
        H3Error err =
            geodesicFill(&invalidGeoPolygon, 5, CONTAINMENT_FULL, &size, NULL);
        t_assert(err == E_SUCCESS && size == 0,
                 "invalid range verts handled with empty result");

        err =
            geodesicFill(&invalid2GeoPolygon, 5, CONTAINMENT_FULL, &size, NULL);
        t_assert(err == E_SUCCESS && size == 0,
                 "NaN verts handled with empty result");
    }

    TEST(geodesicMismatchedHoles) {
        // Test case: numHoles > 0 but holes pointer is NULL
        GeoPolygon mismatchedPolygon = {
            .geoloop = sfGeoLoop,
            .numHoles = 1,  // Says there's 1 hole
            .holes = NULL   // But holes pointer is NULL
        };

        int64_t size = 0;
        H3Error err =
            geodesicFill(&mismatchedPolygon, 5, CONTAINMENT_FULL, &size, NULL);
        t_assert(err == E_DOMAIN,
                 "numHoles > 0 with NULL holes returns E_DOMAIN");
    }

    TEST(geodesicNullHolePolygon) {
        // Test polygon with a hole that has 0 vertices
        int64_t size = 0;
        H3Error err =
            geodesicFill(&nullHoleGeoPolygon, 5, CONTAINMENT_FULL, &size, NULL);
        t_assert(err == E_DOMAIN, "hole with 0 vertices returns E_DOMAIN");
    }

    TEST(geodesicPointHolePolygon) {
        int64_t count =
            geodesicFillCount(&pointHoleGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assert(count == 2469, "point hole ignored for overlap");
    }

    TEST(geodesicLineHolePolygon) {
        int64_t count =
            geodesicFillCount(&lineHoleGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assert(count == 2469, "line hole ignored for overlap");
    }

    TEST(geodesicInvalidContainmentModes) {
        int64_t size = 0;
        H3Error err =
            geodesicFill(&sfGeoPolygon, 5, CONTAINMENT_CENTER, &size, NULL);
        t_assert(err == E_OPTION_INVALID, "center mode rejected for geodesic");

        err = geodesicFill(&sfGeoPolygon, 5, CONTAINMENT_OVERLAPPING_BBOX,
                           &size, NULL);
        t_assert(err == E_OPTION_INVALID,
                 "overlapping bbox mode rejected for geodesic");
    }

    TEST(geodesicMaxSizeNoUnderAllocation) {
        LatLng reproVerts[] = {
            {H3_EXPORT(degsToRads)(20.0), H3_EXPORT(degsToRads)(-70.0)},
            {H3_EXPORT(degsToRads)(20.0), H3_EXPORT(degsToRads)(70.0)},
            {H3_EXPORT(degsToRads)(-5.0), H3_EXPORT(degsToRads)(0.0)}};
        GeoLoop reproGeoLoop = {.numVerts = 3, .verts = reproVerts};
        GeoPolygon reproGeoPolygon = {.geoloop = reproGeoLoop, .numHoles = 0};

        uint32_t flags = CONTAINMENT_OVERLAPPING;
        FLAG_SET_GEODESIC(flags);

        int64_t size = 0;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &reproGeoPolygon, 1, flags, &size));
        t_assert(size > 0, "max size estimate is non-zero");

        H3Index *out = calloc(size, sizeof(H3Index));
        t_assert(out != NULL, "allocated output buffer");

        H3Error err = H3_EXPORT(polygonToCellsExperimental)(&reproGeoPolygon, 1,
                                                            flags, size, out);
        t_assert(err == E_SUCCESS,
                 "maxPolygonToCellsSizeExperimental must not under-allocate");

        free(out);
    }
}
