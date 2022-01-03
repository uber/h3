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
H3Index pentagon = 0x89080000003ffff;

BEGIN_BENCHMARKS();

int64_t outSz;
if (H3_EXPORT(maxGridDiskSize)(40, &outSz)) {
    printf("Failed\n");
    return 1;
}
H3Index *out = calloc(outSz, sizeof(H3Index));

BENCHMARK(gridDisk10, 10000, { H3_EXPORT(gridDisk)(hex, 10, out); });
BENCHMARK(gridDisk20, 10000, { H3_EXPORT(gridDisk)(hex, 20, out); });
BENCHMARK(gridDisk30, 10000, { H3_EXPORT(gridDisk)(hex, 30, out); });
BENCHMARK(gridDisk40, 10000, { H3_EXPORT(gridDisk)(hex, 40, out); });

BENCHMARK(gridDiskPentagon10, 500, { H3_EXPORT(gridDisk)(pentagon, 10, out); });
BENCHMARK(gridDiskPentagon20, 500, { H3_EXPORT(gridDisk)(pentagon, 20, out); });
BENCHMARK(gridDiskPentagon30, 50, { H3_EXPORT(gridDisk)(pentagon, 30, out); });
BENCHMARK(gridDiskPentagon40, 10, { H3_EXPORT(gridDisk)(pentagon, 40, out); });

free(out);

END_BENCHMARKS();
