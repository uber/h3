/*
 * Copyright 2018, 2020 Uber Technologies, Inc.
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
#include "h3Index.h"
#include "h3api.h"

// Fixtures
H3Index hex = 0x89283080ddbffff;

BEGIN_BENCHMARKS();

int64_t outSz;
if (H3_EXPORT(cellToChildrenSize)(hex, 14, &outSz)) {
    printf("Failed\n");
    return 1;
}
H3Index *out = calloc(outSz, sizeof(H3Index));

BENCHMARK(cellToChildren1, 10000, { H3_EXPORT(cellToChildren)(hex, 10, out); });
BENCHMARK(cellToChildren2, 10000, { H3_EXPORT(cellToChildren)(hex, 11, out); });
BENCHMARK(cellToChildren3, 10000, { H3_EXPORT(cellToChildren)(hex, 12, out); });
BENCHMARK(cellToChildren4, 10000, { H3_EXPORT(cellToChildren)(hex, 13, out); });
BENCHMARK(cellToChildren5, 10000, { H3_EXPORT(cellToChildren)(hex, 14, out); });

free(out);

END_BENCHMARKS();
