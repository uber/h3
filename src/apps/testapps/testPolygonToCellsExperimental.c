/*
 * Copyright 2017-2018, 2020-2021 Uber Technologies, Inc.
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

/** @file testPolygonToCellsExperimental.c
 * @brief Tests the experimental polygon-to-cells traversal algorithm.
 */

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "polyfill.h"
#include "polygon.h"
#include "test.h"
#include "utility.h"

// Fixtures
static LatLng sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};
static GeoLoop sfGeoLoop = {.numVerts = 6, .verts = sfVerts};
static GeoPolygon sfGeoPolygon;

static LatLng holeVerts[] = {{0.6595072188743, -2.1371053983433},
                             {0.6591482046471, -2.1373141048153},
                             {0.6592295020837, -2.1365222838402}};
static GeoLoop holeGeoLoop = {.numVerts = 3, .verts = holeVerts};
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

static GeoPolygon nullHoleGeoPolygon;
static GeoPolygon pointHoleGeoPolygon;
static GeoPolygon lineHoleGeoPolygon;

/**
 * Return true if the cell crosses the meridian.
 */
static bool isTransmeridianCell(H3Index h) {
    CellBoundary bndry;
    H3_EXPORT(cellToBoundary)(h, &bndry);

    double minLng = M_PI, maxLng = -M_PI;
    for (int i = 0; i < bndry.numVerts; i++) {
        if (bndry.verts[i].lng < minLng) minLng = bndry.verts[i].lng;
        if (bndry.verts[i].lng > maxLng) maxLng = bndry.verts[i].lng;
    }

    return maxLng - minLng > M_PI - (M_PI / 4);
}

static void fillIndex_assertions(H3Index h) {
    if (isTransmeridianCell(h)) {
        // TODO: these do not work correctly
        return;
    }

    int currentRes = H3_EXPORT(getResolution)(h);
    // TODO: Not testing more than one depth because the assertions fail.
    for (int nextRes = currentRes; nextRes <= currentRes + 1; nextRes++) {
        CellBoundary bndry;
        H3_EXPORT(cellToBoundary)(h, &bndry);
        GeoPolygon polygon = {
            .geoloop = {.numVerts = bndry.numVerts, .verts = bndry.verts},
            .numHoles = 0,
            .holes = 0};

        int64_t polygonToCellsSize;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &polygon, nextRes, 0, &polygonToCellsSize));
        H3Index *polygonToCellsOut =
            calloc(polygonToCellsSize, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &polygon, nextRes, CONTAINMENT_CENTER, polygonToCellsSize,
            polygonToCellsOut));

        int64_t polygonToCellsCount =
            countNonNullIndexes(polygonToCellsOut, polygonToCellsSize);

        int64_t childrenSize;
        t_assertSuccess(
            H3_EXPORT(cellToChildrenSize)(h, nextRes, &childrenSize));
        H3Index *children = calloc(childrenSize, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(cellToChildren)(h, nextRes, children));

        int64_t cellToChildrenCount =
            countNonNullIndexes(children, childrenSize);

        t_assert(polygonToCellsCount == cellToChildrenCount,
                 "PolygonToCells count matches cellToChildren count");

        for (int i = 0; i < childrenSize; i++) {
            bool found = false;
            if (children[i] == H3_NULL) continue;
            for (int j = 0; j < polygonToCellsSize; j++) {
                if (polygonToCellsOut[j] == children[i]) {
                    found = true;
                    break;
                }
            }
            t_assert(
                found,
                "All indexes match between polygonToCells and cellToChildren");
        }

        free(polygonToCellsOut);
        free(children);
    }
}

