/*
 * Copyright 2020-2021 Uber Technologies, Inc.
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
/** @file
 * @brief tests main H3 core library memory management.
 *
 *  usage: `testH3Memory`
 */

#include <math.h>
#include <string.h>

#include "h3Index.h"
#include "h3api.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

// Whether to fail all allocations
static bool failAlloc = false;
// Actual number of malloc/calloc/realloc calls observed
static int actualAllocCalls = 0;
// Actual number of free calls observed
static int actualFreeCalls = 0;
// Set to non-zero to begin failing allocations after a certain number of calls
static int permittedAllocCalls = 0;

void resetMemoryCounters(int permitted) {
    failAlloc = false;
    actualAllocCalls = 0;
    actualFreeCalls = 0;
    permittedAllocCalls = permitted;
}

void *test_prefix_malloc(size_t size) {
    actualAllocCalls++;
    if (permittedAllocCalls && actualAllocCalls > permittedAllocCalls) {
        failAlloc = true;
    }
    if (failAlloc) {
        return NULL;
    }
    return malloc(size);
}

void *test_prefix_calloc(size_t num, size_t size) {
    actualAllocCalls++;
    if (permittedAllocCalls && actualAllocCalls > permittedAllocCalls) {
        failAlloc = true;
    }
    if (failAlloc) {
        return NULL;
    }
    return calloc(num, size);
}

void *test_prefix_realloc(void *ptr, size_t size) {
    actualAllocCalls++;
    if (permittedAllocCalls && actualAllocCalls > permittedAllocCalls) {
        failAlloc = true;
    }
    if (failAlloc) {
        return NULL;
    }
    return realloc(ptr, size);
}

void test_prefix_free(void *ptr) {
    actualFreeCalls++;
    return free(ptr);
}

H3Index sunnyvale = 0x89283470c27ffff;
H3Index pentagon = 0x89080000003ffff;

static LatLng sfVerts[] = {
    {0.659966917655, -2.1364398519396},  {0.6595011102219, -2.1359434279405},
    {0.6583348114025, -2.1354884206045}, {0.6581220034068, -2.1382437718946},
    {0.6594479998527, -2.1384597563896}, {0.6599990002976, -2.1376771158464}};
static GeoLoop sfGeoLoop = {.numVerts = 6, .verts = sfVerts};
static GeoPolygon sfGeoPolygon;

