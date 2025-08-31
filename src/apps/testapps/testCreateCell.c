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
#include "iterators.h"
#include "test.h"
#include "utility.h"

typedef struct {
    uint64_t x;  // expected output: either valid H3 cell or error code
    int res;
    int bc;
    int digits[15];
} MyTest;

void run_mytest(MyTest mt) {
    H3Index h;
    H3Error err = H3_EXPORT(createCell)(mt.res, mt.bc, mt.digits, &h);

    if (H3_EXPORT(isValidCell)(mt.x)) {
        t_assertSuccess(err);
        t_assert(mt.x == h, "Got the expected cell.");
    } else {
        t_assert(mt.x == err, "Got the expected error code.");
    }
}

bool passes_roundtrip(const H3Index h) {
    // test roundtrip: h3index -> components -> h3index
    int res = H3_EXPORT(getResolution)(h);
    int bc = H3_EXPORT(getBaseCellNumber)(h);
    int digits[15];

    for (int r = 1; r <= res; r++) {
        if (H3_EXPORT(getIndexDigit)(h, r, &digits[r - 1]) != E_SUCCESS) {
            return false;
        }
    }

    H3Index out;
    if (H3_EXPORT(createCell)(res, bc, digits, &out) != E_SUCCESS) {
        return false;
    }

    return out == h;
}

static void res_roundtrip(int res) {
    bool all_passed = true;
    IterCellsResolution iter = iterInitRes(res);

    while (iter.h) {
        if (!passes_roundtrip(iter.h)) {
            all_passed = false;
        }
        iterStepRes(&iter);
    }

    t_assert(all_passed, "All cells at this res passed the roundtrip");
}

// ADD test to ensure error codes are never a valid cell. (some other test
// file?)

SUITE(createCell) {
    TEST(tableOfTests) {
        static const MyTest tests[] = {
            {.x = 0x8001fffffffffff, .res = 0, .bc = 0, .digits = {}},
            {.x = 0x8003fffffffffff, .res = 0, .bc = 1, .digits = {}},
            {.x = 0x80f3fffffffffff, .res = 0, .bc = 121, .digits = {}},
            {.x = 0x839253fffffffff, .res = 3, .bc = 73, .digits = {1, 2, 3}},
            {.x = 0x821f67fffffffff, .res = 2, .bc = 15, .digits = {5, 4}},
            {.x = 0x8155bffffffffff, .res = 1, .bc = 42, .digits = {6}},
            {.x = 0x8f754e64992d6d8,
             .res = 15,
             .bc = 58,
             .digits = {5, 1, 6, 3, 1, 1, 1, 4, 4, 5, 5, 3, 3, 3, 0}},

            {.res = 16, .bc = 0, .digits = {}, .x = E_RES_DOMAIN},
            {.res = 18, .bc = 0, .digits = {}, .x = E_RES_DOMAIN},
            {.res = -1, .bc = 0, .digits = {}, .x = E_RES_DOMAIN},
            {.res = 0, .bc = 0, .digits = {}, .x = 0x8001fffffffffff},

            {.res = 0, .bc = 122, .digits = {}, .x = E_BASE_CELL_DOMAIN},
            {.res = 0, .bc = -1, .digits = {}, .x = E_BASE_CELL_DOMAIN},
            {.res = 0, .bc = 259, .digits = {}, .x = E_BASE_CELL_DOMAIN},
            {.res = 2, .bc = 122, .digits = {1, 0}, .x = E_BASE_CELL_DOMAIN},

            {.res = 1, .bc = 40, .digits = {-1}, .x = E_DIGIT_DOMAIN},
            {.res = 1, .bc = 40, .digits = {7}, .x = E_DIGIT_DOMAIN},
            {.res = 1, .bc = 40, .digits = {8}, .x = E_DIGIT_DOMAIN},
            {.res = 1, .bc = 40, .digits = {17}, .x = E_DIGIT_DOMAIN},

            // deleted subsequence tests
            // bc = 4 is a pentagon base cell
            {.bc = 4, .digits = {0, 0, 0}, .res = 3, .x = 0x830800fffffffff},
            {.bc = 4, .digits = {0, 0, 1}, .res = 3, .x = E_DELETED_DIGIT},
            {.bc = 4, .digits = {0, 0, 2}, .res = 3, .x = 0x830802fffffffff},

            // bc = 5 is *not* a pentagon base cell
            {.bc = 5, .digits = {0, 0, 0}, .res = 3, .x = 0x830a00fffffffff},
            {.bc = 5, .digits = {0, 0, 1}, .res = 3, .x = 0x830a01fffffffff},
            {.bc = 5, .digits = {0, 0, 2}, .res = 3, .x = 0x830a02fffffffff},

            {.x = E_RES_DOMAIN, .res = -1}  // avoid trailing comma
        };

        for (size_t i = 0; i < ARRAY_SIZE(tests); i++) {
            run_mytest(tests[i]);
        }
    }

    TEST(roundtrip) {
        res_roundtrip(0);
        res_roundtrip(1);
        res_roundtrip(2);
    }
}