SUITE(polygonToCells) {
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

    TEST(polygonToCells_ZeroSize) {
        t_assert(H3_EXPORT(polygonToCellsExperimental)(&sfGeoPolygon, 9,
                                                       CONTAINMENT_CENTER, 0,
                                                       NULL) == E_MEMORY_BOUNDS,
                 "Doesn't write to empty array");
    }

    TEST(polygonToCells_CenterContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1253, "got expected polygonToCells size");
        free(hexagons);
    }

    TEST(polygonToCells_FullContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_FULL, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_FULL, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1175,
                 "got expected polygonToCells size (full containment mode)");
        free(hexagons);
    }

    TEST(polygonToCells_Overlapping) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_OVERLAPPING, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_OVERLAPPING, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1334,
                 "got expected polygonToCells size (overlapping mode)");
        free(hexagons);
    }

    TEST(polygonToCells_OverlappingBBox) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1416,
                 "got expected polygonToCells size (overlapping bbox mode)");
        free(hexagons);
    }

    TEST(polygonToCellsHole_CenterContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1214,
                 "got expected polygonToCells size (hole)");
        free(hexagons);
    }

    TEST(polygonToCellsHole_FullContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_FULL, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_FULL, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(
            actualNumIndexes == 1118,
            "got expected polygonToCells size (hole, full containment mode)");
        free(hexagons);
    }

    TEST(polygonToCellsHole_Overlapping) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_OVERLAPPING, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_OVERLAPPING, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1311,
                 "got expected polygonToCells size (hole, overlapping mode)");
        free(hexagons);
    }

    TEST(polygonToCellsHole_OverlappingBBox) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &holeGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(
            actualNumIndexes == 1403,
            "got expected polygonToCells size (hole, overlapping bbox mode)");
        free(hexagons);
    }

    TEST(polygonToCellsHoleParentIssue) {
        // This checks a specific issue where the bounding box of the parent
        // cell fully contains the hole.
        LatLng outer[] = {{0.7774570821346158, 0.19441847890170674},
                          {0.7528853613617879, 0.19441847890170674},
                          {0.7528853613617879, 0.23497118026107888},
                          {0.7774570821346158, 0.23497118026107888}};
        LatLng sanMarino[] = {{0.7662242554877188, 0.21790879024779208},
                              {0.7660964275733029, 0.21688101821117023},
                              {0.7668029019479251, 0.21636628570817204},
                              {0.7676380769015895, 0.21713838446266925},
                              {0.7677659048160054, 0.21823092566783267},
                              {0.7671241996099247, 0.2184218123281233},
                              {0.7662242554877188, 0.21790879024779208}};
        GeoPolygon polygon = {
            .geoloop = {.numVerts = ARRAY_SIZE(outer), .verts = outer},
            .numHoles = 1,
            .holes = (GeoLoop[]){
                {.numVerts = ARRAY_SIZE(sanMarino), .verts = sanMarino}}};

        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &polygon, 6, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &polygon, 6, CONTAINMENT_CENTER, numHexagons, hexagons));

        // This is the cell inside San Marino (i.e. inside the hole)
        H3Index holeCell = 0x861ea3cefffffff;

        int found = 0;
        for (int64_t i = 0; i < numHexagons; i++) {
            if (hexagons[i] == holeCell) found = 1;
        }

        t_assert(!found, "Did not include cell in hole");
        free(hexagons);
    }

    TEST(polygonToCellsEmpty) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &emptyGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &emptyGeoPolygon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 0,
                 "got expected polygonToCells size (empty)");
        free(hexagons);
    }

    TEST(polygonToCellsContainsPolygon) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 0, "got expected polygonToCells size");
        free(hexagons);
    }

    TEST(polygonToCellsContainsPolygon_CenterContainment) {
        // Contains the center point of a res 4 cell
        LatLng centerVerts[] = {{0.6595645, -2.1353315},
                                {0.6595645, -2.1353314},
                                {0.6595644, -2.1353314},
                                {0.6595644, -2.1353314265}};
        GeoLoop centerGeoLoop = {.numVerts = 4, .verts = centerVerts};
        GeoPolygon centerGeoPolygon;
        centerGeoPolygon.geoloop = centerGeoLoop;
        centerGeoPolygon.numHoles = 0;

        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &centerGeoPolygon, 4, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &centerGeoPolygon, 4, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1, "got expected polygonToCells size");
        t_assert(hexagons[0] == 0x8428309ffffffff, "got expected hexagon");

        free(hexagons);
    }

    TEST(polygonToCellsContainsPolygon_FullContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_FULL, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_FULL, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 0,
                 "got expected polygonToCells size (full containment mode)");
        free(hexagons);
    }

    TEST(polygonToCellsContainsPolygon_Overlapping) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_OVERLAPPING, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_OVERLAPPING, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1,
                 "got expected polygonToCells size (overlapping mode)");
        t_assert(hexagons[0] == 0x8428309ffffffff, "got expected hexagon");
        free(hexagons);
    }

    TEST(polygonToCellsContainsPolygon_OverlappingBBox) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_OVERLAPPING_BBOX, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &sfGeoPolygon, 4, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 5,
                 "got expected polygonToCells size (overlapping bbox mode)");
        t_assert(hexagons[0] == 0x8428309ffffffff, "got expected hexagon");
        free(hexagons);
    }

    TEST(polygonToCellsExact) {
        LatLng somewhere = {1, 2};
        H3Index origin;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&somewhere, 9, &origin));
        CellBoundary boundary;
        H3_EXPORT(cellToBoundary)(origin, &boundary);

        LatLng *verts = calloc(boundary.numVerts + 1, sizeof(LatLng));
        for (int i = 0; i < boundary.numVerts; i++) {
            verts[i] = boundary.verts[i];
        }
        verts[boundary.numVerts] = boundary.verts[0];

        GeoLoop someGeoLoop;
        someGeoLoop.numVerts = boundary.numVerts + 1;
        someGeoLoop.verts = verts;
        GeoPolygon someHexagon;
        someHexagon.geoloop = someGeoLoop;
        someHexagon.numHoles = 0;

        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &someHexagon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        int64_t actualNumIndexes;

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &someHexagon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));
        actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);
        t_assert(actualNumIndexes == 1,
                 "got expected polygonToCells size for center containment (1)");

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &someHexagon, 9, CONTAINMENT_FULL, numHexagons, hexagons));
        actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);
        t_assert(actualNumIndexes == 1,
                 "got expected polygonToCells size for full containment (1)");

        // TODO: CONTAINMENT_OVERLAPPING yields 7 cells, presumably due to FPE
        // in the various cell boundaries

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &someHexagon, 9, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);
        // Overlapping BBox is very rough, so we get a couple of overlaps from
        // non-neighboring cells
        t_assert(actualNumIndexes == 9,
                 "got expected polygonToCells size for overlapping bbox "
                 "containment");

        free(hexagons);
        free(verts);
    }

    TEST(polygonToCellsTransmeridian) {
        LatLng primeMeridianVerts[] = {
            {0.01, 0.01}, {0.01, -0.01}, {-0.01, -0.01}, {-0.01, 0.01}};
        GeoLoop primeMeridianGeoLoop = {.numVerts = 4,
                                        .verts = primeMeridianVerts};
        GeoPolygon primeMeridianGeoPolygon = {.geoloop = primeMeridianGeoLoop,
                                              .numHoles = 0};

        LatLng transMeridianVerts[] = {{0.01, -M_PI + 0.01},
                                       {0.01, M_PI - 0.01},
                                       {-0.01, M_PI - 0.01},
                                       {-0.01, -M_PI + 0.01}};
        GeoLoop transMeridianGeoLoop = {.numVerts = 4,
                                        .verts = transMeridianVerts};
        GeoPolygon transMeridianGeoPolygon = {.geoloop = transMeridianGeoLoop,
                                              .numHoles = 0};

        LatLng transMeridianHoleVerts[] = {{0.005, -M_PI + 0.005},
                                           {0.005, M_PI - 0.005},
                                           {-0.005, M_PI - 0.005},
                                           {-0.005, -M_PI + 0.005}};
        GeoLoop transMeridianHoleGeoLoop = {.numVerts = 4,
                                            .verts = transMeridianHoleVerts};
        GeoPolygon transMeridianHoleGeoPolygon = {
            .geoloop = transMeridianGeoLoop,
            .numHoles = 1,
            .holes = &transMeridianHoleGeoLoop};
        GeoPolygon transMeridianFilledHoleGeoPolygon = {
            .geoloop = transMeridianHoleGeoLoop, .numHoles = 0};

        int64_t expectedSize;

        // Prime meridian case
        expectedSize = 4228;
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &primeMeridianGeoPolygon, 7, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &primeMeridianGeoPolygon, 7, CONTAINMENT_CENTER, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == expectedSize,
                 "got expected polygonToCells size (prime meridian)");

        // Transmeridian case
        // This doesn't exactly match the prime meridian count because of slight
        // differences in hex size and grid offset between the two cases
        expectedSize = 4238;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &transMeridianGeoPolygon, 7, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagonsTM = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &transMeridianGeoPolygon, 7, CONTAINMENT_CENTER, numHexagons,
            hexagonsTM));
        actualNumIndexes = countNonNullIndexes(hexagonsTM, numHexagons);

        t_assert(actualNumIndexes == expectedSize,
                 "got expected polygonToCells size (transmeridian)");

        // Transmeridian filled hole case -- only needed for calculating hole
        // size
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &transMeridianFilledHoleGeoPolygon, 7, CONTAINMENT_CENTER,
            &numHexagons));
        H3Index *hexagonsTMFH = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &transMeridianFilledHoleGeoPolygon, 7, CONTAINMENT_CENTER,
            numHexagons, hexagonsTMFH));
        int64_t actualNumHoleIndexes =
            countNonNullIndexes(hexagonsTMFH, numHexagons);

        // Transmeridian hole case
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &transMeridianHoleGeoPolygon, 7, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagonsTMH = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &transMeridianHoleGeoPolygon, 7, CONTAINMENT_CENTER, numHexagons,
            hexagonsTMH));
        actualNumIndexes = countNonNullIndexes(hexagonsTMH, numHexagons);

        t_assert(actualNumIndexes == expectedSize - actualNumHoleIndexes,
                 "got expected polygonToCells size (transmeridian hole)");

        free(hexagons);
        free(hexagonsTM);
        free(hexagonsTMFH);
        free(hexagonsTMH);
    }

    TEST(polygonToCellsTransmeridianComplex) {
        // This polygon is "complex" in that it has > 4 vertices - this
        // tests for a bug that was taking the max and min longitude as
        // the bounds for transmeridian polygons
        LatLng verts[] = {{0.1, -M_PI + 0.00001},  {0.1, M_PI - 0.00001},
                          {0.05, M_PI - 0.2},      {-0.1, M_PI - 0.00001},
                          {-0.1, -M_PI + 0.00001}, {-0.05, -M_PI + 0.2}};
        GeoLoop geoloop = {.numVerts = 6, .verts = verts};
        GeoPolygon polygon = {.geoloop = geoloop, .numHoles = 0};

        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &polygon, 4, CONTAINMENT_CENTER, &numHexagons));

        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &polygon, 4, CONTAINMENT_CENTER, numHexagons, hexagons));

        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 1204,
                 "got expected polygonToCells size (complex transmeridian)");

        free(hexagons);
    }

    TEST(polygonToCellsPentagon) {
        H3Index pentagon;
        setH3Index(&pentagon, 9, 24, 0);
        LatLng coord;
        H3_EXPORT(cellToLatLng)(pentagon, &coord);

        // Length of half an edge of the polygon, in radians
        double edgeLength2 = H3_EXPORT(degsToRads)(0.001);

        LatLng boundingTopRigt = coord;
        boundingTopRigt.lat += edgeLength2;
        boundingTopRigt.lng += edgeLength2;

        LatLng boundingTopLeft = coord;
        boundingTopLeft.lat += edgeLength2;
        boundingTopLeft.lng -= edgeLength2;

        LatLng boundingBottomRight = coord;
        boundingBottomRight.lat -= edgeLength2;
        boundingBottomRight.lng += edgeLength2;

        LatLng boundingBottomLeft = coord;
        boundingBottomLeft.lat -= edgeLength2;
        boundingBottomLeft.lng -= edgeLength2;

        LatLng verts[] = {boundingBottomLeft, boundingTopLeft, boundingTopRigt,
                          boundingBottomRight};

        GeoLoop geoloop;
        geoloop.verts = verts;
        geoloop.numVerts = 4;

        GeoPolygon polygon;
        polygon.geoloop = geoloop;
        polygon.numHoles = 0;

        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &polygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &polygon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));

        int found = 0;
        int numPentagons = 0;
        for (int i = 0; i < numHexagons; i++) {
            if (hexagons[i] != 0) {
                found++;
            }
            if (H3_EXPORT(isPentagon)(hexagons[i])) {
                numPentagons++;
            }
        }
        t_assert(found == 1, "one index found");
        t_assert(numPentagons == 1, "one pentagon found");
        free(hexagons);
    }

    TEST(polygonToCellsNullPolygon) {
        for (int res = 0; res <= MAX_H3_RES; res++) {
            for (uint32_t flags = 0; flags < CONTAINMENT_INVALID; flags++) {
                int64_t numHexagons;
                t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                    &nullGeoPolygon, res, flags, &numHexagons));
                t_assert(numHexagons == 0, "got expected estimated size");
                H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

                t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
                    &nullGeoPolygon, res, flags, numHexagons, hexagons));
                int64_t actualNumIndexes =
                    countNonNullIndexes(hexagons, numHexagons);

                t_assert(actualNumIndexes == 0,
                         "got expected polygonToCells size");
                free(hexagons);
            }
        }
    }

    TEST(polygonToCellsPointPolygon_CenterContainment) {
        for (int res = 0; res <= MAX_H3_RES; res++) {
            int64_t numHexagons;
            t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_CENTER, &numHexagons));
            t_assert(numHexagons >= 1 && numHexagons <= 5,
                     "got expected estimated size");
            H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

            t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_CENTER, numHexagons,
                hexagons));
            int64_t actualNumIndexes =
                countNonNullIndexes(hexagons, numHexagons);

            t_assert(actualNumIndexes == 0, "got expected polygonToCells size");
            free(hexagons);
        }
    }

    TEST(polygonToCellsPointPolygon_FullContainment) {
        for (int res = 0; res <= MAX_H3_RES; res++) {
            int64_t numHexagons;
            t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_FULL, &numHexagons));
            t_assert(numHexagons >= 1 && numHexagons <= 5,
                     "got expected estimated size");
            H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

            t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_FULL, numHexagons,
                hexagons));
            int64_t actualNumIndexes =
                countNonNullIndexes(hexagons, numHexagons);

            t_assert(actualNumIndexes == 0, "got expected polygonToCells size");
            free(hexagons);
        }
    }

    TEST(polygonToCellsPointPolygon_Overlapping) {
        for (int res = 0; res <= MAX_H3_RES; res++) {
            int64_t numHexagons;
            t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_OVERLAPPING, &numHexagons));
            t_assert(numHexagons >= 1 && numHexagons <= 5,
                     "got expected estimated size");
            H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

            t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_OVERLAPPING, numHexagons,
                hexagons));
            int64_t actualNumIndexes =
                countNonNullIndexes(hexagons, numHexagons);

            t_assert(actualNumIndexes == 1, "got expected polygonToCells size");
            free(hexagons);
        }
    }

    TEST(polygonToCellsPointPolygon_OverlappingBBox) {
        for (int res = 0; res <= MAX_H3_RES; res++) {
            int64_t numHexagons;
            t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_OVERLAPPING_BBOX,
                &numHexagons));
            t_assert(numHexagons >= 1 && numHexagons <= 5,
                     "got expected estimated size");
            H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

            t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
                &pointGeoPolygon, res, CONTAINMENT_OVERLAPPING_BBOX,
                numHexagons, hexagons));
            int64_t actualNumIndexes =
                countNonNullIndexes(hexagons, numHexagons);

            t_assert(actualNumIndexes >= 1 && actualNumIndexes <= 5,
                     "got expected polygonToCells size");
            free(hexagons);
        }
    }

    TEST(polygonToCellsOutOfBoundsPolygon) {
        for (int res = 0; res <= MAX_H3_RES; res++) {
            for (uint32_t flags = 0; flags < CONTAINMENT_INVALID; flags++) {
                int64_t numHexagons;
                t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                    &outOfBoundsVertGeoPolygon, res, flags, &numHexagons));
                t_assert(numHexagons == 0, "got expected estimated size");
                // Note: We're allocating more memory than the estimate to test
                // for out-of-bounds writes here
                H3Index *hexagons = calloc(10, sizeof(H3Index));

                t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
                    &outOfBoundsVertGeoPolygon, res, flags, numHexagons,
                    hexagons));
                int64_t actualNumIndexes =
                    countNonNullIndexes(hexagons, numHexagons);

                t_assert(actualNumIndexes == 0,
                         "got expected polygonToCells size");
                free(hexagons);
            }
        }
    }

    TEST(polygonToCellsLinePolygon_CenterContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 0, "got expected polygonToCells size");
        free(hexagons);
    }

    TEST(polygonToCellsLinePolygon_FullContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_FULL, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_FULL, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 0, "got expected polygonToCells size");
        free(hexagons);
    }

    TEST(polygonToCellsLinePolygon_Overlapping) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_OVERLAPPING, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_OVERLAPPING, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 9, "got expected polygonToCells size");
        free(hexagons);
    }

    TEST(polygonToCellsLinePolygon_OverlappingBBox) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        t_assert(actualNumIndexes == 21, "got expected polygonToCells size");
        free(hexagons);
    }

    TEST(polygonToCellsNullHole_CenterContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1253,
                 "got expected polygonToCells size (null hole)");
        free(hexagons);
    }

    TEST(polygonToCellsNullHole_FullContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_FULL, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_FULL, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1175,
                 "got expected polygonToCells size (null hole)");
        free(hexagons);
    }

    TEST(polygonToCellsNullHole_Overlapping) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1334,
                 "got expected polygonToCells size (null hole)");
        free(hexagons);
    }

    TEST(polygonToCellsNullHole_OverlappingBBox) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX,
            &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &nullHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1416,
                 "got expected polygonToCells size (null hole)");
        free(hexagons);
    }

    TEST(polygonToCellsPointHole_CenterContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_CENTER, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1253,
                 "got expected polygonToCells size (point hole)");
        free(hexagons);
    }

    TEST(polygonToCellsPointHole_FullContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_FULL, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_FULL, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // We expect that the cell containing the hole is not included
        t_assert(actualNumIndexes == 1175 - 1,
                 "got expected polygonToCells size (point hole)");
        free(hexagons);
    }

    TEST(polygonToCellsPointHole_Overlapping) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1334,
                 "got expected polygonToCells size (point hole)");
        free(hexagons);
    }

    TEST(polygonToCellsPointHole_OverlappingBBox) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX,
            &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &pointHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1416,
                 "got expected polygonToCells size (point hole)");
        free(hexagons);
    }

    TEST(polygonToCellsLineHole_CenterContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_CENTER, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1253,
                 "got expected polygonToCells size (line hole)");
        free(hexagons);
    }

    TEST(polygonToCellsLineHole_FullContainment) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_FULL, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_FULL, numHexagons, hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // We expect that the cells intersecting the line are not included
        t_assert(actualNumIndexes == 1175 - 9,
                 "got expected polygonToCells size (line hole)");
        free(hexagons);
    }

    TEST(polygonToCellsLineHole_Overlapping) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING, &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1334,
                 "got expected polygonToCells size (line hole)");
        free(hexagons);
    }

    TEST(polygonToCellsLineHole_OverlappingBBox) {
        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX,
            &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(polygonToCellsExperimental)(
            &lineHoleGeoPolygon, 9, CONTAINMENT_OVERLAPPING_BBOX, numHexagons,
            hexagons));
        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);

        // Same as without the hole
        t_assert(actualNumIndexes == 1416,
                 "got expected polygonToCells size (line hole)");
        free(hexagons);
    }

    TEST(invalidFlags) {
        int64_t numHexagons;

        // Test invalid containment modes (>= CONTAINMENT_INVALID) for
        // non-geodesic
        for (uint32_t mode = CONTAINMENT_INVALID;
             mode <= CONTAINMENT_INVALID + 5; mode++) {
            t_assert(
                H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                    &sfGeoPolygon, 9, mode, &numHexagons) == E_OPTION_INVALID,
                "Invalid containment modes should be rejected");
        }

        // Test invalid containment modes for geodesic (only FULL and
        // OVERLAPPING are valid)
        uint32_t invalidGeodesicFlags[] = {
            FLAG_GEODESIC_MASK | CONTAINMENT_CENTER,
            FLAG_GEODESIC_MASK | CONTAINMENT_OVERLAPPING_BBOX,
            FLAG_GEODESIC_MASK | CONTAINMENT_INVALID};
        for (int i = 0; i < 3; i++) {
            t_assert(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                         &sfGeoPolygon, 9, invalidGeodesicFlags[i],
                         &numHexagons) == E_OPTION_INVALID,
                     "Invalid geodesic containment modes should be rejected");
        }

        // Test invalid flag bits (bits outside valid masks)
        t_assert(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                     &sfGeoPolygon, 9, 0x100, &numHexagons) == E_OPTION_INVALID,
                 "Invalid flag bits should be rejected");

        // Test valid flags work
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 9, CONTAINMENT_CENTER, &numHexagons));
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSizeExperimental)(
            &sfGeoPolygon, 9, FLAG_GEODESIC_MASK | CONTAINMENT_FULL,
            &numHexagons));

        // Test polygonToCellsExperimental with same invalid flags
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));
        t_assert(H3_EXPORT(polygonToCellsExperimental)(
                     &sfGeoPolygon, 9, CONTAINMENT_INVALID, numHexagons,
                     hexagons) == E_OPTION_INVALID,
                 "Invalid flags should be rejected");
        t_assert(H3_EXPORT(polygonToCellsExperimental)(&sfGeoPolygon, 9, 0x100,
                                                       numHexagons, hexagons) ==
                     E_OPTION_INVALID,
                 "Invalid flag bits should be rejected");
        free(hexagons);
    }

    TEST(fillIndex) {
        iterateAllIndexesAtRes(0, fillIndex_assertions);
        iterateAllIndexesAtRes(1, fillIndex_assertions);
        iterateAllIndexesAtRes(2, fillIndex_assertions);
    }
}
