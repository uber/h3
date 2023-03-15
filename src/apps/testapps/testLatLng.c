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
