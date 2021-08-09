/*
 * Copyright 2017-2021 Uber Technologies, Inc.
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

SUITE(cellToChildrenSize) {
    TEST(cellToChildrenSize_hexagon) {
        H3Index h = 0x87283080dffffff;  // res 7 *hexagon*

        int64_t sz;
        t_assert(H3_EXPORT(cellToChildrenSize)(h, 3, &sz) == E_RES_DOMAIN,
                 "got expected size for coarser res");
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 7, &sz));
        t_assert(sz == 1, "got expected size for same res");
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 8, &sz));
        t_assert(sz == 7, "got expected size for child res");
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 9, &sz));
        t_assert(sz == 7 * 7, "got expected size for grandchild res");
    }

    TEST(cellToChildrenSize_pentagon) {
        H3Index h = 0x870800000ffffff;  // res 7 *pentagon*

        int64_t sz;
        t_assert(H3_EXPORT(cellToChildrenSize)(h, 3, &sz) == E_RES_DOMAIN,
                 "got expected size for coarser res");
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 7, &sz));
        t_assert(sz == 1, "got expected size for same res");
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 8, &sz));
        t_assert(sz == 6, "got expected size for child res");
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 9, &sz));
        t_assert(sz == (5 * 7) + (1 * 6),
                 "got expected size for grandchild res");
    }

    TEST(cellToChildrenSize_largest_hexagon) {
        H3Index h = 0x806dfffffffffff;      // res 0 *hexagon*
        int64_t expected = 4747561509943L;  // 7^15
        int64_t out;
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 15, &out));

        t_assert(out == expected,
                 "got right size for children 15 levels below");
    }

    TEST(cellToChildrenSize_largest_pentagon) {
        H3Index h = 0x8009fffffffffff;      // res 0 *pentagon*
        int64_t expected = 3956301258286L;  // 1 + 5*(7^15 - 1)/6
        int64_t out;
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h, 15, &out));

        t_assert(out == expected,
                 "got right size for children 15 levels below");
    }
}
