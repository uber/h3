/*
 * Copyright 2019-2020 Uber Technologies, Inc.
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

SUITE(getPentagons) {
    TEST(propertyTests) {
        int expectedCount = H3_EXPORT(pentagonCount)();

        for (int res = 0; res <= 15; res++) {
            H3Index h3Indexes[PADDED_COUNT] = {0};
            t_assertSuccess(H3_EXPORT(getPentagons)(res, h3Indexes));

            int numFound = 0;

            for (int i = 0; i < PADDED_COUNT; i++) {
                H3Index h3Index = h3Indexes[i];
                if (h3Index) {
                    numFound++;
                    t_assert(H3_EXPORT(isValidCell(h3Index)),
                             "index should be valid");
                    t_assert(H3_EXPORT(isPentagon(h3Index)),
                             "index should be pentagon");
                    t_assert(H3_EXPORT(getResolution(h3Index)) == res,
                             "index should have correct resolution");

                    // verify uniqueness
                    for (int j = i + 1; j < PADDED_COUNT; j++) {
                        if (h3Indexes[j] == h3Index) {
                            t_assert(false, "index should be seen only once");
                        }
                    }
                }
            }

            t_assert(numFound == expectedCount,
                     "there should be exactly 12 pentagons");
        }
    }

    TEST(getPentagonsInvalid) {
        H3Index h3Indexes[PADDED_COUNT] = {0};
        t_assert(H3_EXPORT(getPentagons)(16, h3Indexes) == E_RES_DOMAIN,
                 "getPentagons of invalid resolutions fails");
        t_assert(H3_EXPORT(getPentagons)(100, h3Indexes) == E_RES_DOMAIN,
                 "getPentagons of invalid resolutions fails");
        t_assert(H3_EXPORT(getPentagons)(-1, h3Indexes) == E_RES_DOMAIN,
                 "getPentagons of invalid resolutions fails");
    }

    TEST(invalidPentagons) {
        t_assert(!H3_EXPORT(isPentagon)(0), "0 is not a pentagon");
        t_assert(!H3_EXPORT(isPentagon)(0x7fffffffffffffff),
                 "all but high bit is not a pentagon");
    }
}
