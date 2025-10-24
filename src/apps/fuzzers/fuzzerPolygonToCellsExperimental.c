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
/** @file fuzzerPolygonToCellsExperimental.c
 * @brief Fuzzes the experimental polygon-to-cells implementation.
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

const int MAX_RES = 15;
const int MAX_SZ = 4000000;
const int MAX_HOLES = 100;

int populateGeoLoop(GeoLoop *g, const uint8_t *data, size_t *offset,
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

void run(GeoPolygon *geoPolygon, uint32_t flags, int res) {
    int64_t sz;
    H3Error err = H3_EXPORT(maxPolygonToCellsSizeExperimental)(geoPolygon, res,
                                                               flags, &sz);
    if (!err && sz < MAX_SZ) {
        H3Index *out = calloc(sz, sizeof(H3Index));
        H3_EXPORT(polygonToCellsExperimental)(geoPolygon, res, flags, sz, out);
        free(out);
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // TODO: It is difficult for the fuzzer to generate inputs that are
    // considered valid by this fuzzer. fuzzerPolygonToCellsNoHoles.c
    // is a workaround for that.
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;
    int res = args->res % (MAX_RES + 1);

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
        geoPolygon.numHoles = originalNumHoles;
        run(&geoPolygon, flags, res);
        if (flags == CONTAINMENT_FULL || flags == CONTAINMENT_OVERLAPPING) {
            uint32_t geodesicFlags = flags;
            FLAG_SET_GEODESIC(geodesicFlags);
            run(&geoPolygon, geodesicFlags, res);
        }

        geoPolygon.numHoles = 0;
        run(&geoPolygon, flags, res);
        if (flags == CONTAINMENT_FULL || flags == CONTAINMENT_OVERLAPPING) {
            uint32_t geodesicFlags = flags;
            FLAG_SET_GEODESIC(geodesicFlags);
            run(&geoPolygon, geodesicFlags, res);
        }
    }
    free(geoPolygon.holes);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
