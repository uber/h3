/*
 * Copyright 2018 Uber Technologies, Inc.
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
/**
 * Example program that finds neighboring indexes (within distance 2)
 * of an index.
 */

#include <h3/h3api.h>
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    H3Index indexed = 0x8a2a1072b59ffffL;
    // Distance away from the origin to find:
    int k = 2;

    int maxNeighboring = maxKringSize(k);
    H3Index* neighboring = calloc(maxNeighboring, sizeof(H3Index));
    kRing(indexed, k, neighboring);

    printf("Neighbors:\n");
    for (int i = 0; i < maxNeighboring; i++) {
        // Some indexes may be 0 to indicate fewer than the maximum
        // number of indexes.
        if (neighboring[i] != 0) {
            printf("%" PRIx64 "\n", neighboring[i]);
        }
    }

    free(neighboring);
}
