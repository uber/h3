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
/** @file
 * @brief Fuzzer program for geoLoopAreaRads2
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Interpret the input buffer as an array of LatLng vertices.
    GeoLoop loop = {
        .numVerts = (int)(size / sizeof(LatLng)),
        .verts = (LatLng *)data,
    };
    double area;
    H3_EXPORT(geoLoopAreaRads2)(loop, &area);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(LatLng) * 1024);
