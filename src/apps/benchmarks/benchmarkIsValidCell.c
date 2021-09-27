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

// return a pentagon at the given resolution
H3Index pentagonAtRes(int res) {
    H3Index p = 0x80c3fffffffffff;  // res 0 pentagon
    p = H3_EXPORT(cellToCenterChild)(p, res);

    return p;
}

int parentRes;
int childRes;

H3Index p;
int64_t N;
H3Index *cells;

BEGIN_BENCHMARKS();

// pentagon 8->14
//
// Starting with a parent *pentagon* cell of resolution 8,
// loop through all its children at resolution 14, applying `isValidCell`
parentRes = 8;
childRes = 14;

p = pentagonAtRes(parentRes);
H3_EXPORT(cellToChildrenSize)(p, childRes, &N);

cells = calloc(N, sizeof(H3Index));
H3_EXPORT(cellToChildren)(p, childRes, cells);

BENCHMARK(pentagonChildren_8_14, 1000, {
    for (int64_t i = 0; i < N; i++) {
        H3_EXPORT(isValidCell)(cells[i]);
    }
});
free(cells);

// pentagon 2->8
//
// Starting with a parent *pentagon* cell of resolution 2,
// loop through all its children at resolution 8, applying `isValidCell`
parentRes = 2;
childRes = 8;
p = pentagonAtRes(parentRes);
H3_EXPORT(cellToChildrenSize)(p, childRes, &N);

cells = calloc(N, sizeof(H3Index));
H3_EXPORT(cellToChildren)(p, childRes, cells);

BENCHMARK(pentagonChildren_2_8, 1000, {
    for (int64_t i = 0; i < N; i++) {
        H3_EXPORT(isValidCell)(cells[i]);
    }
});
free(cells);

END_BENCHMARKS();
