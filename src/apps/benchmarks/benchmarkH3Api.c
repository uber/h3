/*
 * Copyright 2017, 2020 Uber Technologies, Inc.
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
GeoPoint coord = {0.659966917655, -2.1364398519396};
H3Index hex = 0x89283080ddbffff;

BEGIN_BENCHMARKS();

GeoPoint outCoord;
CellBoundary outBoundary;

BENCHMARK(pointToCell, 10000, { H3_EXPORT(pointToCell)(&coord, 9); });

BENCHMARK(cellToPoint, 10000, { H3_EXPORT(cellToPoint)(hex, &outCoord); });

BENCHMARK(cellToBoundary, 10000, {
    H3_EXPORT(cellToBoundary)(hex, &outBoundary);
});

END_BENCHMARKS();
