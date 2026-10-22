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
#include "h3api.h"
#include "test.h"

SUITE(gridRing) {
    LatLng sf = {0.659966917655, 2 * 3.14159 - 2.1364398519396};
    H3Index sfHex;
    t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, 9, &sfHex));

    TEST(identityGridRing) {
        H3Index k0[] = {0};
        t_assertSuccess(H3_EXPORT(gridRing)(sfHex, 0, k0));
        t_assert(k0[0] == sfHex, "generated identity k-ring");
    }

    TEST(ring1) {
        H3Index k1[] = {0, 0, 0, 0, 0, 0};
        H3Index expectedK1[] = {0x89283080ddbffff, 0x89283080c37ffff,
                                0x89283080c27ffff, 0x89283080d53ffff,
                                0x89283080dcfffff, 0x89283080dc3ffff};
        t_assertSuccess(H3_EXPORT(gridRing)(sfHex, 1, k1));

        for (int i = 0; i < 6; i++) {
            t_assert(k1[i] != 0, "index is populated");
            int inList = 0;
            for (int j = 0; j < 6; j++) {
                if (k1[i] == expectedK1[j]) {
                    inList++;
                }
            }
            t_assert(inList == 1, "index found in expected set");
        }
    }

    TEST(ring2) {
        H3Index k2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        H3Index expectedK2[] = {
            0x89283080ca7ffff, 0x89283080cafffff, 0x89283080c33ffff,
            0x89283080c23ffff, 0x89283080c2fffff, 0x89283080d5bffff,
            0x89283080d43ffff, 0x89283080d57ffff, 0x89283080d1bffff,
            0x89283080dc7ffff, 0x89283080dd7ffff, 0x89283080dd3ffff};
        t_assertSuccess(H3_EXPORT(gridRing)(sfHex, 2, k2));

        for (int i = 0; i < 12; i++) {
            t_assert(k2[i] != 0, "index is populated");
            int inList = 0;
            for (int j = 0; j < 12; j++) {
                if (k2[i] == expectedK2[j]) {
                    inList++;
                }
            }
            t_assert(inList == 1, "index found in expected set");
        }
    }

    TEST(gridRing0_PolarPentagon) {
        H3Index polar;
        setH3Index(&polar, 0, 4, 0);
        H3Index k2[] = {0, 0, 0, 0, 0, 0};
        H3Index expectedK2[] = {0x8007fffffffffff, 0x8001fffffffffff,
                                0x8011fffffffffff, 0x801ffffffffffff,
                                0x8019fffffffffff, 0};
        t_assertSuccess(H3_EXPORT(gridRing)(polar, 1, k2));

        int k2present = 0;
        for (int i = 0; i < 6; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 6; j++) {
                    if (k2[i] == expectedK2[j]) {
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 5, "pentagon has 5 neighbors in k-ring 1");
    }

    TEST(gridRing1_PolarPentagon) {
        H3Index polar;
        setH3Index(&polar, 1, 4, 0);
        H3Index k2[] = {0, 0, 0, 0, 0, 0};
        H3Index expectedK2[] = {0x81093ffffffffff, 0x81097ffffffffff,
                                0x8108fffffffffff, 0x8108bffffffffff,
                                0x8109bffffffffff, 0};
        t_assertSuccess(H3_EXPORT(gridRing)(polar, 1, k2));

        int k2present = 0;
        for (int i = 0; i < 6; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 6; j++) {
                    if (k2[i] == expectedK2[j]) {
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 5, "pentagon has 5 neighbors in k-ring 1");
    }

    TEST(gridRing1_PolarPentagon_k3) {
        H3Index polar;
        setH3Index(&polar, 1, 4, 0);
        H3Index k2[18] = {0};
        H3Index expectedK2[18] = {0x811fbffffffffff,
                                  0x81003ffffffffff,
                                  0x81183ffffffffff,
                                  0x8111bffffffffff,
                                  0x81067ffffffffff,
                                  0x811e7ffffffffff,
                                  0x8101bffffffffff,
                                  0x81107ffffffffff,
                                  0x81063ffffffffff,
                                  0x811e3ffffffffff,
                                  0x8119bffffffffff,
                                  0x81103ffffffffff,
                                  0x81007ffffffffff,
                                  0x81187ffffffffff,
                                  0x8107bffffffffff,
                                  0,
                                  0,
                                  0};
        t_assertSuccess(H3_EXPORT(gridRing)(polar, 3, k2));

        int k2present = 0;
        for (int i = 0; i < 18; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 18; j++) {
                    if (k2[i] == expectedK2[j]) {
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 15, "pentagon has 15 neighbors in k-ring 3");
    }

    TEST(gridRing1_Pentagon_k4) {
        H3Index pent;
        setH3Index(&pent, 1, 14, 0);
        H3Index k2[24] = {0};
        H3Index expectedK2[24] = {
            0x81227ffffffffff,
            0x81293ffffffffff,
            0x8136bffffffffff,
            0x81167ffffffffff,
            0x81477ffffffffff,
            0x810dbffffffffff,
            0x81473ffffffffff,
            0x81237ffffffffff,
            0x81127ffffffffff,
            0x8126bffffffffff,
            0x81177ffffffffff,
            0x810d3ffffffffff,
            0x8150fffffffffff,
            0x8102fffffffffff,
            0x8129bffffffffff,
            0x8102bffffffffff,
            0x81507ffffffffff,
            0x8136fffffffffff,
            0x8127bffffffffff,
            0x81137ffffffffff,
            0,
            0,
            0,
            0,
        };
        t_assertSuccess(H3_EXPORT(gridRing)(pent, 4, k2));

        int k2present = 0;
        for (int i = 0; i < 24; i++) {
            if (k2[i] != 0) {
                k2present++;
                int inList = 0;
                for (int j = 0; j < 24; j++) {
                    if (k2[i] == expectedK2[j]) {
                        inList++;
                    }
                }
                t_assert(inList == 1, "index found in expected set");
            }
        }
        t_assert(k2present == 20, "pentagon has 20 neighbors in k-ring 4");
    }

    TEST(gridRing_matches_gridDiskDistancesSafe) {
        for (int res = 0; res < 2; res++) {
            for (int i = 0; i < NUM_BASE_CELLS; i++) {
                H3Index bc;
                setH3Index(&bc, 0, i, 0);
                int64_t childrenSz;
                t_assertSuccess(
                    H3_EXPORT(uncompactCellsSize)(&bc, 1, res, &childrenSz));
                H3Index *children = calloc(childrenSz, sizeof(H3Index));
                t_assertSuccess(H3_EXPORT(uncompactCells)(&bc, 1, children,
                                                          childrenSz, res));

                for (int j = 0; j < childrenSz; j++) {
                    if (children[j] == 0) {
                        continue;
                    }

                    for (int k = 0; k < 3; k++) {
                        int64_t kSz;
                        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &kSz));

                        int64_t ringSize;
                        t_assertSuccess(
                            H3_EXPORT(maxGridRingSize)(k, &ringSize));

                        H3Index *ring = calloc(ringSize, sizeof(H3Index));
                        t_assertSuccess(
                            H3_EXPORT(gridRing)(children[j], k, ring));

                        H3Index *internalNeighbors =
                            calloc(kSz, sizeof(H3Index));
                        int *internalDistances = calloc(kSz, sizeof(int));
                        t_assertSuccess(H3_EXPORT(gridDiskDistancesSafe)(
                            children[j], k, internalNeighbors,
                            internalDistances));

                        int found = 0;
                        int internalFound = 0;
                        for (int iRing = 0; iRing < ringSize; iRing++) {
                            if (ring[iRing] != 0) {
                                found++;

                                for (int64_t iInternal = 0; iInternal < kSz;
                                     iInternal++) {
                                    if (internalNeighbors[iInternal] ==
                                        ring[iRing]) {
                                        internalFound++;

                                        t_assert(
                                            internalDistances[iInternal] == k,
                                            "Ring and internal agree on "
                                            "distance");

                                        break;
                                    }
                                }

                                t_assert(found == internalFound,
                                         "Ring and internal implementations "
                                         "produce same output");
                            }
                        }
                        free(internalNeighbors);
                        free(internalDistances);
                        free(ring);
                    }
                }

                free(children);
            }
        }
    }

    TEST(maxGridRingSize_invalid) {
        int64_t sz;
        t_assert(H3_EXPORT(maxGridRingSize)(-1, &sz) == E_DOMAIN,
                 "negative k is invalid");
    }

    TEST(maxGridRingSize_identity) {
        int64_t sz;
        t_assertSuccess(H3_EXPORT(maxGridRingSize)(0, &sz));
        t_assert(sz == 1, "k = 0 returns 1");
    }

    TEST(maxGridRingSize) {
        int64_t sz;
        t_assertSuccess(H3_EXPORT(maxGridRingSize)(2, &sz));
        t_assert(sz == 12, "k = 2 returns 12");
    }
}
