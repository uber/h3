/*
 * Copyright 2016-2021 Uber Technologies, Inc.
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
/** @file bbox.c
 * @brief   Geographic bounding box functions
 */

#include "polyfill.h"

#include <string.h>

#include "h3Assert.h"
#include "h3Index.h"
#include "polygon.h"

// Factor by which to scale the cell bounding box to include all children.
// This was determined empirically by finding the smallest factor that
// passed exhaustive tests.
#define CHILD_SCALE_FACTOR 1.4

static const LatLng NORTH_POLE = {M_PI_2, 0};
static const LatLng SOUTH_POLE = {-M_PI_2, 0};

/**
 * For a given cell, return a bounding box guaranteed to contain its
 * children at any finer resolution. Note that no guarantee is provided
 * as to the level of accuracy, and the bounding box may have a significant
 * margin of error.
 */
H3Error cellToChildBBox(H3Index cell, BBox *out) {
    H3Error err;

    CellBoundary boundary;
    err = H3_EXPORT(cellToBoundary)(cell, &boundary);
    if (err) {
        return err;
    }
    // Convert to GeoLoop
    GeoLoop loop;
    loop.numVerts = boundary.numVerts;
    LatLng *verts = malloc(boundary.numVerts * sizeof(LatLng));
    if (!verts) {
        return E_MEMORY_ALLOC;
    }
    memcpy(verts, &boundary.verts, boundary.numVerts * sizeof(LatLng));
    loop.verts = verts;
    // Calculate bbox
    bboxFromGeoLoop(&loop, out);

    // Buffer the bounding box to cover children
    scaleBBox(out, CHILD_SCALE_FACTOR);

    // Adjust the BBox to handle poles
    H3Index poleTest;
    // North pole
    err = H3_EXPORT(latLngToCell)(&NORTH_POLE, H3_GET_RESOLUTION(cell),
                                  &poleTest);
    if (NEVER(err != E_SUCCESS)) {
        return err;
    }
    if (cell == poleTest) {
        out->north = M_PI_2;
    }
    // South pole
    err = H3_EXPORT(latLngToCell)(&SOUTH_POLE, H3_GET_RESOLUTION(cell),
                                  &poleTest);
    if (NEVER(err != E_SUCCESS)) {
        return err;
    }
    if (cell == poleTest) {
        out->south = -M_PI_2;
    }
    // If we contain a pole, expand the longitude to include the full domain,
    // effectively making the bbox a circle around the pole.
    if (out->north == M_PI_2 || out->south == -M_PI_2) {
        out->east = M_PI;
        out->west = -M_PI;
    }

    return E_SUCCESS;
}
