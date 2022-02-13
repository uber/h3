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

#include <stdlib.h>

#include "algos.h"
#include "test.h"

SUITE(gridDisksUnsafe) {
    LatLng sf = {0.659966917655, 2 * 3.14159 - 2.1364398519396};
    H3Index sfHex;
    t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, 9, &sfHex));
    H3Index *sfHexPtr = &sfHex;

    H3Index k1[] = {0x89283080ddbffff, 0x89283080c37ffff, 0x89283080c27ffff,
                    0x89283080d53ffff, 0x89283080dcfffff, 0x89283080dc3ffff};
    H3Index withPentagon[] = {0x8029fffffffffff, 0x801dfffffffffff};

    TEST(identityGridDiskCells) {
        H3Index k0[] = {0};
        t_assertSuccess(H3_EXPORT(gridDisksUnsafe)(sfHexPtr, 1, 0, k0));

        t_assert(k0[0] == sfHex, "generated identity k-ring");
    }

    TEST(ring1of1) {
        H3Index allKrings[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        t_assertSuccess(H3_EXPORT(gridDisksUnsafe)(k1, 6, 1, allKrings));

        for (int i = 0; i < 42; i++) {
            t_assert(allKrings[i] != 0, "index is populated");
            if (i % 7 == 0) {
                int index = i / 7;
                t_assert(k1[index] == allKrings[i],
                         "The beginning of the segment is the correct hexagon");
            }
        }
    }

    TEST(ring2of1) {
        H3Index *allKrings2 = calloc(6 * (1 + 6 + 12), sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(gridDisksUnsafe)(k1, 6, 2, allKrings2));

        for (int i = 0; i < (6 * (1 + 6 + 12)); i++) {
            t_assert(allKrings2[i] != 0, "index is populated");

            if (i % (1 + 6 + 12) == 0) {
                int index = i / (1 + 6 + 12);
                t_assert(k1[index] == allKrings2[i],
                         "The beginning of the segment is the correct hexagon");
            }
        }
        free(allKrings2);
    }

    TEST(failed) {
        H3Index *allKrings = calloc(2 * (1 + 6), sizeof(H3Index));
        t_assert(H3_EXPORT(gridDisksUnsafe)(withPentagon, 2, 1, allKrings) ==
                     E_PENTAGON,
                 "Expected error on gridDisksUnsafe");
        free(allKrings);
    }

    TEST(invalid_k) {
        t_assert(H3_EXPORT(gridDisksUnsafe)(k1, 6, -1, NULL) == E_DOMAIN,
                 "gridDisksUnsafe invalid k");
    }
}
