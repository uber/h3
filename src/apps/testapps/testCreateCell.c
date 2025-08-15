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

#include "h3Index.h"
#include "h3api.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

typedef struct {
    int res;
    int digits[16];
} CellComponents;

H3Index components_to_cell(CellComponents cc) {
    H3Index h;

    H3_EXPORT(createCell)(cc.res, cc.digits[0], &cc.digits[1], &h);

    return h;
}

CellComponents cell_to_components(H3Index h) {
    CellComponents cc;

    cc.res = H3_EXPORT(getResolution)(h);
    cc.digits[0] = H3_EXPORT(getBaseCellNumber)(h);

    for (int r = 1; r <= MAX_H3_RES; r++) {
        if (r <= cc.res) {
            H3_EXPORT(getIndexDigit)(h, r, &cc.digits[r]);
        } else {
            cc.digits[r] = 7;
        }
    }

    return cc;
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

        int digits[] = {1, 2, 3};  // classic array literal at definition time
        int res = 3;
        int bc = 170;
        t_assertSuccess(H3_EXPORT(createCell)(res, bc, digits, &h));
    }

    TEST(createCell2) {
        H3Index h;

        int res = 3;
        int bc = 170;
        int digits[] = {1, 2, 3};
        t_assertSuccess(H3_EXPORT(createCell)(res, bc, digits, &h));
    }

    TEST(createCellFancy) {
        H3Index h;

        // TODO
    }
}
