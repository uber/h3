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
 * @brief Fuzzer program for compactCells and uncompactCells
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

const int MAX_UNCOMPACT_RES = 9;
const int64_t MAX_UNCOMPACT_SIZE = 4000000;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(H3Index) * 2) {
        return 0;
    }

    uint8_t res = *data;
    H3Index *input = (H3Index *)(data + sizeof(H3Index));
    size_t inputSize = (size - sizeof(H3Index)) / sizeof(H3Index);

    // fuzz compactCells
    H3Index *compacted = calloc(inputSize, sizeof(H3Index));
    H3_EXPORT(compactCells)(input, compacted, inputSize);
    free(compacted);

    // fuzz uncompactCells using the original input
    int64_t uncompactedSize;
    H3Error err =
        H3_EXPORT(uncompactCellsSize)(input, inputSize, res, &uncompactedSize);

    if (!err && uncompactedSize < MAX_UNCOMPACT_SIZE) {
        H3Index *uncompacted = calloc(uncompactedSize, sizeof(H3Index));
        H3_EXPORT(uncompactCells)
        (input, inputSize, uncompacted, uncompactedSize, res);
        free(uncompacted);
    }

    return 0;
}

AFL_HARNESS_MAIN(sizeof(H3Index) * 1024);
