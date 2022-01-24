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
    for (IterCellsResolution iter = iterInitRes(res); iter.h;
         iterStepRes(&iter)) {
        count++;
    }

    int64_t expected;
    t_assertSuccess(H3_EXPORT(getNumCells)(res, &expected));

    t_assert(count == expected,
             "expect the correct number of cells from the iterator");
}

static void test_valid(int res) {
    for (IterCellsResolution iter = iterInitRes(res); iter.h;
         iterStepRes(&iter)) {
        t_assert(H3_EXPORT(isValidCell)(iter.h), "iterator cell is valid");
    }
}

static void test_resolution(int res) {
    for (IterCellsResolution iter = iterInitRes(res); iter.h;
         iterStepRes(&iter)) {
        t_assert(H3_EXPORT(getResolution)(iter.h) == res,
                 "iterator cell is the correct resolution");
    }
}

// also verifies uniqueness of iterated cells
static void test_ordered(int res) {
    IterCellsResolution iter = iterInitRes(res);
    H3Index prev = iter.h;
    iterStepRes(&iter);

    for (; iter.h; iterStepRes(&iter)) {
        t_assert(prev < iter.h,
                 "cells should be iterated in order without duplicates");
        prev = iter.h;
    }
}

static void assert_is_null_iterator(IterCellsChildren iter) {
    t_assert(iter.h == H3_NULL, "null iterator cell is H3_NULL");
    t_assert(iter._parentRes == -1, "null iterator parentRes is -1");
    t_assert(iter._skipDigit == -1, "null iterator skipDigit is -1");
}

SUITE(h3Iterators) {
    TEST(iterator_setup_invalid) {
        assert_is_null_iterator(iterInitBaseCellNum(-1, 0));
        assert_is_null_iterator(iterInitBaseCellNum(1000, 0));
        assert_is_null_iterator(iterInitBaseCellNum(0, -1));
        assert_is_null_iterator(iterInitBaseCellNum(0, 100));

        assert_is_null_iterator(iterInitParent(0, 0));
        H3Index test = 0x85283473fffffff;
        assert_is_null_iterator(iterInitParent(test, 0));
        assert_is_null_iterator(iterInitParent(test, 100));
    }

    TEST(null_iterator_base_cell) {
        IterCellsChildren iter = iterInitBaseCellNum(-1, 0);
        assert_is_null_iterator(iter);
        iterStepChild(&iter);
        t_assert(iter.h == H3_NULL, "null iterator returns null");
    }

    TEST(null_iterator_res) {
        IterCellsResolution iter = iterInitRes(-1);
        assert_is_null_iterator(iter._itC);
        iterStepRes(&iter);
        t_assert(iter.h == H3_NULL, "null iterator returns null");
    }

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

    TEST(iterator_cell_resolution) {
        test_resolution(0);
        test_resolution(1);
        test_resolution(2);
        test_resolution(3);
    }

    TEST(iterator_cell_ordered) {
        test_ordered(0);
        test_ordered(1);
        test_ordered(2);
        test_ordered(3);
    }
}
