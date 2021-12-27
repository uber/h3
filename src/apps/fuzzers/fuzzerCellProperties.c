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
 * @brief Fuzzer program for cell property functions
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    H3Index index;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    H3_EXPORT(getResolution)(args->index);
    H3_EXPORT(getBaseCellNumber)(args->index);
    H3_EXPORT(isValidCell)(args->index);
    H3_EXPORT(isPentagon)(args->index);
    H3_EXPORT(isResClassIII)(args->index);

    int faceCount;
    H3Error err = H3_EXPORT(maxFaceCount)(args->index, &faceCount);
    if (!err && faceCount > 0) {
        int *out = calloc(faceCount, sizeof(int));
        H3_EXPORT(getIcosahedronFaces)(args->index, out);
        free(out);
    }

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
