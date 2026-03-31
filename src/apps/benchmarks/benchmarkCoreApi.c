/*
 * Copyright 2026 Uber Technologies, Inc.
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

/**
 * @file benchmarkCoreApi.c
 * @brief Benchmark latLngToCell, cellToLatLng, cellToBoundary,
 *        directedEdgeToBoundary with diverse inputs.
 */

#include "benchmark.h"
#include "h3api.h"

#define N_POINTS 20
#define N_RESOLUTIONS 4
#define ITERATIONS 50000

// Points spread across the globe (hitting different icosahedron faces)
static const LatLng points[N_POINTS] = {
    {0.659966917655, -2.1364398519396}, {0.8527087756, -0.0405865662},
    {0.6234025842, 2.0075945568},       {-0.5934119457, 2.5368879644},
    {0.4799655443, 0.6457718232},       {-0.4014257280, -0.7610418886},
    {0.9679776674, -1.7453292520},      {-1.2217304764, 0.0000000000},
    {1.2217304764, 0.0000000000},       {0.0000000000, 0.0000000000},
    {0.0000000000, 3.1415926536},       {0.7853981634, 1.5707963268},
    {-0.7853981634, -1.5707963268},     {0.3490658504, -1.2217304764},
    {-0.1745329252, 0.5235987756},      {1.0471975512, -0.5235987756},
    {-1.0471975512, 2.0943951024},      {0.2617993878, 1.8325957146},
    {-0.8726646260, -1.0471975512},     {0.5235987756, -2.6179938780},
};

static const int resolutions[N_RESOLUTIONS] = {0, 5, 9, 15};

H3Index cells[N_POINTS * N_RESOLUTIONS];
int nCells = 0;
H3Index edges[N_POINTS * N_RESOLUTIONS];
int nEdges = 0;

BEGIN_BENCHMARKS();

// Pre-compute cells and edges
for (int r = 0; r < N_RESOLUTIONS; r++) {
    for (int p = 0; p < N_POINTS; p++) {
        H3_EXPORT(latLngToCell)(&points[p], resolutions[r], &cells[nCells]);
        nCells++;
    }
}
for (int c = 0; c < nCells; c++) {
    H3Index out[6];
    H3_EXPORT(originToDirectedEdges)(cells[c], out);
    edges[nEdges++] = out[0];
}

H3Index h;
LatLng outCoord;
CellBoundary cb;

BENCHMARK(latLngToCell, ITERATIONS, {
    for (int r = 0; r < N_RESOLUTIONS; r++) {
        for (int p = 0; p < N_POINTS; p++) {
            H3_EXPORT(latLngToCell)(&points[p], resolutions[r], &h);
        }
    }
});

BENCHMARK(cellToLatLng, ITERATIONS, {
    for (int c = 0; c < nCells; c++) {
        H3_EXPORT(cellToLatLng)(cells[c], &outCoord);
    }
});

BENCHMARK(cellToBoundary, ITERATIONS, {
    for (int c = 0; c < nCells; c++) {
        H3_EXPORT(cellToBoundary)(cells[c], &cb);
    }
});

BENCHMARK(directedEdgeToBoundary, ITERATIONS, {
    for (int e = 0; e < nEdges; e++) {
        H3_EXPORT(directedEdgeToBoundary)(edges[e], &cb);
    }
});

END_BENCHMARKS();
