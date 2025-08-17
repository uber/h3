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
 * @brief tests function to create cell from components
 *
 *  usage: `testCreateCell`
 */

#include "h3api.h"
#include "test.h"
#include "utility.h"

// Helper struct to represent validation tests
typedef struct {
    H3Index h;
    int res;
    int bc;
    int digits[15];
} CellAndComponents;

typedef struct {
    H3ErrorCodes err;
    int res;
    int bc;
    int digits[15];
} ErrorAndComponents;

H3Index components_to_cell(CellAndComponents cnc) {
    H3Index h;
    t_assertSuccess(H3_EXPORT(createCell)(cnc.res, cnc.bc, cnc.digits, &h));
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

// Validate components_to_cell and cell_to_components work based on given test
// data
void validate_cnc(CellAndComponents a) {
    H3Index h = components_to_cell(a);
    t_assert(h == a.h, "Index matches");
    t_assert(H3_EXPORT(isValidCell)(h), "Should be valid cell");

    CellAndComponents b = cell_to_components(a.h);

    t_assert(a.h == b.h, "Index matches");
    t_assert(a.res == b.res, "Resolution matches");
    t_assert(a.bc == b.bc, "Base cell number matches");

    for (int r = 1; r <= a.res; r++) {
        t_assert(a.digits[r - 1] == b.digits[r - 1], "Digit matches");
    }
}

void expect_error(ErrorAndComponents a) {
    H3Index h;
    H3Error err = H3_EXPORT(createCell)(a.res, a.bc, a.digits, &h);

    t_assert(err == a.err, "Expecting an error");
}

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

        // t_assertSuccess(H3_EXPORT(createCell)(0, 122, NULL, &h));
        // t_assert(h == 0x80f5fffffffffff, "match");
        // t_assert(!H3_EXPORT(isValidCell)(h), "should NOT be valid cell");
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
            {.h = 0x839253fffffffff, .res = 3, .bc = 73, .digits = {1, 2, 3}},
            {.h = 0x821f67fffffffff, .res = 2, .bc = 15, .digits = {5, 4}},
            {.h = 0x8155bffffffffff, .res = 1, .bc = 42, .digits = {6}},
            {.h = 0x8f754e64992d6d8,
             .res = 15,
             .bc = 58,
             .digits = {5, 1, 6, 3, 1, 1, 1, 4, 4, 5, 5, 3, 3, 3, 0}}};

        for (int i = 0; i < ARRAY_SIZE(tests); i++) {
            validate_cnc(tests[i]);
        }
    }

    TEST(createCellErrors) {
        ErrorAndComponents tests[] = {
            {.err = E_RES_DOMAIN, .res = 16, .bc = 0, .digits = {}},
            {.err = E_DOMAIN, .res = 0, .bc = 122, .digits = {}},
            {.err = E_DOMAIN, .res = 1, .bc = 40, .digits = {-1}},
            {.err = E_DOMAIN, .res = 1, .bc = 40, .digits = {7}}};

        for (int i = 0; i < ARRAY_SIZE(tests); i++) {
            expect_error(tests[i]);
        }
    }

    TEST(sneakyInvalidCell) {
        // Create cell with a "deleted subsequence".
        // This is the trickiest case to detect of an invalid cell.

        H3Index h;
        int res = 3;
        int bc = 4;
        int digits[] = {0, 0, 1};
        t_assertSuccess(H3_EXPORT(createCell)(res, bc, digits, &h));

        t_assert(h == 0x830801fffffffff, "match");
        t_assert(!H3_EXPORT(isValidCell)(h), "should NOT be a valid cell");
    }
}
