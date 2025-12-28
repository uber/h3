/*
 * Copyright 2018, 2025 Uber Technologies, Inc.
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
#include <stdlib.h>

#include "benchmark.h"
#include "h3api.h"

/**
 * Macro to benchmark both `cellsToLinkedMultiPolygon` and
 * `cellsToMultiPolygon` on the same dataset.
 */
#define BENCHMARK_LINKED(NAME, ITERS)                                       \
    do {                                                                    \
        BENCHMARK(linked_##NAME, ITERS, {                                   \
            LinkedGeoPolygon lmpoly;                                        \
            H3_EXPORT(cellsToLinkedMultiPolygon)(cells, numCells, &lmpoly); \
            H3_EXPORT(destroyLinkedMultiPolygon)(&lmpoly);                  \
        });                                                                 \
    } while (0)

#define BENCHMARK_DIRECT(NAME, ITERS)                                \
    do {                                                             \
        BENCHMARK(direct_##NAME, ITERS, {                            \
            GeoMultiPolygon mpoly;                                   \
            H3_EXPORT(cellsToMultiPolygon)(cells, numCells, &mpoly); \
            H3_EXPORT(destroyGeoMultiPolygon)(&mpoly);               \
        });                                                          \
    } while (0)

BEGIN_BENCHMARKS();

{
    // disk2: A filled-in 2-disk
    int numCells = 19;
    H3Index cells[] = {
        0x8930062838bffff, 0x8930062838fffff, 0x89300628383ffff,
        0x8930062839bffff, 0x893006283d7ffff, 0x893006283c7ffff,
        0x89300628313ffff, 0x89300628317ffff, 0x893006283bbffff,
        0x89300628387ffff, 0x89300628397ffff, 0x89300628393ffff,
        0x89300628067ffff, 0x8930062806fffff, 0x893006283d3ffff,
        0x893006283c3ffff, 0x893006283cfffff, 0x8930062831bffff,
        0x89300628303ffff,
    };

    BENCHMARK_LINKED(disk2, 10000);
    BENCHMARK_DIRECT(disk2, 10000);
}

{
    // Donut: A 1-ring
    int numCells = 6;
    H3Index cells[] = {
        0x892830828c7ffff, 0x892830828d7ffff, 0x8928308289bffff,
        0x89283082813ffff, 0x8928308288fffff, 0x89283082883ffff,
    };

    BENCHMARK_LINKED(donut, 10000);
    BENCHMARK_DIRECT(donut, 10000);
}

{
    // Nested donuts: A 1-ring and a 3-ring
    int numCells = 24;
    H3Index cells[] = {
        0x89283082813ffff, 0x8928308281bffff, 0x8928308280bffff,
        0x8928308280fffff, 0x89283082807ffff, 0x89283082817ffff,
        0x8928308289bffff, 0x892830828d7ffff, 0x892830828c3ffff,
        0x892830828cbffff, 0x89283082853ffff, 0x89283082843ffff,
        0x8928308284fffff, 0x8928308287bffff, 0x89283082863ffff,
        0x89283082867ffff, 0x8928308282bffff, 0x89283082823ffff,
        0x89283082837ffff, 0x892830828afffff, 0x892830828a3ffff,
        0x892830828b3ffff, 0x89283082887ffff, 0x89283082883ffff,
    };

    BENCHMARK_LINKED(nestedDonuts, 10000);
    BENCHMARK_DIRECT(nestedDonuts, 10000);
}

{
    // Uncompact a single cell to some target res
    H3Index h = 0x8075fffffffffff;
    int child_res = 5;

    int64_t numCells;
    H3_EXPORT(cellToChildrenSize)(h, child_res, &numCells);

    H3Index *cells = malloc(numCells * sizeof(H3Index));
    H3_EXPORT(cellToChildren)(h, child_res, cells);

    BENCHMARK_LINKED(manyChildren, 10);
    BENCHMARK_DIRECT(manyChildren, 10);

    free(cells);
}

{
    // Colorado, approximately. (4 corners, counterclockwise from southwest)
    int res = 6;
    LatLng verts[] = {
        {37.0, -109.0},
        {37.0, -102.0},
        {41.0, -102.0},
        {41.0, -109.0},
    };

    for (int i = 0; i < 4; i++) {
        verts[i].lat = H3_EXPORT(degsToRads)(verts[i].lat);
        verts[i].lng = H3_EXPORT(degsToRads)(verts[i].lng);
    }

    GeoPolygon polygon = {
        .numHoles = 0,
        .holes = NULL,
        .geoloop = {.numVerts = 4, .verts = verts},
    };

    int64_t maxCells;
    H3_EXPORT(maxPolygonToCellsSize)(&polygon, res, 0, &maxCells);

    H3Index *cells = calloc(maxCells, sizeof(H3Index));
    H3_EXPORT(polygonToCells)(&polygon, res, 0, cells);

    // Move all valid cells to the front and resize
    int64_t numCells = 0;
    for (int64_t i = 0; i < maxCells; i++) {
        if (cells[i] != H3_NULL) {
            cells[numCells++] = cells[i];
        }
    }
    cells = realloc(cells, numCells * sizeof(H3Index));

    BENCHMARK_LINKED(colorado, 100);
    BENCHMARK_DIRECT(colorado, 100);

    free(cells);
}

END_BENCHMARKS();
