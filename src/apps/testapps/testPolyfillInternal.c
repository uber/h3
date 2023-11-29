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

#include <math.h>

#include "bbox.h"
#include "h3Index.h"
#include "h3api.h"
#include "latLng.h"
#include "polyfill.h"
#include "polygon.h"
#include "test.h"
#include "utility.h"

// Fixtures
static GeoPolygon sfGeoPolygon = {
    .geoloop = {.numVerts = 6,
                .verts = (LatLng[]){{0.659966917655, -2.1364398519396},
                                    {0.6595011102219, -2.1359434279405},
                                    {0.6583348114025, -2.1354884206045},
                                    {0.6581220034068, -2.1382437718946},
                                    {0.6594479998527, -2.1384597563896},
                                    {0.6599990002976, -2.1376771158464}}},
    .numHoles = 0};

static GeoPolygon invalidGeoPolygon = {
    .geoloop = {.numVerts = 4,
                .verts = (LatLng[]){{NAN, -2.1364398519396},
                                    {0.6595011102219, NAN},
                                    {NAN, -2.1354884206045},
                                    {0.6581220034068, NAN}}},
    .numHoles = 0};

SUITE(polyfillInternal) {
    TEST(iterInitPolygonCompact_errors) {
        IterCellsPolygonCompact iter;

        iter = iterInitPolygonCompact(&sfGeoPolygon, -1, CONTAINMENT_CENTER);
        t_assert(iter.error == E_RES_DOMAIN,
                 "Got expected error for invalid res");
        t_assert(iter.cell == H3_NULL, "Got null output for invalid res");

        iter = iterInitPolygonCompact(&sfGeoPolygon, 16, CONTAINMENT_CENTER);
        t_assert(iter.error == E_RES_DOMAIN,
                 "Got expected error for invalid res");
        t_assert(iter.cell == H3_NULL, "Got null output for invalid res");

        iter = iterInitPolygonCompact(&sfGeoPolygon, 9, 42);
        t_assert(iter.error == E_OPTION_INVALID,
                 "Got expected error for invalid flags");
        t_assert(iter.cell == H3_NULL, "Got null output for invalid flags");
    }

    TEST(iterStepPolygonCompact_invalidCellErrors) {
        IterCellsPolygonCompact iter;
        H3Index cell;

        // Give the iterator a cell with a bad base cell
        iter = iterInitPolygonCompact(&sfGeoPolygon, 9, CONTAINMENT_CENTER);
        t_assertSuccess(iter.error);

        cell = 0x85283473fffffff;
        H3_SET_BASE_CELL(cell, 123);
        iter.cell = cell;

        iterStepPolygonCompact(&iter);
        t_assert(iter.error == E_CELL_INVALID,
                 "Got expected error for invalid cell");
        t_assert(iter.cell == H3_NULL, "Got null output for invalid cell");

        // Give the iterator a cell with a bad base cell, at the target res
        iter = iterInitPolygonCompact(&sfGeoPolygon, 9, CONTAINMENT_CENTER);
        t_assertSuccess(iter.error);

        cell = 0x89283470003ffff;
        H3_SET_BASE_CELL(cell, 123);
        iter.cell = cell;

        iterStepPolygonCompact(&iter);
        t_assert(iter.error == E_CELL_INVALID,
                 "Got expected error for invalid cell");
        t_assert(iter.cell == H3_NULL,
                 "Got null output for invalid cell at res");

        // Give the iterator a cell with a bad base cell, at the target res
        // (full containment)
        iter = iterInitPolygonCompact(&sfGeoPolygon, 9, CONTAINMENT_FULL);
        t_assertSuccess(iter.error);

        cell = 0x89283470003ffff;
        H3_SET_BASE_CELL(cell, 123);
        iter.cell = cell;

        iterStepPolygonCompact(&iter);
        t_assert(iter.error == E_CELL_INVALID,
                 "Got expected error for invalid cell");
        t_assert(iter.cell == H3_NULL,
                 "Got null output for invalid cell at res");

        // Give the iterator a cell with a bad base cell, at the target res
        // (overlapping bounding box)
        iter = iterInitPolygonCompact(&sfGeoPolygon, 9,
                                      CONTAINMENT_OVERLAPPING_BBOX);
        t_assertSuccess(iter.error);

        cell = 0x89283470003ffff;
        H3_SET_BASE_CELL(cell, 123);
        iter.cell = cell;

        iterStepPolygonCompact(&iter);
        t_assert(iter.error == E_CELL_INVALID,
                 "Got expected error for invalid cell");
        t_assert(iter.cell == H3_NULL,
                 "Got null output for invalid cell at res");

        // Give the iterator a cell that's too fine for a child check,
        // and a target resolution that allows this to run. This cell has
        // to be inside the polygon to reach the error.
        iter = iterInitPolygonCompact(&sfGeoPolygon, 9, CONTAINMENT_CENTER);
        t_assertSuccess(iter.error);

        cell = 0x8f283080dcb019a;
        iter.cell = cell;
        iter._res = 42;

        iterStepPolygonCompact(&iter);
        t_assert(iter.error == E_RES_DOMAIN,
                 "Got expected error for too-fine cell");
        t_assert(iter.cell == H3_NULL, "Got null output for invalid cell");
    }

    TEST(iterStepPolygonCompact_invalidPolygonErrors) {
        IterCellsPolygonCompact iter;

        // Start with a good polygon, otherwise we error out early
        iter =
            iterInitPolygonCompact(&sfGeoPolygon, 5, CONTAINMENT_OVERLAPPING);
        t_assertSuccess(iter.error);

        // Give the iterator a bad polygon and a cell at target res
        iter._polygon = &invalidGeoPolygon;
        iter.cell = 0x85283473fffffff;

        iterStepPolygonCompact(&iter);
        t_assert(iter.error == E_LATLNG_DOMAIN,
                 "Got expected error for invalid polygon");
        t_assert(iter.cell == H3_NULL, "Got null output for invalid cell");
    }

    TEST(iterDestroyPolygonCompact) {
        IterCellsPolygonCompact iter =
            iterInitPolygonCompact(&sfGeoPolygon, 9, CONTAINMENT_CENTER);
        t_assertSuccess(iter.error);

        iterDestroyPolygonCompact(&iter);
        t_assert(iter.error == E_SUCCESS, "Got success for destroyed iterator");
        t_assert(iter.cell == H3_NULL,
                 "Got null output for destroyed iterator");

        for (int i = 0; i < 3; i++) {
            iterStepPolygonCompact(&iter);
            t_assert(iter.cell == H3_NULL,
                     "Got null output for destroyed iterator");
        }
    }

    TEST(iterDestroyPolygon) {
        IterCellsPolygon iter =
            iterInitPolygon(&sfGeoPolygon, 9, CONTAINMENT_CENTER);
        t_assertSuccess(iter.error);

        iterDestroyPolygon(&iter);
        t_assert(iter.error == E_SUCCESS, "Got success for destroyed iterator");
        t_assert(iter.cell == H3_NULL,
                 "Got null output for destroyed iterator");

        for (int i = 0; i < 3; i++) {
            iterStepPolygon(&iter);
            t_assert(iter.cell == H3_NULL,
                     "Got null output for destroyed iterator");
        }
    }

    TEST(cellToBBox_noScale) {
        // arbitrary cell
        H3Index cell = 0x85283473fffffff;
        BBox bbox;
        t_assertSuccess(cellToBBox(cell, &bbox, false));

        double cellArea;
        t_assertSuccess(H3_EXPORT(cellAreaRads2)(cell, &cellArea));
        double bboxArea = bboxWidthRads(&bbox) * bboxHeightRads(&bbox);
        double ratio = bboxArea / cellArea;

        CellBoundary verts;
        t_assertSuccess(H3_EXPORT(cellToBoundary)(cell, &verts));

        t_assert(ratio < 3 && ratio > 1,
                 "Got reasonable area ratio between cell and bbox");
    }

    TEST(cellToBBox_boundaryError) {
        // arbitrary cell
        H3Index cell = 0x85283473fffffff;
        H3_SET_BASE_CELL(cell, 123);

        BBox bbox;
        t_assert(cellToBBox(cell, &bbox, false) == E_CELL_INVALID,
                 "Got expected error for cell with invalid base cell");
    }

    TEST(cellToBBox_res0boundaryError) {
        // arbitrary res 0 cell
        H3Index cell = 0x8001fffffffffff;
        H3_SET_BASE_CELL(cell, 123);

        BBox bbox;
        t_assert(cellToBBox(cell, &bbox, false) == E_CELL_INVALID,
                 "Got expected error for res 0 cell with invalid base cell");
    }

    TEST(baseCellNumToCell) {
        for (int i = 0; i < NUM_BASE_CELLS; i++) {
            H3Index cell = baseCellNumToCell(i);
            t_assert(H3_EXPORT(isValidCell)(cell), "Cell is valid");
            t_assert(
                H3_GET_BASE_CELL(cell) == i && H3_GET_RESOLUTION(cell) == 0,
                "Cell has correct base number and res");
        }
    }

    TEST(baseCellNumToCell_boundaryErrors) {
        t_assert(baseCellNumToCell(-1) == H3_NULL,
                 "Expected null for less than 0");
        t_assert(baseCellNumToCell(NUM_BASE_CELLS) == H3_NULL,
                 "Expected null for positive out of bounds");
    }
}
