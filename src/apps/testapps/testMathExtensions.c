/*
 * Copyright 2022 Uber Technologies, Inc.
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
 * @brief tests functions and macros in mathExtensions.h
 *
 *  usage: `testMathExtensions`
 */

#include "mathExtensions.h"
#include "test.h"
#include "utility.h"

#define ASSERT_SUB_OVERFLOW(a, b) \
    t_assert(SUB_INT32S_OVERFLOWS((a), (b)), #a " - " #b " Overflows")
#define ASSERT_SUB_DOES_NOT_OVERFLOW(a, b)                    \
    t_assert(!SUB_INT32S_OVERFLOWS((a), (b)), #a " - " #b     \
                                                 " Does Not " \
                                                 "Overflow")
#define ASSERT_ADD_OVERFLOW(a, b) \
    t_assert(ADD_INT32S_OVERFLOWS((a), (b)), #a " + " #b " Overflows")
#define ASSERT_ADD_DOES_NOT_OVERFLOW(a, b)                    \
    t_assert(!ADD_INT32S_OVERFLOWS((a), (b)), #a " + " #b     \
                                                 " Does Not " \
                                                 "Overflow")

SUITE(mathExtensions) {
    TEST(_ipow) {
        t_assert(_ipow(7, 0) == 1, "7 ** 0 == 1");
        t_assert(_ipow(7, 1) == 7, "7 ** 1 == 7");
        t_assert(_ipow(7, 2) == 49, "7 ** 2 == 49");
        t_assert(_ipow(1, 20) == 1, "1 ** 20 == 1");
        t_assert(_ipow(2, 5) == 32, "2 ** 5 == 32");
    }

    TEST(subOverflows) {
        ASSERT_SUB_DOES_NOT_OVERFLOW(0, 0);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MIN, 0);
        ASSERT_SUB_OVERFLOW(INT32_MIN, 1);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MIN, -1);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MIN + 1, 0);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MIN + 1, 1);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MIN + 1, -1);
        ASSERT_SUB_OVERFLOW(INT32_MIN + 1, 2);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MIN + 1, -2);
        ASSERT_SUB_DOES_NOT_OVERFLOW(100, 10);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MAX, 0);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MAX, 1);
        ASSERT_SUB_OVERFLOW(INT32_MAX, -1);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MAX - 1, 1);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MAX - 1, -1);
        ASSERT_SUB_OVERFLOW(INT32_MAX - 1, -2);
        ASSERT_SUB_OVERFLOW(INT32_MAX - 1, -2);
        ASSERT_SUB_OVERFLOW(INT32_MIN, INT32_MAX);
        ASSERT_SUB_OVERFLOW(INT32_MAX, INT32_MIN);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MIN, INT32_MIN);
        ASSERT_SUB_DOES_NOT_OVERFLOW(INT32_MAX, INT32_MAX);
        ASSERT_SUB_DOES_NOT_OVERFLOW(-1, 0);
        ASSERT_SUB_DOES_NOT_OVERFLOW(-1, 10);
        ASSERT_SUB_DOES_NOT_OVERFLOW(-1, -10);
        ASSERT_SUB_DOES_NOT_OVERFLOW(-1, INT32_MAX);
        ASSERT_SUB_OVERFLOW(-2, INT32_MAX);
        ASSERT_SUB_DOES_NOT_OVERFLOW(-1, INT32_MIN);
        ASSERT_SUB_OVERFLOW(0, INT32_MIN);
    }

    TEST(addOverflows) {
        ASSERT_ADD_DOES_NOT_OVERFLOW(0, 0);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MIN, 0);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MIN, 1);
        ASSERT_ADD_OVERFLOW(INT32_MIN, -1);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MIN + 1, 0);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MIN + 1, 1);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MIN + 1, -1);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MIN + 1, 2);
        ASSERT_ADD_OVERFLOW(INT32_MIN + 1, -2);
        ASSERT_ADD_DOES_NOT_OVERFLOW(100, 10);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MAX, 0);
        ASSERT_ADD_OVERFLOW(INT32_MAX, 1);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MAX, -1);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MAX - 1, 1);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MAX - 1, -1);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MAX - 1, -2);
        ASSERT_ADD_OVERFLOW(INT32_MAX - 1, 2);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MIN, INT32_MAX);
        ASSERT_ADD_DOES_NOT_OVERFLOW(INT32_MAX, INT32_MIN);
        ASSERT_ADD_OVERFLOW(INT32_MAX, INT32_MAX);
        ASSERT_ADD_OVERFLOW(INT32_MIN, INT32_MIN);
        ASSERT_ADD_DOES_NOT_OVERFLOW(-1, 0);
        ASSERT_ADD_DOES_NOT_OVERFLOW(-1, 10);
        ASSERT_ADD_DOES_NOT_OVERFLOW(-1, -10);
        ASSERT_ADD_DOES_NOT_OVERFLOW(-1, INT32_MAX);
        ASSERT_ADD_DOES_NOT_OVERFLOW(-2, INT32_MAX);
        ASSERT_ADD_OVERFLOW(-1, INT32_MIN);
        ASSERT_ADD_DOES_NOT_OVERFLOW(0, INT32_MIN);
    }
}