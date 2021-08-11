/*
 * Copyright 2017-2018, 2020-2021 Uber Technologies, Inc.
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
 *  usage: `testH3Index`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

SUITE(h3Index) {
    TEST(latLngToCellExtremeCoordinates) {
        H3Index h;
        // Check that none of these cause crashes.
        LatLng g = {0, 1E45};
        t_assertSuccess(H3_EXPORT(latLngToCell)(&g, 14, &h));

        LatLng g2 = {1E46, 1E45};
        t_assertSuccess(H3_EXPORT(latLngToCell)(&g2, 15, &h));

        LatLng g4;
        setGeoDegs(&g4, 2, -3E39);
        t_assertSuccess(H3_EXPORT(latLngToCell)(&g4, 0, &h));
    }

    TEST(faceIjkToH3ExtremeCoordinates) {
        FaceIJK fijk0I = {0, {3, 0, 0}};
        t_assert(_faceIjkToH3(&fijk0I, 0) == 0, "i out of bounds at res 0");
        FaceIJK fijk0J = {1, {0, 4, 0}};
        t_assert(_faceIjkToH3(&fijk0J, 0) == 0, "j out of bounds at res 0");
        FaceIJK fijk0K = {2, {2, 0, 5}};
        t_assert(_faceIjkToH3(&fijk0K, 0) == 0, "k out of bounds at res 0");

        FaceIJK fijk1I = {3, {6, 0, 0}};
        t_assert(_faceIjkToH3(&fijk1I, 1) == 0, "i out of bounds at res 1");
        FaceIJK fijk1J = {4, {0, 7, 1}};
        t_assert(_faceIjkToH3(&fijk1J, 1) == 0, "j out of bounds at res 1");
        FaceIJK fijk1K = {5, {2, 0, 8}};
        t_assert(_faceIjkToH3(&fijk1K, 1) == 0, "k out of bounds at res 1");

        FaceIJK fijk2I = {6, {18, 0, 0}};
        t_assert(_faceIjkToH3(&fijk2I, 2) == 0, "i out of bounds at res 2");
        FaceIJK fijk2J = {7, {0, 19, 1}};
        t_assert(_faceIjkToH3(&fijk2J, 2) == 0, "j out of bounds at res 2");
        FaceIJK fijk2K = {8, {2, 0, 20}};
        t_assert(_faceIjkToH3(&fijk2K, 2) == 0, "k out of bounds at res 2");
    }

    TEST(isValidCellAtResolution) {
        for (int i = 0; i <= MAX_H3_RES; i++) {
            LatLng g = {0, 0};
            H3Index h3;
            t_assertSuccess(H3_EXPORT(latLngToCell)(&g, i, &h3));
            char failureMessage[BUFF_SIZE];
            sprintf(failureMessage, "isValidCell failed on resolution %d", i);
            t_assert(H3_EXPORT(isValidCell)(h3), failureMessage);
        }
    }

    TEST(isValidCellDigits) {
        LatLng g = {0, 0};
        H3Index h3;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&g, 1, &h3));
        // Set a bit for an unused digit to something else.
        h3 ^= 1;
        t_assert(!H3_EXPORT(isValidCell)(h3),
                 "isValidCell failed on invalid unused digits");
    }

    TEST(isValidCellBaseCell) {
        for (int i = 0; i < NUM_BASE_CELLS; i++) {
            H3Index h = H3_INIT;
            H3_SET_MODE(h, H3_CELL_MODE);
            H3_SET_BASE_CELL(h, i);
            char failureMessage[BUFF_SIZE];
            sprintf(failureMessage, "isValidCell failed on base cell %d", i);
            t_assert(H3_EXPORT(isValidCell)(h), failureMessage);

            t_assert(H3_EXPORT(getBaseCellNumber)(h) == i,
                     "failed to recover base cell");
        }
    }

    TEST(isValidCellBaseCellInvalid) {
        H3Index hWrongBaseCell = H3_INIT;
        H3_SET_MODE(hWrongBaseCell, H3_CELL_MODE);
        H3_SET_BASE_CELL(hWrongBaseCell, NUM_BASE_CELLS);
        t_assert(!H3_EXPORT(isValidCell)(hWrongBaseCell),
                 "isValidCell failed on invalid base cell");
    }

    TEST(isValidCellWithMode) {
        for (int i = 0; i <= 0xf; i++) {
            H3Index h = H3_INIT;
            H3_SET_MODE(h, i);
            if (i == H3_CELL_MODE) {
                t_assert(H3_EXPORT(isValidCell)(h),
                         "isValidCell succeeds on valid mode");
            } else {
                char failureMessage[BUFF_SIZE];
                sprintf(failureMessage, "isValidCell failed on mode %d", i);
                t_assert(!H3_EXPORT(isValidCell)(h), failureMessage);
            }
        }
    }

    TEST(isValidCellReservedBits) {
        for (int i = 0; i < 8; i++) {
            H3Index h = H3_INIT;
            H3_SET_MODE(h, H3_CELL_MODE);
            H3_SET_RESERVED_BITS(h, i);
            if (i == 0) {
                t_assert(H3_EXPORT(isValidCell)(h),
                         "isValidCell succeeds on valid reserved bits");
            } else {
                char failureMessage[BUFF_SIZE];
                sprintf(failureMessage,
                        "isValidCell failed on reserved bits %d", i);
                t_assert(!H3_EXPORT(isValidCell)(h), failureMessage);
            }
        }
    }

    TEST(isValidCellHighBit) {
        H3Index h = H3_INIT;
        H3_SET_MODE(h, H3_CELL_MODE);
        H3_SET_HIGH_BIT(h, 1);
        t_assert(!H3_EXPORT(isValidCell)(h), "isValidCell failed on high bit");
    }

    TEST(h3BadDigitInvalid) {
        H3Index h = H3_INIT;
        // By default the first index digit is out of range.
        H3_SET_MODE(h, H3_CELL_MODE);
        H3_SET_RESOLUTION(h, 1);
        t_assert(!H3_EXPORT(isValidCell)(h),
                 "isValidCell failed on too large digit");
    }

    TEST(h3DeletedSubsequenceInvalid) {
        H3Index h;
        // Create an index located in a deleted subsequence of a pentagon.
        setH3Index(&h, 1, 4, K_AXES_DIGIT);
        t_assert(!H3_EXPORT(isValidCell)(h),
                 "isValidCell failed on deleted subsequence");
    }

    TEST(h3ToString) {
        const size_t bufSz = 17;
        char buf[17] = {0};
        t_assert(
            H3_EXPORT(h3ToString)(0x1234, buf, bufSz - 1) == E_MEMORY_BOUNDS,
            "h3ToString failed on buffer too small");
        t_assertSuccess(H3_EXPORT(h3ToString)(0xcafe, buf, bufSz));
        t_assert(strcmp(buf, "cafe") == 0,
                 "h3ToString failed to produce base 16 results");
        t_assertSuccess(H3_EXPORT(h3ToString)(0xffffffffffffffff, buf, bufSz));
        t_assert(strcmp(buf, "ffffffffffffffff") == 0,
                 "h3ToString failed on large input");
        t_assert(buf[bufSz - 1] == 0, "didn't null terminate");
    }

    TEST(stringToH3) {
        H3Index h3;
        t_assert(H3_EXPORT(stringToH3)("", &h3) == E_FAILED,
                 "no index from nothing");
        t_assert(H3_EXPORT(stringToH3)("**", &h3) == E_FAILED,
                 "no index from junk");
        t_assertSuccess(H3_EXPORT(stringToH3)("ffffffffffffffff", &h3));
        t_assert(h3 == 0xffffffffffffffff, "got expected on large input");
    }

    TEST(setH3Index) {
        H3Index h;
        setH3Index(&h, 5, 12, 1);
        t_assert(H3_GET_RESOLUTION(h) == 5, "resolution as expected");
        t_assert(H3_GET_BASE_CELL(h) == 12, "base cell as expected");
        t_assert(H3_GET_MODE(h) == H3_CELL_MODE, "mode as expected");
        for (int i = 1; i <= 5; i++) {
            t_assert(H3_GET_INDEX_DIGIT(h, i) == 1, "digit as expected");
        }
        for (int i = 6; i <= MAX_H3_RES; i++) {
            t_assert(H3_GET_INDEX_DIGIT(h, i) == 7,
                     "blanked digit as expected");
        }
        t_assert(h == 0x85184927fffffffL, "index matches expected");
    }

    TEST(isResClassIII) {
        LatLng coord = {0, 0};
        for (int i = 0; i <= MAX_H3_RES; i++) {
            H3Index h;
            t_assertSuccess(H3_EXPORT(latLngToCell)(&coord, i, &h));
            t_assert(H3_EXPORT(isResClassIII)(h) == isResolutionClassIII(i),
                     "matches existing definition");
        }
    }
}
