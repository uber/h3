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

#include <stdlib.h>

#include "algos.h"
#include "constants.h"
#include "h3Index.h"
#include "test.h"

SUITE(gridRingInternal) {
    LatLng sf = {0.659966917655, 2 * 3.14159 - 2.1364398519396};
    H3Index sfHex;
    t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, 9, &sfHex));

    TEST(identityGridRing) {
        H3Index k0[] = {0};
        t_assertSuccess(_gridRingInternal(sfHex, 0, k0));
        t_assert(k0[0] == sfHex, "generated identity k-ring");
    }

    TEST(negativeK) {
        H3Index k0[] = {0};
        t_assert(_gridRingInternal(sfHex, -1, k0) == E_DOMAIN,
                 "Should return an error when k is negative");
    }

    TEST(gridDiskInvalid) {
        const int k = 1000;
        int64_t kSz;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &kSz));
        H3Index *neighbors = calloc(kSz, sizeof(H3Index));
        t_assert(_gridRingInternal(0x7fffffffffffffff, k, neighbors) ==
                     E_CELL_INVALID,
                 "_gridRingInternal returns error for invalid input");
        free(neighbors);
    }

    TEST(gridDiskInvalidDigit) {
        const int k = 2;
        int64_t kSz;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &kSz));
        H3Index *neighbors = calloc(kSz, sizeof(H3Index));
        t_assert(_gridRingInternal(0x4d4b00fe5c5c3030, k, neighbors) ==
                     E_CELL_INVALID,
                 "_gridRingInternal returns error for invalid input");
        free(neighbors);
    }
}
