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

typedef struct {
    uint64_t x;  // expected output, either valid H3 cell or error code
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

// ADD test to ensure error codes are never a valid cell. (some other test file?)
// ADD new error codes!

SUITE(createCell) {
    TEST(createCell) {
        H3Index h;

        // t_assertSuccess(H3_EXPORT(createCell)(0, 122, NULL, &h));
        // t_assert(h == 0x80f5fffffffffff, "match");
        // t_assert(!H3_EXPORT(isValidCell)(h), "should NOT be valid cell");
    }

    TEST(createCellNew) {
        MyTest tests[] = {
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

            // try some res domain errors
            {.x = E_RES_DOMAIN, .res = 16, .bc = 0, .digits = {}},
            {.x = E_RES_DOMAIN, .res = 18, .bc = 0, .digits = {}},

            // this is a mixture of base cell domain and child domain errors
            {.x = E_DOMAIN, .res = 0, .bc = 122, .digits = {}},
            {.x = E_DOMAIN, .res = 1, .bc = 40, .digits = {-1}},
            {.x = E_DOMAIN, .res = 1, .bc = 40, .digits = {7}},
            {.x = E_DOMAIN, .res = 1, .bc = 40, .digits = {8}},

            // deleted subsequence tests
            {.bc = 4, .digits = {0, 0, 0}, .res = 3, .x = 0x830800fffffffff},
            {.bc = 4, .digits = {0, 0, 1}, .res = 3, .x = E_CELL_INVALID},
            {.bc = 4, .digits = {0, 0, 2}, .res = 3, .x = 0x830802fffffffff},

            // obvious test to capture the "last comma" issue
            {.x = E_RES_DOMAIN, .res = -1, .bc = 0, .digits = {}}
            // ...
        };

        for (int i = 0; i < ARRAY_SIZE(tests); i++) {
            run_mytest(tests[i]);
        }
    }
}
