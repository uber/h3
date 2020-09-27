/*
 * Copyright 2020 Uber Technologies, Inc.
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
 * @brief tests H3 cell area functions on a few specific cases
 *
 *  usage: `testH3CellArea`
 */

#include <math.h>

#include "h3Index.h"
#include "test.h"

static const double areasKm2[] = {
    2.562182162955496e+06, 4.476842018179411e+05, 6.596162242711056e+04,
    9.228872919002590e+03, 1.318694490797110e+03, 1.879593512281298e+02,
    2.687164354763186e+01, 3.840848847060638e+00, 5.486939641329893e-01,
    7.838600808637444e-02, 1.119834221989390e-02, 1.599777169186614e-03,
    2.285390931423380e-04, 3.264850232091780e-05, 4.664070326136774e-06,
    6.662957615868888e-07};

SUITE(h3CellArea) {
    TEST(specific_cell_area) {
        GeoCoord gc = {0.0, 0.0};
        for (int res = 0; res <= MAX_H3_RES - 1; res++) {
            H3Index cell = H3_EXPORT(geoToH3)(&gc, res);
            double area = H3_EXPORT(cellAreaKm2)(cell);

            t_assert(fabs(area - areasKm2[res]) < 1e-8,
                     "cell area should match expectation");
        }
    }
}
