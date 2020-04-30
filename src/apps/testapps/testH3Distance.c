/*
 * Copyright 2018-2019 Uber Technologies, Inc.
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
 * @brief tests H3 distance function.
 *
 *  usage: `testH3Distance`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algos.h"
#include "baseCells.h"
#include "constants.h"
#include "h3Index.h"
#include "h3api.h"
#include "localij.h"
#include "test.h"
#include "utility.h"

SUITE(h3Distance) {
    // Some indexes that represent base cells. Base cells
    // are hexagons except for `pent1`.
    H3Index bc1 = H3_INIT;
    setH3Index(&bc1, 0, 15, 0);

    H3Index bc2 = H3_INIT;
    setH3Index(&bc2, 0, 8, 0);

    H3Index bc3 = H3_INIT;
    setH3Index(&bc3, 0, 31, 0);

    H3Index pent1 = H3_INIT;
    setH3Index(&pent1, 0, 4, 0);

    TEST(testIndexDistance) {
        H3Index bc = 0;
        setH3Index(&bc, 1, 17, 0);
        H3Index p = 0;
        setH3Index(&p, 1, 14, 0);
        H3Index p2;
        setH3Index(&p2, 1, 14, 2);
        H3Index p3;
        setH3Index(&p3, 1, 14, 3);
        H3Index p4;
        setH3Index(&p4, 1, 14, 4);
        H3Index p5;
        setH3Index(&p5, 1, 14, 5);
        H3Index p6;
        setH3Index(&p6, 1, 14, 6);

        t_assert(H3_EXPORT(h3Distance)(bc, p) == 3, "distance onto pentagon");
        t_assert(H3_EXPORT(h3Distance)(bc, p2) == 2, "distance onto p2");
        t_assert(H3_EXPORT(h3Distance)(bc, p3) == 3, "distance onto p3");
        // TODO works correctly but is rejected due to possible pentagon
        // distortion.
        //    t_assert(H3_EXPORT(h3Distance)(bc, p4) == 3, "distance onto p4");
        //    t_assert(H3_EXPORT(h3Distance)(bc, p5) == 4, "distance onto p5");
        t_assert(H3_EXPORT(h3Distance)(bc, p6) == 2, "distance onto p6");
    }

    TEST(testIndexDistance2) {
        H3Index origin = 0x820c4ffffffffffL;
        // Destination is on the other side of the pentagon
        H3Index destination = 0x821ce7fffffffffL;

        // TODO doesn't work because of pentagon distortion. Both should be 5.
        t_assert(H3_EXPORT(h3Distance)(destination, origin) == -1,
                 "distance in res 2 across pentagon");
        t_assert(H3_EXPORT(h3Distance)(origin, destination) == -1,
                 "distance in res 2 across pentagon (reversed)");
    }

    TEST(h3DistanceBaseCells) {
        t_assert(H3_EXPORT(h3Distance)(bc1, pent1) == 1,
                 "distance to neighbor is 1 (15, 4)");
        t_assert(H3_EXPORT(h3Distance)(bc1, bc2) == 1,
                 "distance to neighbor is 1 (15, 8)");
        t_assert(H3_EXPORT(h3Distance)(bc1, bc3) == 1,
                 "distance to neighbor is 1 (15, 31)");
        t_assert(H3_EXPORT(h3Distance)(pent1, bc3) == -1,
                 "distance to neighbor is invalid");
    }

    TEST(ijkDistance) {
        CoordIJK z = {0, 0, 0};
        CoordIJK i = {1, 0, 0};
        CoordIJK ik = {1, 0, 1};
        CoordIJK ij = {1, 1, 0};
        CoordIJK j2 = {0, 2, 0};

        t_assert(ijkDistance(&z, &z) == 0, "identity distance 0,0,0");
        t_assert(ijkDistance(&i, &i) == 0, "identity distance 1,0,0");
        t_assert(ijkDistance(&ik, &ik) == 0, "identity distance 1,0,1");
        t_assert(ijkDistance(&ij, &ij) == 0, "identity distance 1,1,0");
        t_assert(ijkDistance(&j2, &j2) == 0, "identity distance 0,2,0");

        t_assert(ijkDistance(&z, &i) == 1, "0,0,0 to 1,0,0");
        t_assert(ijkDistance(&z, &j2) == 2, "0,0,0 to 0,2,0");
        t_assert(ijkDistance(&z, &ik) == 1, "0,0,0 to 1,0,1");
        t_assert(ijkDistance(&i, &ik) == 1, "1,0,0 to 1,0,1");
        t_assert(ijkDistance(&ik, &j2) == 3, "1,0,1 to 0,2,0");
        t_assert(ijkDistance(&ij, &ik) == 2, "1,0,1 to 1,1,0");
    }

    TEST(h3DistanceResolutionMismatch) {
        t_assert(
            H3_EXPORT(h3Distance)(0x832830fffffffffL, 0x822837fffffffffL) == -1,
            "cannot compare at different resolutions");
    }

    TEST(h3DistanceEdge) {
        H3Index origin = 0x832830fffffffffL;
        H3Index dest = 0x832834fffffffffL;
        H3Index edge = H3_EXPORT(getH3UnidirectionalEdge(origin, dest));

        t_assert(0 != edge, "test edge is valid");
        t_assert(H3_EXPORT(h3Distance)(edge, origin) == 0,
                 "edge has zero distance to origin");
        t_assert(H3_EXPORT(h3Distance)(origin, edge) == 0,
                 "origin has zero distance to edge");

        t_assert(H3_EXPORT(h3Distance)(edge, dest) == 1,
                 "edge has distance to destination");
        t_assert(H3_EXPORT(h3Distance)(edge, dest) == 1,
                 "destination has distance to edge");
    }
}
