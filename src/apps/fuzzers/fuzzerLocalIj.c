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
 * @brief Fuzzer program for local IJ and related functions (gridDistance,
 * gridPathCells)
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    H3Index index;
    H3Index index2;
    int i;
    int j;
    uint32_t mode;
} inputArgs;

void testTwoIndexes(H3Index index, H3Index index2) {
    int64_t distance;
    H3_EXPORT(gridDistance)(index, index2, &distance);
    int64_t size;
    H3Error err = H3_EXPORT(gridPathCellsSize)(index, index2, &size);
    if (!err) {
        H3Index *output = calloc(size, sizeof(H3Index));
        H3_EXPORT(gridPathCells)(index, index2, output);
        free(output);
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    // Note that index and index2 need to be in the approximate area for these
    // tests to make sense.
    // Test with mode set to 0 since that is expected to yield more interesting
    // results.
    testTwoIndexes(args->index, args->index2);
    H3Index out;
    CoordIJ ij = {.i = args->i, .j = args->j};
    H3Error err = H3_EXPORT(localIjToCell)(args->index, &ij, 0, &out);
    if (!err) {
        testTwoIndexes(args->index, out);
    }

    H3_EXPORT(cellToLocalIj)(args->index, args->index2, 0, &ij);

    // Test again with mode
    uint32_t mode = args->mode;
    err = H3_EXPORT(localIjToCell)(args->index, &ij, mode, &out);
    if (!err) {
        testTwoIndexes(args->index, out);
    }

    H3_EXPORT(cellToLocalIj)(args->index, args->index2, mode, &ij);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
