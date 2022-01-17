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
 * @brief Fuzzer program for resolution specific functions
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    int res;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    double out;
    H3_EXPORT(getHexagonAreaAvgKm2)(args->res, &out);
    H3_EXPORT(getHexagonAreaAvgM2)(args->res, &out);
    H3_EXPORT(getHexagonEdgeLengthAvgKm)(args->res, &out);
    H3_EXPORT(getHexagonEdgeLengthAvgM)(args->res, &out);

    int64_t outInt;
    H3_EXPORT(getNumCells)(args->res, &outInt);

    H3Index pentagons[12];
    H3_EXPORT(getPentagons)(args->res, pentagons);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
