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
#include "benchmark.h"
#include "h3api.h"

typedef struct {
    H3Index *cells;
    int64_t N;
} CellArray;

CellArray pentagonSetup(int parentRes, int childRes, int nullEvery) {
    // Starting with a parent *pentagon* cell of resolution `parentRes`,
    // write an array of all its children at resolution `childRes`.
    //
    // If `nullEvery > 0`, then modify the array to have H3_NULL
    // every `nullEvery` indices.
    H3Index pParent = 0x80c3fffffffffff;  // res 0 pentagon
    H3Index p;
    H3_EXPORT(cellToCenterChild)(pParent, parentRes, &p);

    CellArray ca;

    H3_EXPORT(cellToChildrenSize)(p, childRes, &(ca.N));
    ca.cells = calloc(ca.N, sizeof(H3Index));
    H3_EXPORT(cellToChildren)(p, childRes, ca.cells);

    if (nullEvery > 0) {
        // Note: `isValidCell` should return False on H3_NULL values.
        for (int64_t i = 0; i < ca.N; i += nullEvery) {
            ca.cells[i] = H3_NULL;
        }
    }

    return ca;
}

static inline void runValidation(const CellArray ca) {
    // Apply `isValidCell` to every element of `ca.cells`.
    for (int64_t i = 0; i < ca.N; i++) {
        H3_EXPORT(isValidCell)(ca.cells[i]);
    }
}

CellArray ca;

BEGIN_BENCHMARKS();

// pentagon 2->8
ca = pentagonSetup(2, 8, 0);
BENCHMARK(pentagonChildren_2_8, 1000, { runValidation(ca); });
free(ca.cells);

// pentagon 8->14
ca = pentagonSetup(8, 14, 0);
BENCHMARK(pentagonChildren_8_14, 1000, { runValidation(ca); });
free(ca.cells);

// pentagon 8->14; H3_NULL every 2
ca = pentagonSetup(8, 14, 2);
BENCHMARK(pentagonChildren_8_14_null_2, 1000, { runValidation(ca); });
free(ca.cells);

// pentagon 8->14; H3_NULL every 10
ca = pentagonSetup(8, 14, 10);
BENCHMARK(pentagonChildren_8_14_null_10, 1000, { runValidation(ca); });
free(ca.cells);

// pentagon 8->14; H3_NULL every 100
ca = pentagonSetup(8, 14, 100);
BENCHMARK(pentagonChildren_8_14_null_100, 1000, { runValidation(ca); });
free(ca.cells);

END_BENCHMARKS();
