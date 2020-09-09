/*
 * Copyright 2017-2019 Uber Technologies, Inc.
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
/** @file testGeoCoord.c
 * @brief Tests geographic coordinate functions
 *
 * usage: `testGeoCoord`
 */

#include <math.h>

#include "constants.h"
#include "geoCoord.h"
#include "h3api.h"
#include "test.h"
#include "utility.h"

/**
 * Test a function for all resolutions, where the value should be decreasing as
 * resolution increases.
 *
 * @param function
 * @param message
 */
static void testDecreasingFunction(double (*function)(int),
                                   const char* message) {
    double last = 0;
    double next;
    for (int i = MAX_H3_RES; i >= 0; i--) {
        next = function(i);
        t_assert(next > last, message);
        last = next;
    }
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
    H3Index origin = H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edge);
    H3Index destination =
        H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(edge);

    GeoCoord a, b;

    H3_EXPORT(h3ToGeo)(origin, &a);
    H3_EXPORT(h3ToGeo)(destination, &b);

    double ab = H3_EXPORT(pointDistM)(&a, &b);
    double ba = H3_EXPORT(pointDistM)(&b, &a);

    t_assert(ab > 0, "distance between cell centers is positive");
    t_assert(ab == ba, "pairwise cell distances should be commutative");
}

