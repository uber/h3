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
#include "algos.h"
#include "benchmark.h"
#include "h3api.h"
#include "linkedGeo.h"

// Fixtures
H3Index ring2[] = {
    0x8930062838bffff, 0x8930062838fffff, 0x89300628383ffff, 0x8930062839bffff,
    0x893006283d7ffff, 0x893006283c7ffff, 0x89300628313ffff, 0x89300628317ffff,
    0x893006283bbffff, 0x89300628387ffff, 0x89300628397ffff, 0x89300628393ffff,
    0x89300628067ffff, 0x8930062806fffff, 0x893006283d3ffff, 0x893006283c3ffff,
    0x893006283cfffff, 0x8930062831bffff, 0x89300628303ffff};
int ring2Count = 19;

H3Index donut[] = {0x892830828c7ffff, 0x892830828d7ffff, 0x8928308289bffff,
                   0x89283082813ffff, 0x8928308288fffff, 0x89283082883ffff};
int donutCount = 6;

H3Index nestedDonuts[] = {
    0x89283082813ffff, 0x8928308281bffff, 0x8928308280bffff, 0x8928308280fffff,
    0x89283082807ffff, 0x89283082817ffff, 0x8928308289bffff, 0x892830828d7ffff,
    0x892830828c3ffff, 0x892830828cbffff, 0x89283082853ffff, 0x89283082843ffff,
    0x8928308284fffff, 0x8928308287bffff, 0x89283082863ffff, 0x89283082867ffff,
    0x8928308282bffff, 0x89283082823ffff, 0x89283082837ffff, 0x892830828afffff,
    0x892830828a3ffff, 0x892830828b3ffff, 0x89283082887ffff, 0x89283082883ffff};
int nestedDonutsCount = 24;

BEGIN_BENCHMARKS();
LinkedGeoPolygon polygon;

BENCHMARK(h3SetToLinkedGeoRing2, 10000, {
    H3_EXPORT(h3SetToLinkedGeo)(ring2, ring2Count, &polygon);
    H3_EXPORT(destroyLinkedPolygon)(&polygon);
});

BENCHMARK(h3SetToLinkedGeoDonut, 10000, {
    H3_EXPORT(h3SetToLinkedGeo)(donut, donutCount, &polygon);
    H3_EXPORT(destroyLinkedPolygon)(&polygon);
});

BENCHMARK(h3SetToLinkedGeoNestedDonuts, 10000, {
    H3_EXPORT(h3SetToLinkedGeo)(nestedDonuts, nestedDonutsCount, &polygon);
    H3_EXPORT(destroyLinkedPolygon)(&polygon);
});

END_BENCHMARKS();
