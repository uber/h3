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
 * @file dumpCoreApi.c
 * @brief Dump latLngToCell, cellToLatLng, cellToBoundary results
 *        for cross-branch correctness comparison.
 */

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "h3api.h"
#include "utility.h"

#define N_RANDOM 10000
#define SEED 12345
#define MAX_EXHAUST_RES 4

static const int testResolutions[] = {0, 1, 2, 3, 4, 7, 10, 15};
#define N_RES (sizeof(testResolutions) / sizeof(testResolutions[0]))

static double rand01(void) { return (double)rand() / (double)RAND_MAX; }

static void dumpCellToLatLng(H3Index h) {
    LatLng g;
    H3_EXPORT(cellToLatLng)(h, &g);
    printf("%" PRIx64 " %.17g %.17g\n", h, g.lat, g.lng);
}

static void dumpCellToBoundary(H3Index h) {
    CellBoundary cb;
    H3_EXPORT(cellToBoundary)(h, &cb);
    printf("%" PRIx64 " %d", h, cb.numVerts);
    for (int i = 0; i < cb.numVerts; i++) {
        printf(" %.17g %.17g", cb.verts[i].lat, cb.verts[i].lng);
    }
    printf("\n");
}

int main(void) {
    // Section 1: latLngToCell with random points
    printf("#section latLngToCell\n");
    srand(SEED);
    for (int i = 0; i < N_RANDOM; i++) {
        // Uniform distribution on the sphere
        double lat = asin(2.0 * rand01() - 1.0);
        double lng = (2.0 * rand01() - 1.0) * M_PI;
        LatLng g = {lat, lng};
        for (int r = 0; r < (int)N_RES; r++) {
            H3Index h;
            H3_EXPORT(latLngToCell)(&g, testResolutions[r], &h);
            printf("%.17g %.17g %d %" PRIx64 "\n", lat, lng,
                   testResolutions[r], h);
        }
    }

    // Section 2: cellToLatLng exhaustive
    printf("#section cellToLatLng\n");
    for (int res = 0; res <= MAX_EXHAUST_RES; res++) {
        iterateAllIndexesAtRes(res, dumpCellToLatLng);
    }

    // Section 3: cellToBoundary exhaustive
    printf("#section cellToBoundary\n");
    for (int res = 0; res <= MAX_EXHAUST_RES; res++) {
        iterateAllIndexesAtRes(res, dumpCellToBoundary);
    }

    return 0;
}
