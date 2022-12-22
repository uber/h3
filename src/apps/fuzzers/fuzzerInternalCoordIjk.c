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
 * @brief Fuzzer program for internal functions in coordijk.c
 */

#include "aflHarness.h"
#include "algos.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    CoordIJK ijk;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    // These functions require that input be non-negative
    if (args->ijk.i >= 0 && args->ijk.j >= 0 && args->ijk.k >= 0) {
        CoordIJK ijkCopy1 = args->ijk;
        _upAp7Checked(&ijkCopy1);
        CoordIJK ijkCopy2 = args->ijk;
        _upAp7rChecked(&ijkCopy2);
    }
    // This function needs a guard check to be safe and that guard
    // check assumes k = 0.
    CoordIJK ijkCopy3 = args->ijk;
    ijkCopy3.k = 0;
    if (!_ijkNormalizeCouldOverflow(&ijkCopy3)) {
        _ijkNormalize(&ijkCopy3);
    }

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
