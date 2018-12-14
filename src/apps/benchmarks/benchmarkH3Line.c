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
#include "h3api.h"

// Fixtures
H3Index startIndex = 0x89283082803ffff;
H3Index endNear = 0x892830814b3ffff;
H3Index endFar = 0x8929a5653c3ffff;

BEGIN_BENCHMARKS();

H3Index* out =
    calloc(H3_EXPORT(h3LineSize)(startIndex, endFar), sizeof(H3Index));

BENCHMARK(h3LineNear, 10000, { H3_EXPORT(h3Line)(startIndex, endNear, out); });
BENCHMARK(h3LineFar, 1000, { H3_EXPORT(h3Line)(startIndex, endFar, out); });

free(out);

END_BENCHMARKS();
