/*
 * Copyright 2020 Uber Technologies, Inc.
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

#include "geoCoord.h"
#include "h3Index.h"
#include "h3api.h"
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

void* test_prefix_malloc(size_t size) {
    actualAllocCalls++;
    if (permittedAllocCalls && actualAllocCalls > permittedAllocCalls) {
        failAlloc = true;
    }
    if (failAlloc) {
        return NULL;
    }
    return malloc(size);
}

void* test_prefix_calloc(size_t num, size_t size) {
    actualAllocCalls++;
    if (permittedAllocCalls && actualAllocCalls > permittedAllocCalls) {
        failAlloc = true;
    }
    if (failAlloc) {
        return NULL;
    }
    return calloc(num, size);
}

void* test_prefix_realloc(void* ptr, size_t size) {
    actualAllocCalls++;
    if (permittedAllocCalls && actualAllocCalls > permittedAllocCalls) {
        failAlloc = true;
    }
    if (failAlloc) {
        return NULL;
    }
    return realloc(ptr, size);
}

void test_prefix_free(void* ptr) {
    actualFreeCalls++;
    return free(ptr);
}

H3Index sunnyvale = 0x89283470c27ffff;
H3Index pentagon = 0x89080000003ffff;

SUITE(h3Memory) {
    TEST(kRing) {
        int k = 2;
        int hexCount = H3_EXPORT(maxKringSize)(k);
        H3Index* kRingOutput = calloc(hexCount, sizeof(H3Index));

        resetMemoryCounters(0);
        H3_EXPORT(kRing)(sunnyvale, k, kRingOutput);
        t_assert(actualAllocCalls == 0, "kRing did not call alloc");
        t_assert(actualFreeCalls == 0, "kRing did not call free");

        resetMemoryCounters(0);
        H3_EXPORT(kRing)(pentagon, k, kRingOutput);
        t_assert(actualAllocCalls == 1, "kRing called alloc");
        t_assert(actualFreeCalls == 1, "kRing called free");

        resetMemoryCounters(0);
        failAlloc = true;
        memset(kRingOutput, 0, hexCount * sizeof(H3Index));
        H3_EXPORT(kRing)(pentagon, k, kRingOutput);
        t_assert(actualAllocCalls == 1, "kRing called alloc");
        t_assert(actualFreeCalls == 0, "kRing did not call free");

        for (int i = 0; i < hexCount; i++) {
            t_assert(!kRingOutput[i],
                     "kRing did not produce output without alloc");
        }

        free(kRingOutput);
    }

    TEST(compact) {
        int k = 9;
        int hexCount = H3_EXPORT(maxKringSize)(k);
        int expectedCompactCount = 73;

        // Generate a set of hexagons to compact
        H3Index* sunnyvaleExpanded = calloc(hexCount, sizeof(H3Index));
        resetMemoryCounters(0);
        H3_EXPORT(kRing)(sunnyvale, k, sunnyvaleExpanded);
        t_assert(actualAllocCalls == 0, "kRing did not call alloc");
        t_assert(actualFreeCalls == 0, "kRing did not call free");

        H3Index* compressed = calloc(hexCount, sizeof(H3Index));

        resetMemoryCounters(0);
        failAlloc = true;
        int err = H3_EXPORT(compact)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == COMPACT_ALLOC_FAILED, "malloc failed (1)");
        t_assert(actualAllocCalls == 1, "alloc called once");
        t_assert(actualFreeCalls == 0, "free not called");

        resetMemoryCounters(1);
        err = H3_EXPORT(compact)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == COMPACT_ALLOC_FAILED, "malloc failed (2)");
        t_assert(actualAllocCalls == 2, "alloc called twice");
        t_assert(actualFreeCalls == 1, "free called once");

        resetMemoryCounters(2);
        err = H3_EXPORT(compact)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == COMPACT_ALLOC_FAILED, "malloc failed (3)");
        t_assert(actualAllocCalls == 3, "alloc called three times");
        t_assert(actualFreeCalls == 2, "free called twice");

        resetMemoryCounters(3);
        err = H3_EXPORT(compact)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == COMPACT_ALLOC_FAILED, "compact failed (4)");
        t_assert(actualAllocCalls == 4, "alloc called four times");
        t_assert(actualFreeCalls == 3, "free called three times");

        resetMemoryCounters(4);
        err = H3_EXPORT(compact)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == COMPACT_SUCCESS, "compact using successful malloc");
        t_assert(actualAllocCalls == 4, "alloc called four times");
        t_assert(actualFreeCalls == 4, "free called four times");

        int count = 0;
        for (int i = 0; i < hexCount; i++) {
            if (compressed[i] != 0) {
                count++;
            }
        }
        t_assert(count == expectedCompactCount, "got expected compacted count");

        free(compressed);
        free(sunnyvaleExpanded);
    }
}
