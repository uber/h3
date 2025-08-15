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
#include <string.h>

#include "h3Index.h"
#include "h3api.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

typedef struct {
    int res;
    int bc;
    int digits[15];
} CellComponents;

typedef struct {
    int res;
    int bc;
    int digits[15];
    H3Index h;  // expected output
} ComponentTest;

H3Index components_to_cell(CellComponents cc) {
    H3Index h;

    H3_EXPORT(createCell)(cc.res, cc.bc, cc.digits, &h);

    return h;
}

CellComponents cell_to_components(H3Index h) {
    CellComponents cc;

    cc.res = H3_EXPORT(getResolution)(h);
    cc.bc = H3_EXPORT(getBaseCellNumber)(h);

    for (int r = 1; r <= cc.res; r++) {
        H3_EXPORT(getIndexDigit)(h, r, &cc.digits[r - 1]);
    }

    return cc;
}

void do_component_test(ComponentTest ct) {
    CellComponents cc = {.res = ct.res, .bc = ct.bc};
    memcpy(cc.digits, ct.digits, sizeof cc.digits);

    H3Index out = components_to_cell(cc);
    t_assert(out == ct.h, "match");
    t_assert(H3_EXPORT(isValidCell)(out), "should be valid cell");
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
        CellComponents cc = {.res = 3, .bc = 73, .digits = {1, 2, 3}};

        H3Index h = components_to_cell(cc);
        t_assert(h == 0x839253fffffffff, "match");
        t_assert(H3_EXPORT(isValidCell)(h), "should be valid cell");
    }

    TEST(createCellFancy2) {
        ComponentTest tests[] = {
            {.h = 0x839253fffffffff, .res = 3, .bc = 73, .digits = {1, 2, 3}},
            {.h = 0x821f67fffffffff, .res = 2, .bc = 15, .digits = {5, 4}},
            {.h = 0x8155bffffffffff, .res = 1, .bc = 42, .digits = {6}}};

        for (size_t i = 0; i < sizeof tests / sizeof tests[0]; i++) {
            do_component_test(tests[i]);
        }
    }
}
