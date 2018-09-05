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
#include "benchmark.h"
#include "h3Index.h"
#include "h3api.h"

// Fixtures
H3Index hex = 0x89283080ddbffff;
H3Index pentagon = 0x89080000003ffff;

BEGIN_BENCHMARKS();

H3Index* out = malloc(H3_EXPORT(maxKringSize)(40) * sizeof(H3Index));

BENCHMARK(kRing10, 10000, { H3_EXPORT(kRing)(hex, 10, out); });
BENCHMARK(kRing20, 10000, { H3_EXPORT(kRing)(hex, 20, out); });
BENCHMARK(kRing30, 10000, { H3_EXPORT(kRing)(hex, 30, out); });
BENCHMARK(kRing40, 10000, { H3_EXPORT(kRing)(hex, 40, out); });

BENCHMARK(kRingPentagon10, 500, { H3_EXPORT(kRing)(pentagon, 10, out); });
BENCHMARK(kRingPentagon20, 500, { H3_EXPORT(kRing)(pentagon, 20, out); });
BENCHMARK(kRingPentagon30, 50, { H3_EXPORT(kRing)(pentagon, 30, out); });
BENCHMARK(kRingPentagon40, 10, { H3_EXPORT(kRing)(pentagon, 40, out); });

free(out);

END_BENCHMARKS();
