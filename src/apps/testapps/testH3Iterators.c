/*
 * Copyright 2021 Uber Technologies, Inc.
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
 * @brief tests the H3 cell iterators
 *
 *  usage: `testH3Iterators`
 */

#include "h3api.h"
#include "iterators.h"
#include "test.h"

static void test_number(int res) {
    int64_t count = 0;

    IterResCells iter = iterInitRes(res);

    for (; iter.h; iterStepRes(&iter)) {
        count++;
    }

    int64_t expected = H3_EXPORT(getNumCells)(res);

    t_assert(count == expected,
             "expect the correct number of cells from the iterator");
}

static void test_valid(int res) {
    IterResCells iter = iterInitRes(res);

    for (; iter.h; iterStepRes(&iter)) {
        t_assert(H3_EXPORT(isValidCell)(iter.h), "iterator cell is valid");
    }
}

SUITE(h3Iterators) {
    TEST(iterator_cell_count) {
        test_number(0);
        test_number(1);
        test_number(2);
        test_number(3);
    }

    TEST(iterator_cell_valid) {
        test_valid(0);
        test_valid(1);
        test_valid(2);
        test_valid(3);
    }
}
