/*
 * Copyright 2017-2021 Uber Technologies, Inc.
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
/** @file testLatLngInternal.c
 * @brief Tests geographic coordinate functions
 *
 * usage: `testLatLngInternal`
 */

#include <float.h>
#include <math.h>

#include "constants.h"
#include "h3api.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

SUITE(latLng) {
    TEST(geoAlmostEqualThreshold) {
        LatLng a = {15, 10};
        LatLng b = {15, 10};
        t_assert(geoAlmostEqualThreshold(&a, &b, DBL_EPSILON), "same point");

        b.lat = 15.00001;
        b.lng = 10.00002;
        t_assert(geoAlmostEqualThreshold(&a, &b, 0.0001),
                 "differences under threshold");

        b.lat = 15.00001;
        b.lng = 10;
        t_assert(!geoAlmostEqualThreshold(&a, &b, 0.000001),
                 "lat over threshold");

        b.lat = 15;
        b.lng = 10.00001;
        t_assert(!geoAlmostEqualThreshold(&a, &b, 0.000001),
                 "lng over threshold");
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
        LatLng start = {15, 10};
        LatLng out;
        LatLng expected = {15, 10};

        _geoAzDistanceRads(&start, 0, 0, &out);
        t_assert(geoAlmostEqual(&expected, &out),
                 "0 distance produces same point");
    }

    TEST(_geoAzDistanceRads_dueNorthSouth) {
        LatLng start;
        LatLng out;
        LatLng expected;

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
        LatLng start;
        LatLng out;
        LatLng expected;

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
        LatLng start;
        setGeoDegs(&start, 15, 10);
        LatLng out;

        double azimuth = H3_EXPORT(degsToRads)(20);
        double degrees180 = H3_EXPORT(degsToRads)(180);
        double distance = H3_EXPORT(degsToRads)(15);

        _geoAzDistanceRads(&start, azimuth, distance, &out);
        t_assert(fabs(H3_EXPORT(greatCircleDistanceRads)(&start, &out) -
                      distance) < EPSILON_RAD,
                 "moved distance is as expected");

        LatLng start2 = out;
        _geoAzDistanceRads(&start2, azimuth + degrees180, distance, &out);
        // TODO: Epsilon is relatively large
        t_assert(H3_EXPORT(greatCircleDistanceRads)(&start, &out) < 0.01,
                 "moved back to origin");
    }
}
