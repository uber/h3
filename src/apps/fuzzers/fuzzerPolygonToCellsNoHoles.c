/*
 * Copyright 2022 Uber Technologies, Inc.
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
 * @brief Fuzzer program for polygonToCells and related functions, without holes
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

const int MAX_RES = 15;
const int MAX_SZ = 4000000;

void run(GeoPolygon *geoPolygon, uint32_t flags, int res) {
    int64_t sz;
    H3Error err = H3_EXPORT(maxPolygonToCellsSize)(geoPolygon, res, flags, &sz);
    if (!err && sz < MAX_SZ) {
        H3Index *out = calloc(sz, sizeof(H3Index));
        H3_EXPORT(polygonToCells)(geoPolygon, res, flags, out);
        free(out);
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(int)) {
        return 0;
    }

    uint8_t res = *data;
    size_t vertsSize = size - 1;
    int numVerts = vertsSize / sizeof(LatLng);

    GeoPolygon geoPolygon;
    geoPolygon.numHoles = 0;
    geoPolygon.holes = NULL;
    geoPolygon.geoloop.numVerts = numVerts;
    // Offset by 1 since *data was used for `res`, above.
    geoPolygon.geoloop.verts = (LatLng *)(data + 1);

    // TODO: Fuzz the `flags` input as well when it has meaningful input
    run(&geoPolygon, 0, res);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(H3Index) * 1024);
