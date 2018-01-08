/*
 * Copyright 2017 Uber Technologies, Inc.
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
 * @brief tests main H3 core library entry points.
 *
 *  usage: `testH3Api`
 */

#include <math.h>
#include "h3api.h"
#include "test.h"

BEGIN_TESTS(h3Api);

TEST(geoToH3_res) {
    GeoCoord anywhere = {0, 0};

    t_assert(H3_EXPORT(geoToH3)(&anywhere, -1) == 0,
             "resolution below 0 is invalid");
    t_assert(H3_EXPORT(geoToH3)(&anywhere, 16) == 0,
             "resolution above 15 is invalid");
}

TEST(geoToH3_coord) {
    GeoCoord invalidLat = {NAN, 0};
    GeoCoord invalidLon = {0, NAN};
    GeoCoord invalidLatLon = {INFINITY, -INFINITY};

    t_assert(H3_EXPORT(geoToH3)(&invalidLat, 1) == 0,
             "invalid latitude is rejected");
    t_assert(H3_EXPORT(geoToH3)(&invalidLon, 1) == 0,
             "invalid longitude is rejected");
    t_assert(H3_EXPORT(geoToH3)(&invalidLatLon, 1) == 0,
             "coordinates with infinity are rejected");
}

END_TESTS();