SUITE(geoCoord) {
    TEST(radsToDegs) {
        double originalRads = 1;
        double degs = H3_EXPORT(radsToDegs)(originalRads);
        double rads = H3_EXPORT(degsToRads)(degs);
        t_assert(fabs(rads - originalRads) < EPSILON_RAD,
                 "radsToDegs/degsToRads invertible");
    }

    TEST(pointDistRads) {
        GeoCoord p1;
        setGeoDegs(&p1, 10, 10);
        GeoCoord p2;
        setGeoDegs(&p2, 0, 10);

        // TODO: Epsilon is relatively large
        t_assert(H3_EXPORT(pointDistRads)(&p1, &p1) < EPSILON_RAD * 1000,
                 "0 distance as expected");
        t_assert(fabs(H3_EXPORT(pointDistRads)(&p1, &p2) -
                      H3_EXPORT(degsToRads)(10)) < EPSILON_RAD * 1000,
                 "distance along longitude as expected");
    }

    TEST(constrainLatLng) {
        t_assert(constrainLat(0) == 0, "lat 0");
        t_assert(constrainLat(1) == 1, "lat 1");
        t_assert(constrainLat(M_PI_2) == M_PI_2, "lat pi/2");
        t_assert(constrainLat(M_PI) == 0, "lat pi");
        t_assert(constrainLat(M_PI + 1) == 1, "lat pi+1");
        t_assert(constrainLat(2 * M_PI + 1) == 1, "lat 2pi+1");

        t_assert(constrainLng(0) == 0, "lng 0");
        t_assert(constrainLng(1) == 1, "lng 1");
        t_assert(constrainLng(M_PI) == M_PI, "lng pi");
        t_assert(constrainLng(2 * M_PI) == 0, "lng 2pi");
        t_assert(constrainLng(3 * M_PI) == M_PI, "lng 2pi");
        t_assert(constrainLng(4 * M_PI) == 0, "lng 4pi");
    }

    TEST(_geoAzDistanceRads_noop) {
        GeoCoord start = {15, 10};
        GeoCoord out;
        GeoCoord expected = {15, 10};

        _geoAzDistanceRads(&start, 0, 0, &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "0 distance produces same point");
    }

    TEST(_geoAzDistanceRads_dueNorthSouth) {
        GeoCoord start;
        GeoCoord out;
        GeoCoord expected;

        // Due north to north pole
        setGeoDegs(&start, 45, 1);
        setGeoDegs(&expected, 90, 0);
        _geoAzDistanceRads(&start, 0, H3_EXPORT(degsToRads)(45), &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "due north to north pole produces north pole");

        // Due north to south pole, which doesn't get wrapped correctly
        setGeoDegs(&start, 45, 1);
        setGeoDegs(&expected, 270, 1);
        _geoAzDistanceRads(&start, 0, H3_EXPORT(degsToRads)(45 + 180), &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "due north to south pole produces south pole");

        // Due south to south pole
        setGeoDegs(&start, -45, 2);
        setGeoDegs(&expected, -90, 0);
        _geoAzDistanceRads(&start, H3_EXPORT(degsToRads)(180),
                           H3_EXPORT(degsToRads)(45), &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "due south to south pole produces south pole");

        // Due north to non-pole
        setGeoDegs(&start, -45, 10);
        setGeoDegs(&expected, -10, 10);
        _geoAzDistanceRads(&start, 0, H3_EXPORT(degsToRads)(35), &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "due north produces expected result");
    }

    TEST(_geoAzDistanceRads_poleToPole) {
        GeoCoord start;
        GeoCoord out;
        GeoCoord expected;

        // Azimuth doesn't really matter in this case. Any azimuth from the
        // north pole is south, any azimuth from the south pole is north.

        setGeoDegs(&start, 90, 0);
        setGeoDegs(&expected, -90, 0);
        _geoAzDistanceRads(&start, H3_EXPORT(degsToRads)(12),
                           H3_EXPORT(degsToRads)(180), &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "some direction to south pole produces south pole");

        setGeoDegs(&start, -90, 0);
        setGeoDegs(&expected, 90, 0);
        _geoAzDistanceRads(&start, H3_EXPORT(degsToRads)(34),
                           H3_EXPORT(degsToRads)(180), &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "some direction to north pole produces north pole");
    }

    TEST(_geoAzDistanceRads_invertible) {
        GeoCoord start;
        setGeoDegs(&start, 15, 10);
        GeoCoord out;

        double azimuth = H3_EXPORT(degsToRads)(20);
        double degrees180 = H3_EXPORT(degsToRads)(180);
        double distance = H3_EXPORT(degsToRads)(15);

        _geoAzDistanceRads(&start, azimuth, distance, &out);
        t_assert(fabs(H3_EXPORT(pointDistRads)(&start, &out) - distance) <
                     EPSILON_RAD,
                 "moved distance is as expected");

        GeoCoord start2 = out;
        _geoAzDistanceRads(&start2, azimuth + degrees180, distance, &out);
        // TODO: Epsilon is relatively large
        t_assert(H3_EXPORT(pointDistRads)(&start, &out) < 0.01,
                 "moved back to origin");
    }

    TEST(pointDistRads_wrappedLongitude) {
        const GeoCoord negativeLongitude = {.lat = 0, .lon = -(M_PI + M_PI_2)};
        const GeoCoord zero = {.lat = 0, .lon = 0};

        t_assert(fabs(M_PI_2 - H3_EXPORT(pointDistRads)(&negativeLongitude,
                                                        &zero)) < EPSILON_RAD,
                 "Distance with wrapped longitude");
        t_assert(fabs(M_PI_2 - H3_EXPORT(pointDistRads)(
                                   &zero, &negativeLongitude)) < EPSILON_RAD,
                 "Distance with wrapped longitude and swapped arguments");
    }

    TEST(doubleConstants) {
        // Simple checks for ordering of values
        testDecreasingFunction(H3_EXPORT(hexAreaKm2), "hexAreaKm2 ordering");
        testDecreasingFunction(H3_EXPORT(hexAreaM2), "hexAreaM2 ordering");
        testDecreasingFunction(H3_EXPORT(edgeLengthKm),
                               "edgeLengthKm ordering");
        testDecreasingFunction(H3_EXPORT(edgeLengthM), "edgeLengthM ordering");
    }

    TEST(intConstants) {
        // Simple checks for ordering of values
        int64_t last = 0;
        int64_t next;
        for (int i = 0; i <= MAX_H3_RES; i++) {
            next = H3_EXPORT(numHexagons)(i);
            t_assert(next > last, "numHexagons ordering");
            last = next;
        }
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
    }
}
