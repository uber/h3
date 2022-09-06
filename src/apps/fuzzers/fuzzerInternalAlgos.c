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
 * @brief Fuzzer program for internal functions in algos.c
 */

#include "aflHarness.h"
#include "algos.h"
#include "h3api.h"
#include "utility.h"

typedef struct {
    H3Index index;
    Direction dir;
    int rotations;
    H3Index index2;
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    const inputArgs *args = (const inputArgs *)data;

    H3Index out;
    int rotations = args->rotations;
    h3NeighborRotations(args->index, args->dir, &rotations, &out);

    directionForNeighbor(args->index, args->index2);

    VertexGraph graph;
    H3Index *h3Set = (H3Index *)data;
    size_t inputSize = size / sizeof(H3Index);
    H3Error err = h3SetToVertexGraph(h3Set, inputSize, &graph);
    if (!err) {
        LinkedGeoPolygon linkedGeoPolygon;
        _vertexGraphToLinkedGeo(&graph, &linkedGeoPolygon);
        H3_EXPORT(destroyLinkedMultiPolygon)(&linkedGeoPolygon);
        destroyVertexGraph(&graph);
    }
    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
