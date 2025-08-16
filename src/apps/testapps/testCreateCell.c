/*
 * Copyright 2025 Uber Technologies, Inc.
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
 * @brief tests index digits get/set functions
 *
 *  usage: `testCreateCell`
 */

#include <math.h>

#include "h3Index.h"  // can i get rid of this and just pull in the API
#include "h3api.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

typedef struct {
    H3Index h;
    int res;
    int bc;
    int digits[15];
} CellAndComponents;

H3Index components_to_cell(CellAndComponents cnc) {
    H3Index h;
    H3_EXPORT(createCell)(cnc.res, cnc.bc, cnc.digits, &h);
    return h;
}

CellAndComponents cell_to_components(H3Index h) {
    CellAndComponents cnc = {.h = h,
                             .res = H3_EXPORT(getResolution)(h),
                             .bc = H3_EXPORT(getBaseCellNumber)(h)};

    for (int r = 1; r <= cnc.res; r++) {
        H3_EXPORT(getIndexDigit)(h, r, &cnc.digits[r - 1]);
    }

    return cnc;
}

void validate_cnc(CellAndComponents a) {
    H3Index h = components_to_cell(a);
    t_assert(h == a.h, "match");
    t_assert(H3_EXPORT(isValidCell)(h), "should be valid cell");

    CellAndComponents b = cell_to_components(a.h);

    t_assert(a.h == b.h, "bah");
    t_assert(a.res == b.res, "bah");
    t_assert(a.bc == b.bc, "bah");

    for (int r = 1; r <= a.res; r++) {
        t_assert(a.digits[r - 1] == b.digits[r - 1], "bah");
    }
}

// TODO: error on bad res
// TODO: error on bad base cell

SUITE(createCell) {
    TEST(createCell) {
        H3Index h;

        t_assertSuccess(H3_EXPORT(createCell)(0, 0, NULL, &h));
        t_assert(h == 0x8001fffffffffff, "match");
        t_assert(H3_EXPORT(isValidCell)(h), "should be valid cell");

        t_assertSuccess(H3_EXPORT(createCell)(0, 1, NULL, &h));
        t_assert(h == 0x8003fffffffffff, "match");
        t_assert(H3_EXPORT(isValidCell)(h), "should be valid cell");

        t_assertSuccess(H3_EXPORT(createCell)(0, 121, NULL, &h));
        t_assert(h == 0x80f3fffffffffff, "match");
        t_assert(H3_EXPORT(isValidCell)(h), "should be valid cell");

        t_assertSuccess(H3_EXPORT(createCell)(0, 122, NULL, &h));
        t_assert(h == 0x80f5fffffffffff, "match");
        t_assert(!H3_EXPORT(isValidCell)(h), "should not be valid cell");
    }

    TEST(createCell2) {
        H3Index h;

        int res = 3;
        int bc = 73;
        int digits[] = {1, 2, 3};
        t_assertSuccess(H3_EXPORT(createCell)(res, bc, digits, &h));

        t_assert(h == 0x839253fffffffff, "match");
        t_assert(H3_EXPORT(isValidCell)(h), "should be valid cell");
    }

    TEST(createCellFancy) {
        CellAndComponents tests[] = {
            {.h = 0x8001fffffffffff, .res = 0, .bc = 0, .digits = {}},
            {.h = 0x8003fffffffffff, .res = 0, .bc = 1, .digits = {}},
            {.h = 0x80f3fffffffffff, .res = 0, .bc = 121, .digits = {}},
            // {.h = , .res = 0, .bc = 122, .digits = {}},
            {.h = 0x839253fffffffff, .res = 3, .bc = 73, .digits = {1, 2, 3}},
            {.h = 0x821f67fffffffff, .res = 2, .bc = 15, .digits = {5, 4}},
            {.h = 0x8155bffffffffff, .res = 1, .bc = 42, .digits = {6}}};

        for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
            validate_cnc(tests[i]);
        }
        // TODO: can have some examples expect a certain error. then check for
        // those!

        //     ComponentTest tests[] = {
        //         {.h = 0, .res = 3, .bc = 73, .digits = {1, 2, 3}},
        //         {.h = 0x821f67fffffffff, .res = 2, .bc = 15, .digits = {5,
        //         4}},
        //         {.h = 0x8155bffffffffff, .res = 1, .bc = 42, .digits = {6}}};
        // }
    }
}
