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
#include "alloc.h"
#include "geoCoord.h"
#include "h3api.h"
#include "test.h"
#include "utility.h"

#ifndef H3_ALLOC_PREFIX
#error "This test can only be run with a prefix for alloc functions"
#endif

static int failAlloc = 0;

void* H3_MEMORY(malloc)(size_t size) {
    if (failAlloc) {
        return NULL;
    }
    return malloc(size);
}

void* H3_MEMORY(calloc)(size_t num, size_t size) {
    if (failAlloc) {
        return NULL;
    }
    return calloc(num, size);
}

void* H3_MEMORY(realloc)(void* ptr, size_t size) {
    if (failAlloc) {
        return NULL;
    }
    return realloc(ptr, size);
}

void H3_MEMORY(free)(void* ptr) { return free(ptr); }

H3Index sunnyvale = 0x89283470c27ffff;

SUITE(h3Memory) {
    TEST(t) {
        int k = 9;
        int hexCount = H3_EXPORT(maxKringSize)(k);

        // Generate a set of hexagons to compact
        H3Index* sunnyvaleExpanded = calloc(hexCount, sizeof(H3Index));
        H3_EXPORT(kRing)(sunnyvale, k, sunnyvaleExpanded);
    }
}
