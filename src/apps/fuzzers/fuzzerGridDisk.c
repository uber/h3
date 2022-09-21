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
 * @brief Fuzzer program for gridDisk
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    H3Index index;
    int k;
} inputArgs;

// This is limited to avoid timeouts due to the runtime of gridDisk growing with
// k
const int64_t MAX_GRID_DISK_SIZE = 10000;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    int64_t sz;
    H3Error err = H3_EXPORT(maxGridDiskSize)(args->k, &sz);
    if (err || sz > MAX_GRID_DISK_SIZE || sz < 0) {
        // Can't allocate - do not test for cases where sz is extremely large
        // because this is expected to hang, and do not test for cases where
        // sizeof(H3Index) * sz overflows (this is undefined behavior per C
        // and will stop the fuzzer from making progress).
        return 0;
    }
    H3Index *results = calloc(sizeof(H3Index), sz);
    if (results != NULL) {
        H3_EXPORT(gridDisk)(args->index, args->k, results);
    }
    free(results);

    results = calloc(sizeof(H3Index), sz);
    if (results != NULL) {
        H3_EXPORT(gridDiskUnsafe)(args->index, args->k, results);
    }
    free(results);

    // TODO: use int64_t
    int *distances = calloc(sizeof(int), sz);
    results = calloc(sizeof(H3Index), sz);
    if (results != NULL && distances != NULL) {
        H3_EXPORT(gridDiskDistancesUnsafe)
        (args->index, args->k, results, distances);
    }
    free(results);
    free(distances);

    distances = calloc(sizeof(int), sz);
    results = calloc(sizeof(H3Index), sz);
    if (results != NULL && distances != NULL) {
        H3_EXPORT(gridDiskDistancesSafe)
        (args->index, args->k, results, distances);
    }
    free(results);
    free(distances);

    distances = calloc(sizeof(int), sz);
    results = calloc(sizeof(H3Index), sz);
    if (results != NULL && distances != NULL) {
        H3_EXPORT(gridDiskDistances)(args->index, args->k, results, distances);
    }
    free(results);
    free(distances);

    results = calloc(sizeof(H3Index), sz);
    if (results != NULL) {
        H3_EXPORT(gridRingUnsafe)(args->index, args->k, results);
    }
    free(results);

    size_t length = (size - sizeof(inputArgs)) / sizeof(H3Index);
    if (sz * length > MAX_GRID_DISK_SIZE) {
        // Can't allocate.
        return 0;
    }
    results = calloc(sizeof(H3Index), sz * length);
    if (results != NULL) {
        H3Index *h3Set = (H3Index *)(data + sizeof(inputArgs));
        H3_EXPORT(gridDisksUnsafe)(h3Set, length, args->k, results);
    }
    free(results);
    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
