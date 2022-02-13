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
 * @brief Fuzzer program for cellsToDirectedEdge and related functions
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    H3Index index;
    H3Index index2;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    // Note that index and index2 need to be plausibly neighbors
    // for most of these
    int outBool;
    H3_EXPORT(areNeighborCells)(args->index, args->index2, &outBool);
    H3Index out;
    H3_EXPORT(cellsToDirectedEdge)(args->index, args->index2, &out);
    H3_EXPORT(isValidDirectedEdge)(args->index);
    H3_EXPORT(getDirectedEdgeOrigin)(args->index, &out);
    H3_EXPORT(getDirectedEdgeDestination)(args->index, &out);
    H3Index out2[2];
    H3_EXPORT(directedEdgeToCells)(args->index, out2);
    H3Index out6[6];
    H3_EXPORT(originToDirectedEdges)(args->index, out6);
    CellBoundary bndry;
    H3_EXPORT(directedEdgeToBoundary)(args->index, &bndry);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
