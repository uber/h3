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
 * @brief Fuzzer program for distanceRads
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    LatLng a;
    LatLng b;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    H3_EXPORT(greatCircleDistanceRads)(&args->a, &args->b);
    H3_EXPORT(greatCircleDistanceKm)(&args->a, &args->b);
    H3_EXPORT(greatCircleDistanceM)(&args->a, &args->b);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
