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
/** @file testLatLng.c
 * @brief Tests geographic coordinate functions
 *
 * usage: `testLatLng`
 */

#include <float.h>
#include <math.h>

#include "constants.h"
#include "h3api.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

/**
 * Test a function for all resolutions, where the value should be decreasing as
 * resolution increases.
 *
 * @param function
 * @param message
 */
static void testDecreasingFunction(H3Error (*function)(int, double *),
                                   const char *message) {
    double last = 0;
    double next;
    for (int i = MAX_H3_RES; i >= 0; i--) {
        t_assertSuccess(function(i, &next));
        t_assert(next > last, message);
        last = next;
    }
}

SUITE(latLng) {
    TEST(radsToDegs) {
        double originalRads = 1;
        double degs = H3_EXPORT(radsToDegs)(originalRads);
        double rads = H3_EXPORT(degsToRads)(degs);
        t_assert(fabs(rads - originalRads) < EPSILON_RAD,
                 "radsToDegs/degsToRads invertible");
    }

    TEST(distanceRads) {
        LatLng p1;
        setGeoDegs(&p1, 10, 10);
        LatLng p2;
        setGeoDegs(&p2, 0, 10);

        // TODO: Epsilon is relatively large
        t_assert(
            H3_EXPORT(greatCircleDistanceRads)(&p1, &p1) < EPSILON_RAD * 1000,
            "0 distance as expected");
        t_assert(fabs(H3_EXPORT(greatCircleDistanceRads)(&p1, &p2) -
                      H3_EXPORT(degsToRads)(10)) < EPSILON_RAD * 1000,
                 "distance along longitude as expected");
    }

    TEST(distanceRads_wrappedLongitude) {
        const LatLng negativeLongitude = {.lat = 0, .lng = -(M_PI + M_PI_2)};
        const LatLng zero = {.lat = 0, .lng = 0};

        t_assert(fabs(M_PI_2 - H3_EXPORT(greatCircleDistanceRads)(
                                   &negativeLongitude, &zero)) < EPSILON_RAD,
                 "Distance with wrapped longitude");
        t_assert(fabs(M_PI_2 - H3_EXPORT(greatCircleDistanceRads)(
                                   &zero, &negativeLongitude)) < EPSILON_RAD,
                 "Distance with wrapped longitude and swapped arguments");
    }

    TEST(distanceRads_antipodal) {
        // Antipodal points should give pi radians. The haversine term can
        // round fractionally above 1 for these inputs, which previously
        // produced NaN instead.
        LatLng a;
        setGeoDegs(&a, 12, 0);
        LatLng b;
        setGeoDegs(&b, -12, 180);
        double dist = H3_EXPORT(greatCircleDistanceRads)(&a, &b);
        t_assert(isfinite(dist), "Antipodal distance is finite");
        t_assert(fabs(dist - M_PI) < EPSILON_RAD, "Antipodal distance is pi");

        LatLng equatorA;
        setGeoDegs(&equatorA, 0, 0);
        LatLng equatorB;
        setGeoDegs(&equatorB, 0, 180);
        double equatorDist =
            H3_EXPORT(greatCircleDistanceRads)(&equatorA, &equatorB);
        t_assert(isfinite(equatorDist),
                 "Equatorial antipodal distance is finite");
        t_assert(fabs(equatorDist - M_PI) < EPSILON_RAD,
                 "Equatorial antipodal distance is pi");

        LatLng poleA;
        setGeoDegs(&poleA, 90, 0);
        LatLng poleB;
        setGeoDegs(&poleB, -90, 0);
        double poleDist = H3_EXPORT(greatCircleDistanceRads)(&poleA, &poleB);
        t_assert(isfinite(poleDist), "Pole-to-pole distance is finite");
        t_assert(fabs(poleDist - M_PI) < EPSILON_RAD,
                 "Pole-to-pole distance is pi");

        // Guard against an overly broad clamp: a point just short of
        // antipodal should resolve to its actual angle, not pi.
        LatLng nearA;
        setGeoDegs(&nearA, 0, 0);
        LatLng nearB;
        setGeoDegs(&nearB, 0, 179.9999);
        double nearDist = H3_EXPORT(greatCircleDistanceRads)(&nearA, &nearB);
        double expectedNearDist = H3_EXPORT(degsToRads)(179.9999);
        t_assert(isfinite(nearDist), "Near-antipodal distance is finite");
        // Comparing a haversine-computed distance against a direct degree
        // conversion needs a looser tolerance than EPSILON_RAD, same as the
        // existing distance-vs-degsToRads comparison above.
        t_assert(fabs(nearDist - expectedNearDist) < EPSILON_RAD * 1000,
                 "Near-antipodal distance matches the actual angle");
        t_assert(fabs(nearDist - M_PI) > EPSILON_RAD,
                 "Near-antipodal distance is distinguishable from pi");
    }

    TEST(distanceRads_hugeInputsClampNegativeA) {
        // greatCircleDistanceRads has no precondition that lat/lng be valid
        // angles. For most out-of-range inputs A stays >= 0 (the haversine
        // identity A = sin^2(centralAngle/2) holds for any real lat/lng),
        // but at a large enough magnitude sin()/cos() argument reduction
        // itself loses all precision and A can go negative. These specific
        // values are one such case, found by search and confirmed stable
        // at -O0 and -O2.
        LatLng a = {.lat = 7.3976404290321633e+52,
                    .lng = 7.2102191236323398e+52};
        LatLng b = {.lat = -6.8262031828527172e+52,
                    .lng = 4.7762907518189019e+52};
        double dist = H3_EXPORT(greatCircleDistanceRads)(&a, &b);
        t_assert(isfinite(dist), "Huge-input distance is finite, not NaN");
        t_assert(fabs(dist) < EPSILON_RAD, "Huge-input distance clamps to 0");
    }

    TEST(doubleConstants) {
        // Simple checks for ordering of values
        testDecreasingFunction(H3_EXPORT(getHexagonAreaAvgKm2),
                               "getHexagonAreaAvgKm2 ordering");
        testDecreasingFunction(H3_EXPORT(getHexagonAreaAvgM2),
                               "getHexagonAreaAvgM2 ordering");
        testDecreasingFunction(H3_EXPORT(getHexagonEdgeLengthAvgKm),
                               "getHexagonEdgeLengthAvgKm ordering");
        testDecreasingFunction(H3_EXPORT(getHexagonEdgeLengthAvgM),
                               "getHexagonEdgeLengthAvgM ordering");
    }

    TEST(doubleConstantsErrors) {
        double out;
        t_assert(H3_EXPORT(getHexagonAreaAvgKm2)(-1, &out) == E_RES_DOMAIN,
                 "getHexagonAreaAvgKm2 resolution negative");
        t_assert(H3_EXPORT(getHexagonAreaAvgKm2)(16, &out) == E_RES_DOMAIN,
                 "getHexagonAreaAvgKm2 resolution too high");
        t_assert(H3_EXPORT(getHexagonAreaAvgM2)(-1, &out) == E_RES_DOMAIN,
                 "getHexagonAreaAvgM2 resolution negative");
        t_assert(H3_EXPORT(getHexagonAreaAvgM2)(16, &out) == E_RES_DOMAIN,
                 "getHexagonAreaAvgM2 resolution too high");
        t_assert(H3_EXPORT(getHexagonEdgeLengthAvgKm)(-1, &out) == E_RES_DOMAIN,
                 "getHexagonEdgeLengthAvgKm resolution negative");
        t_assert(H3_EXPORT(getHexagonEdgeLengthAvgKm)(16, &out) == E_RES_DOMAIN,
                 "getHexagonEdgeLengthAvgKm resolution too high");
        t_assert(H3_EXPORT(getHexagonEdgeLengthAvgM)(-1, &out) == E_RES_DOMAIN,
                 "getHexagonEdgeLengthAvgM resolution negative");
        t_assert(H3_EXPORT(getHexagonEdgeLengthAvgM)(16, &out) == E_RES_DOMAIN,
                 "getHexagonEdgeLengthAvgM resolution too high");
    }

    TEST(intConstants) {
        // Simple checks for ordering of values
        int64_t last = 0;
        int64_t next;
        for (int i = 0; i <= MAX_H3_RES; i++) {
            t_assertSuccess(H3_EXPORT(getNumCells)(i, &next));
            t_assert(next > last, "getNumCells ordering");
            last = next;
        }
    }

    TEST(intConstantsErrors) {
        int64_t out;
        t_assert(H3_EXPORT(getNumCells)(-1, &out) == E_RES_DOMAIN,
                 "getNumCells resolution negative");
        t_assert(H3_EXPORT(getNumCells)(16, &out) == E_RES_DOMAIN,
                 "getNumCells resolution too high");
    }

    TEST(numHexagons) {
        // Test numHexagon counts of the number of *cells* at each resolution
        static const int64_t expected[] = {122L,
                                           842L,
                                           5882L,
                                           41162L,
                                           288122L,
                                           2016842L,
                                           14117882L,
                                           98825162L,
                                           691776122L,
                                           4842432842L,
                                           33897029882L,
                                           237279209162L,
                                           1660954464122L,
                                           11626681248842L,
                                           81386768741882L,
                                           569707381193162L};

        for (int r = 0; r <= MAX_H3_RES; r++) {
            int64_t num;
            t_assertSuccess(H3_EXPORT(getNumCells)(r, &num));
            t_assert(num == expected[r], "incorrect numHexagons count");
        }
    }
}
