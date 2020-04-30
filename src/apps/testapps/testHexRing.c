/*
 * Copyright 2017-2018 Uber Technologies, Inc.
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

SUITE(hexRing) {
    GeoCoord sf = {0.659966917655, 2 * 3.14159 - 2.1364398519396};
    H3Index sfHex = H3_EXPORT(geoToH3)(&sf, 9);

    TEST(identityKRing) {
        int err;

        H3Index k0[] = {0};
        err = H3_EXPORT(hexRing)(sfHex, 0, k0);

        t_assert(err == 0, "No error on hexRing");
        t_assert(k0[0] == sfHex, "generated identity k-ring");
    }

    TEST(ring1) {
        int err;

        H3Index k1[] = {0, 0, 0, 0, 0, 0};
        H3Index expectedK1[] = {0x89283080ddbffff, 0x89283080c37ffff,
                                0x89283080c27ffff, 0x89283080d53ffff,
                                0x89283080dcfffff, 0x89283080dc3ffff};
        err = H3_EXPORT(hexRing)(sfHex, 1, k1);

        t_assert(err == 0, "No error on hexRing");

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
        int err;

        H3Index k2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        H3Index expectedK2[] = {
            0x89283080ca7ffff, 0x89283080cafffff, 0x89283080c33ffff,
            0x89283080c23ffff, 0x89283080c2fffff, 0x89283080d5bffff,
            0x89283080d43ffff, 0x89283080d57ffff, 0x89283080d1bffff,
            0x89283080dc7ffff, 0x89283080dd7ffff, 0x89283080dd3ffff};
        err = H3_EXPORT(hexRing)(sfHex, 2, k2);

        t_assert(err == 0, "No error on hexRing");

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

    TEST(nearPentagonRing1) {
        int err;

        H3Index nearPentagon = 0x837405fffffffff;
        H3Index kp1[] = {0, 0, 0, 0, 0, 0};
        err = H3_EXPORT(hexRing)(nearPentagon, 1, kp1);

        t_assert(err != 0, "Should return an error when hitting a pentagon");
    }

    TEST(nearPentagonRing2) {
        int err;

        H3Index nearPentagon = 0x837405fffffffff;
        H3Index kp2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        err = H3_EXPORT(hexRing)(nearPentagon, 2, kp2);

        t_assert(err != 0, "Should return an error when hitting a pentagon");
    }

    TEST(onPentagon) {
        int err;

        H3Index nearPentagon;
        setH3Index(&nearPentagon, 0, 4, 0);
        H3Index kp2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        err = H3_EXPORT(hexRing)(nearPentagon, 2, kp2);

        t_assert(err != 0,
                 "Should return an error when starting at a pentagon");
    }

    TEST(hexRing_matches_kRingInternal) {
        for (int res = 0; res < 2; res++) {
            for (int i = 0; i < NUM_BASE_CELLS; i++) {
                H3Index bc;
                setH3Index(&bc, 0, i, 0);
                int childrenSz = H3_EXPORT(maxUncompactSize)(&bc, 1, res);
                H3Index *children = calloc(childrenSz, sizeof(H3Index));
                H3_EXPORT(uncompact)(&bc, 1, children, childrenSz, res);

                for (int j = 0; j < childrenSz; j++) {
                    if (children[j] == 0) {
                        continue;
                    }

                    for (int k = 0; k < 3; k++) {
                        int ringSz = k != 0 ? 6 * k : 1;
                        int kSz = H3_EXPORT(maxKringSize)(k);

                        H3Index *ring = calloc(ringSz, sizeof(H3Index));
                        int failed = H3_EXPORT(hexRing)(children[j], k, ring);

                        if (!failed) {
                            H3Index *internalNeighbors =
                                calloc(kSz, sizeof(H3Index));
                            int *internalDistances = calloc(kSz, sizeof(int));
                            _kRingInternal(children[j], k, internalNeighbors,
                                           internalDistances, kSz, 0);

                            int found = 0;
                            int internalFound = 0;
                            for (int iRing = 0; iRing < ringSz; iRing++) {
                                if (ring[iRing] != 0) {
                                    found++;

                                    for (int iInternal = 0; iInternal < kSz;
                                         iInternal++) {
                                        if (internalNeighbors[iInternal] ==
                                            ring[iRing]) {
                                            internalFound++;

                                            t_assert(
                                                internalDistances[iInternal] ==
                                                    k,
                                                "Ring and internal agree on "
                                                "distance");

                                            break;
                                        }
                                    }

                                    t_assert(
                                        found == internalFound,
                                        "Ring and internal implementations "
                                        "produce same output");
                                }
                            }

                            free(internalNeighbors);
                            free(internalDistances);
                        }

                        free(ring);
                    }
                }

                free(children);
            }
        }
    }
}
