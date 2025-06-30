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
 * @brief tests index digits get/set and unused digits functions
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

    TEST(getMaxUnusedDigits) {
        // TODO: assert all specific values
        H3Index currentMask, previousMask = 0;
        for (int res = MAX_H3_RES; res >= 0; res--) {
            t_assertSuccess(H3_EXPORT(getMaxUnusedDigits)(res, &currentMask));

            if (res != MAX_H3_RES) {
                t_assert(currentMask > previousMask,
                         "mask should get bigger for increasing cell size");
            }
            previousMask = currentMask;

            if (res == MAX_H3_RES) {
                t_assert(currentMask == 0, "res 15 has 0 mask");
            }
            if (res == 14) {
                t_assert(currentMask == 7, "res 14 has 7 mask");
            }
            if (res == 13) {
                t_assert(currentMask == (7 | (7 << 3)),
                         "res 13 has expected mask");
            }
            if (res == 12) {
                t_assert(currentMask == (7 | ((7 | (7 << 3)) << 3)),
                         "res 12 has expected mask");
            }
        }
    }

    TEST(getMaxUnusedDigits_invalid) {
        H3Index mask;
        t_assert(H3_EXPORT(getMaxUnusedDigits)(-1, &mask) == E_RES_DOMAIN,
                 "get max unused digits -1");
        t_assert(H3_EXPORT(getMaxUnusedDigits)(16, &mask) == E_RES_DOMAIN,
                 "get max unused digits 16");
    }

    TEST(getUnusedDigits) {
        H3Index h;

        for (int expectedDigit = CENTER_DIGIT; expectedDigit < INVALID_DIGIT;
             expectedDigit++) {
            for (int resCell = 0; resCell <= MAX_H3_RES; resCell++) {
                setH3Index(&h, resCell, 0, expectedDigit);

                H3Index mask;
                H3Index expectedMask;
                mask = H3_EXPORT(getUnusedDigits)(h);
                t_assertSuccess(
                    H3_EXPORT(getMaxUnusedDigits)(resCell, &expectedMask));

                t_assert(mask == expectedMask,
                         "mask and expected mask are the same");

                if (resCell < MAX_H3_RES) {
                    H3Index h2;
                    H3Index mask2 = mask - 1;
                    t_assertSuccess(H3_EXPORT(setUnusedDigits)(h, mask2, &h2));

                    H3Index mask3 = H3_EXPORT(getUnusedDigits)(h2);

                    t_assert(mask3 == mask2,
                             "retrieved mask is same as expected");
                    t_assert(h2 < h, "index is logically modified");

                    H3Index mask4 = mask + 1;
                    H3Index h3 = 0;
                    t_assert(
                        H3_EXPORT(setUnusedDigits)(h, mask4, &h3) == E_DOMAIN,
                        "index cannot accept larger unused digits");
                    t_assert(h3 == 0, "index unmodified");
                }
            }
        }
    }
}
