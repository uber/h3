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
#include "vertex.h"

// Fixtures. Cells are arbitrary, except that ring2 is all hexagons and
// ring2Pent is centered on a pentagon.

H3Index hex = 0x89283080ddbffff;
H3Index pentagon = 0x89080000003ffff;

H3Index ring2[] = {
    0x89283081083ffff, 0x8928308109bffff, 0x8928308108bffff, 0x8928308108fffff,
    0x89283081087ffff, 0x89283081097ffff, 0x89283081093ffff, 0x89283081467ffff,
    0x8928308146fffff, 0x892830810d7ffff, 0x892830810c7ffff, 0x89283081013ffff,
    0x89283081017ffff, 0x892830810bbffff, 0x892830810b3ffff, 0x8928308154bffff,
    0x8928308155bffff, 0x8928308142fffff, 0x8928308142bffff};
int ring2Count = 19;

H3Index ring2Pent[] = {
    0x8508008bfffffff, 0x8508000ffffffff, 0x85080077fffffff, 0x85080047fffffff,
    0x85080017fffffff, 0x85080003fffffff, 0x8508000bfffffff, 0x85080073fffffff,
    0x85080057fffffff, 0x850800abfffffff, 0x8508008ffffffff, 0x85080013fffffff,
    0x8508001bfffffff, 0x850800c7fffffff, 0x850800cffffffff, 0x850800bbfffffff};
int ring2PentCount = 16;

BEGIN_BENCHMARKS();

H3Index *vertexes = calloc(6, sizeof(H3Index));

BENCHMARK(cellToVertexes, 10000, { H3_EXPORT(cellToVertexes)(hex, vertexes); });

BENCHMARK(cellToVertexesPent, 10000, {
    H3_EXPORT(cellToVertexes)(pentagon, vertexes);
});

BENCHMARK(cellToVertexesRing, 10000, {
    for (int i = 0; i < ring2Count; i++) {
        H3_EXPORT(cellToVertexes)(ring2[i], vertexes);
    }
});

BENCHMARK(cellToVertexesRingPent, 10000, {
    for (int i = 0; i < ring2PentCount; i++) {
        H3_EXPORT(cellToVertexes)(ring2Pent[i], vertexes);
    }
});

free(vertexes);

END_BENCHMARKS();
