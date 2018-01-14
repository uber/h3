/*
 * Copyright 2017 Uber Technologies, Inc.
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
 * @brief tests H3 functions for manipulating H3 indexes
 *
 *  usage: `testH3IndexFat`
 */

#include "h3Index.h"
#include "h3IndexFat.h"
#include "test.h"
#include "utility.h"

BEGIN_TESTS(h3IndexFat);

TEST(h3FatIsValidWithMode) {
    for (int i = 0; i <= 0xf; i++) {
        H3IndexFat hf;
        setH3IndexFat(&hf, 0, 0, 0);
        hf.mode = i;
        char failureMessage[BUFF_SIZE];
        sprintf(failureMessage, "h3FatIsValid failed on mode %d", i);
        t_assert(!h3FatIsValid(&hf) || i == 1, failureMessage);
    }
}

TEST(h3FatNegativeResInvalid) {
    H3IndexFat hfNegativeRes;
    setH3IndexFat(&hfNegativeRes, -1, 0, 0);
    t_assert(!h3FatIsValid(&hfNegativeRes),
             "h3FatIsValid failed on negative resolution");
}

TEST(h3FatLargeResolutionInvalid) {
    H3IndexFat hfLargeRes;
    setH3IndexFat(&hfLargeRes, 15, 0, 0);
    // Can't set resolution to 16 using setH3IndexFat
    // since it would corrupt the stack.
    hfLargeRes.res = 16;
    t_assert(!h3FatIsValid(&hfLargeRes),
             "h3FatIsValid failed on large resolution");
}

TEST(h3FatUnusedDigitInvalid) {
    H3IndexFat hf;
    initH3IndexFat(&hf, 0);
    hf.baseCell = 1;
    hf.index[0] = 0;
    t_assert(!h3FatIsValid(&hf),
             "h3FatIsValid failed on invalid index digit (0)");

    hf.index[0] = 7;
    hf.index[14] = 2;
    t_assert(!h3FatIsValid(&hf),
             "h3FatIsValid failed on invalid index digit (14)");

    hf.res = 1;
    hf.index[14] = 7;
    hf.index[1] = 1;
    t_assert(!h3FatIsValid(&hf),
             "h3FatIsValid failed on invalid index digit (1)");
}

TEST(h3FatBadDigitInvalid) {
    H3IndexFat hfNegative;
    setH3IndexFat(&hfNegative, 1, 0, -1);
    t_assert(!h3FatIsValid(&hfNegative),
             "h3FatIsValid failed on negative digit");

    H3IndexFat hfLarge;
    setH3IndexFat(&hfLarge, 1, 0, 7);
    t_assert(!h3FatIsValid(&hfLarge), "h3FatIsValid failed on too large digit");
}

TEST(h3FatIsValidBaseCell) {
    for (int i = 0; i < NUM_BASE_CELLS; i++) {
        H3IndexFat hf;
        initH3IndexFat(&hf, 0);
        hf.mode = 1;
        hf.baseCell = i;
        char failureMessage[BUFF_SIZE];
        sprintf(failureMessage, "h3FatIsValid failed on base cell %d", i);
        t_assert(h3FatIsValid(&hf), failureMessage);

        H3Index h = h3FatToH3(&hf);
        t_assert(H3_EXPORT(h3GetBaseCell)(h) == i,
                 "failed to recover base cell");
    }
}

TEST(h3FatIsValidNegativeBaseCellInvalid) {
    H3IndexFat hfNegativeBaseCell;
    initH3IndexFat(&hfNegativeBaseCell, 0);
    hfNegativeBaseCell.baseCell = -1;
    t_assert(!h3FatIsValid(&hfNegativeBaseCell),
             "h3FatIsValid failed on negative base cell");
}

TEST(h3FatIsValidBaseCellInvalud) {
    H3IndexFat hfWrongBaseCell;
    initH3IndexFat(&hfWrongBaseCell, 0);
    hfWrongBaseCell.baseCell = NUM_BASE_CELLS;
    t_assert(!h3FatIsValid(&hfWrongBaseCell),
             "h3FatIsValid failed on invalid base cell");
}

TEST(setH3IndexFat) {
    H3IndexFat hf;
    setH3IndexFat(&hf, 5, 12, 1);
    t_assert(hf.res == 5, "resolution as expected");
    t_assert(hf.baseCell == 12, "base cell as expected");
    t_assert(hf.mode == H3_HEXAGON_MODE, "mode as expected");
    for (int i = 0; i < 5; i++) {
        t_assert(hf.index[i] == 1, "digit as expected");
    }
    for (int i = 5; i < MAX_H3_RES; i++) {
        t_assert(hf.index[i] == 7, "blanked digit as expected");
    }

    H3IndexFat comparison = {
        H3_HEXAGON_MODE, 5, 12, {1, 1, 1, 1, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7}};
    t_assert(h3FatEquals(&hf, &comparison), "equals expected value");

    H3IndexFat wrongComparison = {
        H3_HEXAGON_MODE, 5, 11, {1, 1, 1, 1, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7}};
    t_assert(!h3FatEquals(&hf, &wrongComparison),
             "equals wrong value (base cell)");

    H3IndexFat wrongComparison2 = {
        H3_HEXAGON_MODE, 5, 12, {1, 1, 1, 2, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7}};
    t_assert(!h3FatEquals(&hf, &wrongComparison2),
             "equals wrong value (digit)");
}

END_TESTS();
