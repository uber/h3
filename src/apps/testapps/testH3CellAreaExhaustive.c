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
 * @brief tests H3 cell area functions using tests over a large number
 *        of indexes.
 *
 *  usage: `testH3CellAreaExhaustive`
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

//#include "constants.h"
//#include "geoCoord.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

static void h3UniEdge_length_assertions(H3Index edge) {
    char msg[] = "edge has positive length";

    t_assert(H3_EXPORT(exactEdgeLengthRads)(edge) > 0, msg);
    t_assert(H3_EXPORT(exactEdgeLengthKm)(edge) > 0, msg);
    t_assert(H3_EXPORT(exactEdgeLengthM)(edge) > 0, msg);
}

static void cell_area_assertions(H3Index cell) {
    char msg[] = "cell has positive length";

    t_assert(H3_EXPORT(cellAreaRads2)(cell) > 0, msg);
    t_assert(H3_EXPORT(cellAreaKm2)(cell) > 0, msg);
    t_assert(H3_EXPORT(cellAreaM2)(cell) > 0, msg);
}

static void earthAreaTest(int res, double (*callback)(H3Index), double target,
                          double tol) {
    double area = mapSumAllCells_double(res, callback);

    t_assert(fabs(area - target) < tol,
             "sum of all cells should give earth area");
}

static void commutative_distance_assertions(H3Index edge) {
    GeoCoord a, b;
    double ab, ba;

    H3_EXPORT(h3ToGeo)
    (H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge), &a);
    H3_EXPORT(h3ToGeo)
    (H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(edge), &b);

    char pos[] = "distance between cell centers should be positive";
    char comm[] = "pairwise cell distances should be commutative";

    ab = H3_EXPORT(pointDistRads)(&a, &b);
    ba = H3_EXPORT(pointDistRads)(&b, &a);
    t_assert(ab > 0, pos);
    t_assert(ab == ba, comm);

    ab = H3_EXPORT(pointDistKm)(&a, &b);
    ba = H3_EXPORT(pointDistKm)(&b, &a);
    t_assert(ab > 0, pos);
    t_assert(ab == ba, comm);

    ab = H3_EXPORT(pointDistM)(&a, &b);
    ba = H3_EXPORT(pointDistM)(&b, &a);
    t_assert(ab > 0, pos);
    t_assert(ab == ba, comm);
}

SUITE(h3CellArea) {
    TEST(h3UniEdge_length) {
        iterateAllUnidirectionalEdgesAtRes(0, h3UniEdge_length_assertions);
        iterateAllUnidirectionalEdgesAtRes(1, h3UniEdge_length_assertions);
        iterateAllUnidirectionalEdgesAtRes(2, h3UniEdge_length_assertions);
        iterateAllUnidirectionalEdgesAtRes(3, h3UniEdge_length_assertions);
    }

    TEST(cellAreaPositive) {
        iterateAllIndexesAtRes(0, cell_area_assertions);
        iterateAllIndexesAtRes(1, cell_area_assertions);
        iterateAllIndexesAtRes(2, cell_area_assertions);
        iterateAllIndexesAtRes(3, cell_area_assertions);
    }

    TEST(cellAreaEarth) {
        // earth area in different units
        double rads2 = 4 * M_PI;
        double km2 = rads2 * EARTH_RADIUS_KM * EARTH_RADIUS_KM;
        double m2 = km2 * 1000 * 1000;

        // Notice the drop in accuracy at resolution 1.
        // I think this has something to do with Class II vs Class III
        // resolutions.

        earthAreaTest(0, H3_EXPORT(cellAreaRads2), rads2, 1e-14);
        earthAreaTest(0, H3_EXPORT(cellAreaKm2), km2, 1e-6);
        earthAreaTest(0, H3_EXPORT(cellAreaM2), m2, 1e0);

        earthAreaTest(1, H3_EXPORT(cellAreaRads2), rads2, 1e-9);
        earthAreaTest(1, H3_EXPORT(cellAreaKm2), km2, 1e-1);
        earthAreaTest(1, H3_EXPORT(cellAreaM2), m2, 1e5);

        earthAreaTest(2, H3_EXPORT(cellAreaRads2), rads2, 1e-12);
        earthAreaTest(2, H3_EXPORT(cellAreaKm2), km2, 1e-5);
        earthAreaTest(2, H3_EXPORT(cellAreaM2), m2, 1e0);

        earthAreaTest(3, H3_EXPORT(cellAreaRads2), rads2, 1e-11);
        earthAreaTest(3, H3_EXPORT(cellAreaKm2), km2, 1e-3);
        earthAreaTest(3, H3_EXPORT(cellAreaM2), m2, 1e3);

        earthAreaTest(4, H3_EXPORT(cellAreaRads2), rads2, 1e-11);
        earthAreaTest(4, H3_EXPORT(cellAreaKm2), km2, 1e-3);
        earthAreaTest(4, H3_EXPORT(cellAreaM2), m2, 1e2);
    }

    TEST(commutative_distances) {
        iterateAllUnidirectionalEdgesAtRes(0, commutative_distance_assertions);
        iterateAllUnidirectionalEdgesAtRes(1, commutative_distance_assertions);
        iterateAllUnidirectionalEdgesAtRes(2, commutative_distance_assertions);
        iterateAllUnidirectionalEdgesAtRes(3, commutative_distance_assertions);
    }
}
