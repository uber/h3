/*
 * Copyright 2023-2024 Uber Technologies, Inc.
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
/** @file fuzzerPolygonToCellsExperimentalGeodesic.c
 * @brief Fuzzes the experimental polygon-to-cells implementation in geodesic
 *        mode.
 */

#include "aflHarness.h"
#include "h3api.h"
#include "polyfill.h"
#include "polygon.h"
#include "utility.h"

typedef struct {
    int res;
    int numHoles;
    // repeating: num verts, verts
    // We add a large fixed buffer so our test case generator for AFL
    // knows how large to make the file.
    uint8_t buffer[1024];
} inputArgs;

const int MAX_GEODESIC_RES = 4;
const int MAX_SZ = 100000;
const int MAX_HOLES = 100;

static int populateGeoLoop(GeoLoop *g, const uint8_t *data, size_t *offset,
                           size_t size) {
    if (size < *offset + sizeof(int)) {
        return 1;
    }
    int numVerts = *(const int *)(data + *offset);
    *offset = *offset + sizeof(int);
    g->numVerts = numVerts;
    if (size < *offset + sizeof(LatLng) * numVerts) {
        return 1;
    }
    g->verts = (LatLng *)(data + *offset);
    *offset = *offset + sizeof(LatLng) * numVerts;
    return 0;
}

static void runGeodesic(GeoPolygon *geoPolygon, uint32_t flags, int res) {
    uint32_t geodesicFlags = flags;
    FLAG_SET_GEODESIC(geodesicFlags);

    int64_t sz;
    H3Error err = H3_EXPORT(maxPolygonToCellsSizeExperimental)(
        geoPolygon, res, geodesicFlags, &sz);
    if (!err && sz < MAX_SZ) {
        H3Index *out = calloc(sz, sizeof(H3Index));
        H3_EXPORT(polygonToCellsExperimental)
        (geoPolygon, res, geodesicFlags, sz, out);
        free(out);
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;
    int res = args->res % (MAX_GEODESIC_RES + 1);
    if (res == 0) {
        res = 1;  // resolution 1 tests more code paths compared to 0
    }

    GeoPolygon geoPolygon;
    int originalNumHoles = args->numHoles % MAX_HOLES;
    geoPolygon.numHoles = originalNumHoles;
    if (geoPolygon.numHoles < 0) {
        return 0;
    }
    geoPolygon.holes = calloc(geoPolygon.numHoles, sizeof(GeoLoop));
    size_t offset = sizeof(inputArgs) - sizeof(args->buffer);
    if (populateGeoLoop(&geoPolygon.geoloop, data, &offset, size)) {
        free(geoPolygon.holes);
        return 0;
    }
    for (int i = 0; i < geoPolygon.numHoles; i++) {
        if (populateGeoLoop(&geoPolygon.holes[i], data, &offset, size)) {
            free(geoPolygon.holes);
            return 0;
        }
    }

    for (uint32_t flags = 0; flags < CONTAINMENT_INVALID; flags++) {
        bool canRunGeodesic =
            flags == CONTAINMENT_OVERLAPPING || flags == CONTAINMENT_FULL;
        canRunGeodesic &= geoPolygon.geoloop.numVerts <= 256;

        if (!canRunGeodesic) {
            continue;
        }

        geoPolygon.numHoles = originalNumHoles;
        runGeodesic(&geoPolygon, flags, res);

        if (originalNumHoles == 0) {
            continue;
        }

        geoPolygon.numHoles = 0;
        runGeodesic(&geoPolygon, flags, res);
    }
    free(geoPolygon.holes);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
