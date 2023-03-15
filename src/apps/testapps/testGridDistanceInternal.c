/*
 * Copyright 2018-2020 Uber Technologies, Inc.
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
 *  usage: `testGridDistanceInternal`
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

SUITE(gridDistanceInternal) {
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
}
