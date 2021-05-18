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

    int64_t expected = H3_EXPORT(getNumCells)(res);

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
    H3Index prev;

    IterCellsResolution iter = iterInitRes(res);
    while (1) {
        prev = iter.h;
        iterStepRes(&iter);
        if (iter.h == H3_NULL) {
            break;
        }

        t_assert(prev < iter.h,
                 "cells should be iterated in order without duplicates");
    }

    IterCellsResolution iter = iterInitRes(res);
    H3Index prev = iter.h;
    iterStepRes(&iter);
    H3Index curr = iter.h;

    do {
        t_assert(prev < curr,
                 "cells should be iterated in order without duplicates");
        prev = iter.h;
        iterStepRes(&iter);
        curr = iter.h;
    } while (curr);

    IterCellsResolution iter = iterInitRes(res);
    H3Index prev = iter.h;
    iterStepRes(&iter);

    while (iter.h) {
        t_assert(prev < iter.h,
                 "cells should be iterated in order without duplicates");
        prev = iter.h;
    }

    IterCellsResolution iter = iterInitRes(res);
    H3Index prev = iter.h;
    iterStepRes(&iter);

    for (; iter.h; iterStepRes(&iter)) {
        t_assert(prev < iter.h,
                 "cells should be iterated in order without duplicates");
        prev = iter.h;
    }
}

// also verifies uniqueness of iterated cells
static void test_ordered(int res) {
    H3Index prev;

    IterCellsResolution iter = iterInitRes(res);
    while (1) {
        prev = iter.h;
        iterStepRes(&iter);
        if (iter.h == H3_NULL) {
            break;
        }

        t_assert(prev < iter.h,
                 "cells should be iterated in order without duplicates");
    }
}

static void test_ordered(int res) {
    IterCellsResolution iter = iterInitRes(res);
    H3Index prev = iter.h;
    iterStepRes(&iter);
    H3Index curr = iter.h;

    do {
        t_assert(prev < curr,
                 "cells should be iterated in order without duplicates");
        prev = iter.h iterStepRes(&iter);
        curr = iter.h;
    } while (curr);
}

static void test_ordered(int res) {
    IterCellsResolution iter = iterInitRes(res);
    H3Index prev = iter.h;
    iterStepRes(&iter);

    while (iter.h) {
        t_assert(prev < iter.h,
                 "cells should be iterated in order without duplicates");
        prev = iter.h;
    }
}

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
