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
 * @brief Benchmark latLngToCell, cellToLatLng, cellToBoundary with
 *        diverse inputs across faces and resolutions.
 */

#include <stdio.h>
#include <time.h>

#include "h3api.h"

#define N_POINTS 20
#define N_RESOLUTIONS 4
#define ITERATIONS 50000

// Points spread across the globe (hitting different icosahedron faces)
static const LatLng points[N_POINTS] = {
    {0.659966917655, -2.1364398519396},   // San Francisco area
    {0.8527087756, -0.0405865662},        // London area
    {0.6234025842, 2.0075945568},         // Tokyo area
    {-0.5934119457, 2.5368879644},        // Sydney area
    {0.4799655443, 0.6457718232},         // Cairo area
    {-0.4014257280, -0.7610418886},       // São Paulo area
    {0.9679776674, -1.7453292520},        // Alaska
    {-1.2217304764, 0.0000000000},        // near south pole
    {1.2217304764, 0.0000000000},         // near north pole
    {0.0000000000, 0.0000000000},         // null island
    {0.0000000000, 3.1415926536},         // antimeridian
    {0.7853981634, 1.5707963268},         // 45N 90E
    {-0.7853981634, -1.5707963268},       // 45S 90W
    {0.3490658504, -1.2217304764},        // 20N 70W
    {-0.1745329252, 0.5235987756},        // 10S 30E
    {1.0471975512, -0.5235987756},        // 60N 30W
    {-1.0471975512, 2.0943951024},        // 60S 120E
    {0.2617993878, 1.8325957146},         // 15N 105E
    {-0.8726646260, -1.0471975512},       // 50S 60W
    {0.5235987756, -2.6179938780},        // 30N 150W
};

static const int resolutions[N_RESOLUTIONS] = {0, 5, 9, 15};

static double elapsed_us(struct timespec *start, struct timespec *end) {
    double sec = end->tv_sec - start->tv_sec;
    double nsec = end->tv_nsec - start->tv_nsec;
    return (sec * 1e9 + nsec) / 1e3;
}

int main(void) {
    H3Index cells[N_POINTS * N_RESOLUTIONS];
    int nCells = 0;

    // Pre-compute cells for inverse benchmarks
    for (int r = 0; r < N_RESOLUTIONS; r++) {
        for (int p = 0; p < N_POINTS; p++) {
            H3_EXPORT(latLngToCell)(&points[p], resolutions[r], &cells[nCells]);
            nCells++;
        }
    }

    // Benchmark latLngToCell
    {
        struct timespec start, end;
        H3Index h;
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int iter = 0; iter < ITERATIONS; iter++) {
            for (int r = 0; r < N_RESOLUTIONS; r++) {
                for (int p = 0; p < N_POINTS; p++) {
                    H3_EXPORT(latLngToCell)(&points[p], resolutions[r], &h);
                }
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        double us = elapsed_us(&start, &end);
        double per_call = us / (ITERATIONS * N_POINTS * N_RESOLUTIONS);
        printf("latLngToCell: %.4f us/call (%d calls)\n",
               per_call, ITERATIONS * N_POINTS * N_RESOLUTIONS);
    }

    // Benchmark cellToLatLng
    {
        struct timespec start, end;
        LatLng out;
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int iter = 0; iter < ITERATIONS; iter++) {
            for (int c = 0; c < nCells; c++) {
                H3_EXPORT(cellToLatLng)(cells[c], &out);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        double us = elapsed_us(&start, &end);
        double per_call = us / (ITERATIONS * nCells);
        printf("cellToLatLng: %.4f us/call (%d calls)\n",
               per_call, ITERATIONS * nCells);
    }

    // Benchmark cellToBoundary
    {
        struct timespec start, end;
        CellBoundary cb;
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int iter = 0; iter < ITERATIONS; iter++) {
            for (int c = 0; c < nCells; c++) {
                H3_EXPORT(cellToBoundary)(cells[c], &cb);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        double us = elapsed_us(&start, &end);
        double per_call = us / (ITERATIONS * nCells);
        printf("cellToBoundary: %.4f us/call (%d calls)\n",
               per_call, ITERATIONS * nCells);
    }

    return 0;
}
