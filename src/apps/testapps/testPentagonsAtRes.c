/*
 * Copyright 2017-2019 Uber Technologies, Inc.
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
#include "h3api.h"
#include "test.h"

#define PADDED_COUNT 16

SUITE(getPentagonsAtRes) {
    TEST(allResolutions) {
        int expectedCount = H3_EXPORT(pentagonIndexCount)();

        for (int res = 0; res <= 15; res++) {
            H3Index indexes[PADDED_COUNT] = {0};
            H3_EXPORT(getPentagonsAtRes)(res, indexes);

            int numFound = 0;

            for (int i = 0; i < PADDED_COUNT; i++) {
                H3Index h = indexes[i];
                if (H3_EXPORT(h3IsValid)(h)) {
                    t_assert(H3_EXPORT(h3IsPentagon(h)), "only pentagons");
                    t_assert(H3_EXPORT(h3GetResolution(h)) == res,
                             "only correct resolution");
                    numFound++;
                }
            }

            t_assert(numFound == expectedCount, "exactly 12 pentagons");
        }
    }
}
