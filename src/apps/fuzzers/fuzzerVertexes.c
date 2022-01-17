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
 * @brief Fuzzer program for cellToVertex and related functions
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    H3Index index;
    int vertexNum;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    H3Index out;
    H3_EXPORT(cellToVertex)(args->index, args->vertexNum, &out);
    H3Index outArr[6];
    H3_EXPORT(cellToVertexes)(args->index, outArr);
    LatLng geo;
    H3_EXPORT(vertexToLatLng)(args->index, &geo);
    H3_EXPORT(isValidVertex)(args->index);
    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
