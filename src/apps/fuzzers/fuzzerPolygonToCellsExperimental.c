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
/** @file
 * @brief Fuzzer program for polygonToCells2 and related functions
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

static GeoPolygon fuzzerFailure = {
    .geoloop = {.numVerts = 1, .verts = (LatLng[]){{0.000000, 0.000000}}},
    .numHoles = 5,
    .holes = (GeoLoop[]){
        {.numVerts = 9,
         .verts =
             (LatLng[]){
                 {0.000000, -0.000000},
                 {-0.000000,
                  2748442886721409313719038754629364176612174103396910975667688045384466158239047111515156119908354033028732640897702389786583276177564900289955662483960102912.000000},
                 {-0.000000, -0.000000},
                 {-0.000000, -0.000000},
                 {-0.000000, -0.000000},
                 {-0.000000, -0.000000},
                 {-0.000000, -0.000000},
                 {-0.000000, 0.000000},
                 {0.000000, 0.000000}}},
        {.numVerts = 0},
        {.numVerts = 0},
        {.numVerts = 0},
        {.numVerts = 0}}};

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

static H3Error countPolygonToCellsExperimental(const GeoPolygon *polygon,
                                               int res, uint32_t flags,
                                               int64_t *out) {
    IterCellsPolygon iter = iterInitPolygon(polygon, res, flags);
    *out = 0;
    for (; iter.cell; iterStepPolygon(&iter)) *out += 1;
    return iter.error;
}

void run(GeoPolygon *geoPolygon, uint32_t flags, int res) {
    int64_t sz;
    H3Error err = H3_EXPORT(maxPolygonToCellsSizeExperimental)(geoPolygon, res,
                                                               flags, &sz);

    geoPolygonPrintln(geoPolygon);
    printf("estimate: %lld\n", sz);
    int64_t exact;
    H3_EXPORT(countPolygonToCellsExperimental)(geoPolygon, res, flags, &exact);
    printf("exact: %lld\n", exact);

    if (!err && sz < MAX_SZ) {
        H3Index *out = calloc(sz, sizeof(H3Index));
        H3_EXPORT(polygonToCellsExperimental)(geoPolygon, res, flags, out);
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
        run(&fuzzerFailure, flags, res);
        geoPolygon.numHoles = originalNumHoles;
        run(&geoPolygon, flags, res);
        geoPolygon.numHoles = 0;
        run(&geoPolygon, flags, res);
    }
    free(geoPolygon.holes);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
