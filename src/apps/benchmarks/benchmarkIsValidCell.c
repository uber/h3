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

H3Index p = 0x80c3fffffffffff;  // res 0 pentagon

int parentRes;
int childRes;

int64_t N;
H3Index *cells;

BEGIN_BENCHMARKS();

// pentagon 8->14
parentRes = 8;
childRes = 14;
p = H3_EXPORT(cellToCenterChild)(p, parentRes);
N = H3_EXPORT(cellToChildrenSize)(p, childRes);

cells = calloc(N, sizeof(H3Index));
H3_EXPORT(cellToChildren)(p, childRes, cells);

BENCHMARK(pentagonChildren_8_14, 1000, {
    for (int64_t i = 0; i < N; i++) {
        H3_EXPORT(isValidCell)(cells[i]);
    }
});
free(cells);

// pentagon 2->8
parentRes = 2;
childRes = 8;
p = H3_EXPORT(cellToCenterChild)(p, parentRes);
N = H3_EXPORT(cellToChildrenSize)(p, childRes);

cells = calloc(N, sizeof(H3Index));
H3_EXPORT(cellToChildren)(p, childRes, cells);

BENCHMARK(pentagonChildren_2_8, 1000, {
    for (int64_t i = 0; i < N; i++) {
        H3_EXPORT(isValidCell)(cells[i]);
    }
});
free(cells);

END_BENCHMARKS();
