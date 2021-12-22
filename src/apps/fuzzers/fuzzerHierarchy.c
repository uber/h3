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
 * @brief Fuzzer program for cellToLatLng and cellToBoundary
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

#define MAX_CHILDREN_DIFF 10

typedef struct {
    H3Index index;
    int parentRes;
    int childRes;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    H3Index parent;
    H3_EXPORT(cellToParent)(args->index, args->parentRes, &parent);
    h3Println(parent);

    // TODO: Update with new API
    H3Index child = H3_EXPORT(cellToCenterChild)(args->index, args->childRes);
    h3Println(child);

    int resDiff = args->childRes - H3_EXPORT(getResolution)(args->index);
    if (resDiff < MAX_CHILDREN_DIFF) {
        int64_t childrenSize;
        H3Error err = H3_EXPORT(cellToChildrenSize)(args->index, args->childRes,
                                                    &childrenSize);
        if (!err) {
            H3Index *children = calloc(childrenSize, sizeof(H3Index));
            H3_EXPORT(cellToChildren)(args->index, args->childRes, children);
            h3Println(children[0]);
            free(children);
        }
    }
    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
