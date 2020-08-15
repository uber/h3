/*
 * Copyright 2017 Uber Technologies, Inc.
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
GeoCoord coord = {0.659966917655, -2.1364398519396};
H3Index hex = 0x89283080ddbffff;

BEGIN_BENCHMARKS();

GeoCoord outCoord;
GeoBoundary outBoundary;

BENCHMARK(geoToH3, 10000, { H3_EXPORT(geoToH3)(&coord, 9); });

BENCHMARK(h3ToGeo, 10000, { H3_EXPORT(h3ToGeo)(hex, &outCoord); });

BENCHMARK(h3ToGeoBoundary, 10000, {
    H3_EXPORT(h3ToGeoBoundary)(hex, &outBoundary);
});

END_BENCHMARKS();
