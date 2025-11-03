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
 * @brief Fuzzer program for constructCell
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

#define STRING_LENGTH 32

typedef struct {
    int res;
    int baseCellNumber;
    int digits[MAX_H3_RES];
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    int resForAlloc = args->res;
    if (resForAlloc < 0) {
        resForAlloc = 0;
    } else if (resForAlloc > MAX_H3_RES) {
        resForAlloc = MAX_H3_RES;
    }
    int *digits = calloc(resForAlloc, sizeof(int));
    memcpy(digits, args->digits, sizeof(int) * resForAlloc);

    H3Index out;
    H3_EXPORT(constructCell)
    (args->res, args->baseCellNumber, args->digits, &out);
    // Must be OK with the digits array being shorter than the full 15 digits,
    // if res < 15.
    H3_EXPORT(constructCell)
    (args->res, args->baseCellNumber, digits, &out);
    if (args->res == 0) {
        // If res == 0, must also be OK with NULL.
        H3_EXPORT(constructCell)
        (args->res, args->baseCellNumber, NULL, &out);
    }

    free(digits);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
