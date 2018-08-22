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
/** @file test.h
 * @brief Test harness functions and macros.
 */

#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include "geoCoord.h"
#include "h3api.h"

extern int globalTestCount;

#define t_assert(condition, msg)                                            \
    do {                                                                    \
        if (!(condition)) {                                                 \
            fprintf(stderr, "t_assert failed at %s:%d, %s, %s\n", __FILE__, \
                    __LINE__, #condition, msg);                             \
            exit(1);                                                        \
        }                                                                   \
        globalTestCount++;                                                  \
        printf(".\n");                                                      \
    } while (0)

static inline void t_assertBoundary(H3Index h3, const GeoBoundary* b1) {
    // Generate cell boundary for the h3 index
    GeoBoundary b2;
    H3_EXPORT(h3ToGeoBoundary)(h3, &b2);
    t_assert(b1->numVerts == b2.numVerts, "expected cell boundary count");
    for (int v = 0; v < b1->numVerts; v++) {
        t_assert(geoAlmostEqual(&b1->verts[v], &b2.verts[v]),
                 "got expected vertex");
    }
}

#define SUITE(NAME)                                         \
    static void runTests(void);                             \
    int main(void) {                                        \
        printf("TEST %s\n", #NAME);                         \
        runTests();                                         \
        printf("\nDONE: %d assertions\n", globalTestCount); \
        return 0;                                           \
    }                                                       \
    void runTests(void)
#define TEST(NAME)

#endif