SUITE(h3Memory) {
    TEST(gridDisk) {
        int k = 2;
        int64_t hexCount;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &hexCount));
        H3Index *gridDiskOutput = calloc(hexCount, sizeof(H3Index));

        resetMemoryCounters(0);
        t_assertSuccess(H3_EXPORT(gridDisk)(sunnyvale, k, gridDiskOutput));
        t_assert(actualAllocCalls == 0, "gridDisk did not call alloc");
        t_assert(actualFreeCalls == 0, "gridDisk did not call free");

        resetMemoryCounters(0);
        t_assertSuccess(H3_EXPORT(gridDisk)(pentagon, k, gridDiskOutput));
        t_assert(actualAllocCalls == 1, "gridDisk called alloc");
        t_assert(actualFreeCalls == 1, "gridDisk called free");

        resetMemoryCounters(0);
        failAlloc = true;
        memset(gridDiskOutput, 0, hexCount * sizeof(H3Index));
        t_assert(
            H3_EXPORT(gridDisk)(pentagon, k, gridDiskOutput) == E_MEMORY_ALLOC,
            "gridDisk returns E_MEMORY_ALLOC");
        t_assert(actualAllocCalls == 1, "gridDisk called alloc");
        t_assert(actualFreeCalls == 0, "gridDisk did not call free");

        for (int64_t i = 0; i < hexCount; i++) {
            t_assert(!gridDiskOutput[i],
                     "gridDisk did not produce output without alloc");
        }

        free(gridDiskOutput);
    }

    TEST(compactCells) {
        int k = 9;
        int64_t hexCount;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &hexCount));
        int64_t expectedCompactCount = 73;

        // Generate a set of hexagons to compact
        H3Index *sunnyvaleExpanded = calloc(hexCount, sizeof(H3Index));
        resetMemoryCounters(0);
        t_assertSuccess(H3_EXPORT(gridDisk)(sunnyvale, k, sunnyvaleExpanded));
        t_assert(actualAllocCalls == 0, "gridDisk did not call alloc");
        t_assert(actualFreeCalls == 0, "gridDisk did not call free");

        H3Index *compressed = calloc(hexCount, sizeof(H3Index));

        resetMemoryCounters(0);
        failAlloc = true;
        H3Error err =
            H3_EXPORT(compactCells)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == E_MEMORY_ALLOC, "malloc failed (1)");
        t_assert(actualAllocCalls == 1, "alloc called once");
        t_assert(actualFreeCalls == 0, "free not called");

        resetMemoryCounters(1);
        err = H3_EXPORT(compactCells)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == E_MEMORY_ALLOC, "malloc failed (2)");
        t_assert(actualAllocCalls == 2, "alloc called twice");
        t_assert(actualFreeCalls == 1, "free called once");

        resetMemoryCounters(2);
        err = H3_EXPORT(compactCells)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == E_MEMORY_ALLOC, "malloc failed (3)");
        t_assert(actualAllocCalls == 3, "alloc called three times");
        t_assert(actualFreeCalls == 2, "free called twice");

        resetMemoryCounters(3);
        err = H3_EXPORT(compactCells)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == E_MEMORY_ALLOC, "compactCells failed (4)");
        t_assert(actualAllocCalls == 4, "alloc called four times");
        t_assert(actualFreeCalls == 3, "free called three times");

        resetMemoryCounters(4);
        err = H3_EXPORT(compactCells)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == E_SUCCESS, "compact using successful malloc");
        t_assert(actualAllocCalls == 4, "alloc called four times");
        t_assert(actualFreeCalls == 4, "free called four times");

        int64_t count = 0;
        for (int64_t i = 0; i < hexCount; i++) {
            if (compressed[i] != 0) {
                count++;
            }
        }
        t_assert(count == expectedCompactCount, "got expected compact count");

        free(compressed);
        free(sunnyvaleExpanded);
    }

    TEST(polygonToCells) {
        sfGeoPolygon.geoloop = sfGeoLoop;
        sfGeoPolygon.numHoles = 0;

        int64_t numHexagons;
        t_assertSuccess(H3_EXPORT(maxPolygonToCellsSize)(&sfGeoPolygon, 9, 0,
                                                         &numHexagons));
        H3Index *hexagons = calloc(numHexagons, sizeof(H3Index));

        resetMemoryCounters(0);
        failAlloc = true;
        H3Error err = H3_EXPORT(polygonToCells)(&sfGeoPolygon, 9, 0, hexagons);
        t_assert(err == E_MEMORY_ALLOC, "polygonToCells failed (1)");
        t_assert(actualAllocCalls == 1, "alloc called once");
        t_assert(actualFreeCalls == 0, "free not called");

        resetMemoryCounters(1);
        err = H3_EXPORT(polygonToCells)(&sfGeoPolygon, 9, 0, hexagons);
        t_assert(err == E_MEMORY_ALLOC, "polygonToCells failed (2)");
        t_assert(actualAllocCalls == 2, "alloc called twice");
        t_assert(actualFreeCalls == 1, "free called once");

        resetMemoryCounters(2);
        err = H3_EXPORT(polygonToCells)(&sfGeoPolygon, 9, 0, hexagons);
        t_assert(err == E_MEMORY_ALLOC, "polygonToCells failed (3)");
        t_assert(actualAllocCalls == 3, "alloc called three times");
        t_assert(actualFreeCalls == 2, "free called twice");

        resetMemoryCounters(3);
        err = H3_EXPORT(polygonToCells)(&sfGeoPolygon, 9, 0, hexagons);
        t_assert(err == E_SUCCESS, "polygonToCells succeeded (4)");
        t_assert(actualAllocCalls == 3, "alloc called three times");
        t_assert(actualFreeCalls == 3, "free called three times");

        int64_t actualNumIndexes = countNonNullIndexes(hexagons, numHexagons);
        t_assert(actualNumIndexes == 1253, "got expected polygonToCells size");
        free(hexagons);
    }
}
