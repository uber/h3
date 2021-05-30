/*
 * Copyright 2020-2021 Uber Technologies, Inc.
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

#include "constants.h"
#include "h3api.h"
#include "iterators.h"
#include "test.h"
#include "utility.h"

/**
 * Basic checks around the great circle distance between the centers of two
 * neighboring cells. Tests positivity and commutativity.
 *
 * Tests the functions:
 *     distanceRads
 *     distanceKm
 *     distanceM
 *
 * @param  edge  H3 directed edge denoting neighboring cells
 */
static void haversine_assert(H3Index edge) {
    LatLng a, b;
    H3Index origin, destination;

    origin = H3_EXPORT(getDirectedEdgeOrigin)(edge);
    H3_EXPORT(cellToLatLng)(origin, &a);

    destination = H3_EXPORT(getDirectedEdgeDestination)(edge);
    H3_EXPORT(cellToLatLng)(destination, &b);

    char pos[] = "distance between cell centers should be positive";
    char comm[] = "pairwise cell distances should be commutative";

    double ab, ba;

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

    t_assert(H3_EXPORT(pointDistKm)(&a, &b) > H3_EXPORT(pointDistRads)(&a, &b),
             "measurement in kilometers should be greater than in radians");
    t_assert(H3_EXPORT(pointDistM)(&a, &b) > H3_EXPORT(pointDistKm)(&a, &b),
             "measurement in meters should be greater than in kilometers");
}

/**
 * Tests positivity of edge length calculation for the functions:
 *
 *      exactEdgeLengthRads
 *      exactEdgeLengthKm
 *      exactEdgeLengthM
 *
 * @param  edge  edge to compute the length of
 */
static void edge_length_assert(H3Index edge) {
    char msg[] = "edge has positive length";

    t_assert(H3_EXPORT(exactEdgeLengthRads)(edge) > 0, msg);
    t_assert(H3_EXPORT(exactEdgeLengthKm)(edge) > 0, msg);
    t_assert(H3_EXPORT(exactEdgeLengthM)(edge) > 0, msg);
}

/**
 * Test that cell area calculations are positive for the functions:
 *
 *      cellAreaRads2
 *      cellAreaKm2
 *      cellAreaM2
 *
 * @param  cell  cell to compute the area of
 */
static void cell_area_assert(H3Index cell) {
    char msg[] = "cell has positive area";

    t_assert(H3_EXPORT(cellAreaRads2)(cell) > 0, msg);
    t_assert(H3_EXPORT(cellAreaKm2)(cell) > 0, msg);
    t_assert(H3_EXPORT(cellAreaM2)(cell) > 0, msg);
}

/**
 * Apply a cell area calculation function to every cell on the earth at a given
 * resolution, and check that it sums up the total earth area.
 *
 * @param  res        resolution of the cells
 * @param  cell_area  callback to compute area of each cell
 * @param  target     expected earth area in some units
 * @param  tol        error tolerance allowed between expected and actual
 */
static void earth_area_test(int res, double (*cell_area)(H3Index),
                            double target, double tol) {
    double area = 0.0;
    for (IterCellsResolution iter = iterInitRes(res); iter.h;
         iterStepRes(&iter)) {
        area += (*cell_area)(iter.h);
    }

    t_assert(fabs(area - target) < tol,
             "sum of all cells should give earth area");
}

SUITE(h3CellAreaExhaustive) {
    TEST(haversine_distances) {
        iterateAllDirectedEdgesAtRes(0, haversine_assert);
        iterateAllDirectedEdgesAtRes(1, haversine_assert);
        iterateAllDirectedEdgesAtRes(2, haversine_assert);
        iterateAllDirectedEdgesAtRes(3, haversine_assert);
    }

    TEST(edge_length) {
        iterateAllDirectedEdgesAtRes(0, edge_length_assert);
        iterateAllDirectedEdgesAtRes(1, edge_length_assert);
        iterateAllDirectedEdgesAtRes(2, edge_length_assert);
        iterateAllDirectedEdgesAtRes(3, edge_length_assert);
    }

    TEST(cell_area_positive) {
        iterateAllIndexesAtRes(0, cell_area_assert);
        iterateAllIndexesAtRes(1, cell_area_assert);
        iterateAllIndexesAtRes(2, cell_area_assert);
        iterateAllIndexesAtRes(3, cell_area_assert);
    }

    TEST(cell_area_earth) {
        // earth area in different units
        double rads2 = 4 * M_PI;
        double km2 = rads2 * EARTH_RADIUS_KM * EARTH_RADIUS_KM;
        double m2 = km2 * 1000 * 1000;

        // Notice the drop in accuracy at resolution 1.
        // I think this has something to do with Class II vs Class III
        // resolutions.

        earth_area_test(0, H3_EXPORT(cellAreaRads2), rads2, 1e-14);
        earth_area_test(0, H3_EXPORT(cellAreaKm2), km2, 1e-6);
        earth_area_test(0, H3_EXPORT(cellAreaM2), m2, 1e0);

        earth_area_test(1, H3_EXPORT(cellAreaRads2), rads2, 1e-9);
        earth_area_test(1, H3_EXPORT(cellAreaKm2), km2, 1e-1);
        earth_area_test(1, H3_EXPORT(cellAreaM2), m2, 1e5);

        earth_area_test(2, H3_EXPORT(cellAreaRads2), rads2, 1e-12);
        earth_area_test(2, H3_EXPORT(cellAreaKm2), km2, 1e-5);
        earth_area_test(2, H3_EXPORT(cellAreaM2), m2, 1e0);

        earth_area_test(3, H3_EXPORT(cellAreaRads2), rads2, 1e-11);
        earth_area_test(3, H3_EXPORT(cellAreaKm2), km2, 1e-3);
        earth_area_test(3, H3_EXPORT(cellAreaM2), m2, 1e3);

        earth_area_test(4, H3_EXPORT(cellAreaRads2), rads2, 1e-11);
        earth_area_test(4, H3_EXPORT(cellAreaKm2), km2, 1e-3);
        earth_area_test(4, H3_EXPORT(cellAreaM2), m2, 1e2);
    }
}
