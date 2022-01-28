/*
 * Copyright 2021 Uber Technologies, Inc.
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
 * @brief Fuzzer program for cellsToLinkedMultiPolygon
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    const H3Index *h3Set = (const H3Index *)data;
    int sz = size / sizeof(H3Index);

    LinkedGeoPolygon polygon;
    H3Error err = H3_EXPORT(cellsToLinkedMultiPolygon)(h3Set, sz, &polygon);

    if (!err) {
        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }
    return 0;
}

AFL_HARNESS_MAIN(sizeof(H3Index) * 1024);
