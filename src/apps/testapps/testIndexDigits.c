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
 *  usage: `testIndexDigits`
 */

#include <math.h>

#include "h3Index.h"
#include "h3api.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

SUITE(indexDigits) {
    TEST(getIndexDigitForCell) {
        H3Index h;
        LatLng anywhere = {0, 0};

        for (int resCell = 0; resCell <= MAX_H3_RES; resCell++) {
            t_assertSuccess(H3_EXPORT(latLngToCell)(&anywhere, resCell, &h));
            for (int resDigit = 1; resDigit <= MAX_H3_RES; resDigit++) {
                int digit;
                t_assertSuccess(H3_EXPORT(getIndexDigit)(h, resDigit, &digit));
                if (resDigit <= resCell) {
                    t_assert(digit >= CENTER_DIGIT && digit < INVALID_DIGIT,
                             "digit in valid range");
                } else {
                    t_assert(digit == INVALID_DIGIT,
                             "digit should be 'invalid'");
                }
            }
        }

        int digitUnused;
        t_assert(H3_EXPORT(getIndexDigit)(h, -1, &digitUnused) == E_RES_DOMAIN,
                 "negative resolution");
        t_assert(H3_EXPORT(getIndexDigit)(h, 0, &digitUnused) == E_RES_DOMAIN,
                 "zero resolution");
        t_assert(H3_EXPORT(getIndexDigit)(h, 16, &digitUnused) == E_RES_DOMAIN,
                 "too high resolution");
    }

    TEST(getIndexDigitForSetCell) {
        H3Index h;

        for (int expectedDigit = CENTER_DIGIT; expectedDigit < INVALID_DIGIT;
             expectedDigit++) {
            for (int resCell = 0; resCell <= MAX_H3_RES; resCell++) {
                setH3Index(&h, resCell, 0, expectedDigit);
                for (int resDigit = 1; resDigit <= MAX_H3_RES; resDigit++) {
                    int digit;
                    t_assertSuccess(
                        H3_EXPORT(getIndexDigit)(h, resDigit, &digit));
                    if (resDigit <= resCell) {
                        t_assert(digit == expectedDigit,
                                 "digit should be expected");
                    } else {
                        t_assert(digit == INVALID_DIGIT,
                                 "digit should be 'invalid'");
                    }
                }
            }
        }
    }
}
