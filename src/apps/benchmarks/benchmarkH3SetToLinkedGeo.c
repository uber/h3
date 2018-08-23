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
#include "algos.h"
#include "benchmark.h"
#include "h3api.h"
#include "linkedGeo.h"

// Fixtures
H3Index ring2[] = {0x8930062838bffffl, 0x8930062838fffffl, 0x89300628383ffffl,
                   0x8930062839bffffl, 0x893006283d7ffffl, 0x893006283c7ffffl,
                   0x89300628313ffffl, 0x89300628317ffffl, 0x893006283bbffffl,
                   0x89300628387ffffl, 0x89300628397ffffl, 0x89300628393ffffl,
                   0x89300628067ffffl, 0x8930062806fffffl, 0x893006283d3ffffl,
                   0x893006283c3ffffl, 0x893006283cfffffl, 0x8930062831bffffl,
                   0x89300628303ffffl};
int ring2Count = 19;

H3Index donut[] = {0x892830828c7ffffl, 0x892830828d7ffffl, 0x8928308289bffffl,
                   0x89283082813ffffl, 0x8928308288fffffl, 0x89283082883ffffl};
int donutCount = 6;

H3Index nestedDonuts[] = {
    0x89283082813ffffl, 0x8928308281bffffl, 0x8928308280bffffl,
    0x8928308280fffffl, 0x89283082807ffffl, 0x89283082817ffffl,
    0x8928308289bffffl, 0x892830828d7ffffl, 0x892830828c3ffffl,
    0x892830828cbffffl, 0x89283082853ffffl, 0x89283082843ffffl,
    0x8928308284fffffl, 0x8928308287bffffl, 0x89283082863ffffl,
    0x89283082867ffffl, 0x8928308282bffffl, 0x89283082823ffffl,
    0x89283082837ffffl, 0x892830828afffffl, 0x892830828a3ffffl,
    0x892830828b3ffffl, 0x89283082887ffffl, 0x89283082883ffffl};
int nestedDonutsCount = 24;

BEGIN_BENCHMARKS();
LinkedGeoPolygon polygon;

BENCHMARK(h3SetToLinkedGeoRing2, 10000, {
    initLinkedPolygon(&polygon);
    H3_EXPORT(h3SetToLinkedGeo)(ring2, ring2Count, &polygon);
    H3_EXPORT(destroyLinkedPolygon)(&polygon);
});

BENCHMARK(h3SetToLinkedGeoDonut, 10000, {
    initLinkedPolygon(&polygon);
    H3_EXPORT(h3SetToLinkedGeo)(donut, donutCount, &polygon);
    H3_EXPORT(destroyLinkedPolygon)(&polygon);
});

BENCHMARK(h3SetToLinkedGeoNestedDonuts, 10000, {
    initLinkedPolygon(&polygon);
    H3_EXPORT(h3SetToLinkedGeo)(nestedDonuts, nestedDonutsCount, &polygon);
    H3_EXPORT(destroyLinkedPolygon)(&polygon);
});

END_BENCHMARKS();
