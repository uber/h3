/*
 * Copyright 2020 Uber Technologies, Inc.
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
#include "geoCoord.h"
#include "h3api.h"

// Fixtures (arbitrary res 9 hexagon)
H3Index edges[6] = {0};
H3Index hex = 0x89283080ddbffff;

BEGIN_BENCHMARKS();

GeoBoundary outBoundary;
H3_EXPORT(getH3UnidirectionalEdgesFromHexagon)(hex, edges);

BENCHMARK(getH3UnidirectionalEdgeBoundary, 10000, {
    for (int i = 0; i < 6; i++)
        H3_EXPORT(getH3UnidirectionalEdgeBoundary)(edges[i], &outBoundary);
});

END_BENCHMARKS();
