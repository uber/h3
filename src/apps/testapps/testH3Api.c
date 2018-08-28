/*
 * Copyright 2017-2018 Uber Technologies, Inc.
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
#include "geoCoord.h"
#include "h3api.h"
#include "test.h"
#include "utility.h"

SUITE(h3Api) {
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

    TEST(h3ToGeoBoundary_classIIIEdgeVertex) {
        // Bug test for https://github.com/uber/h3/issues/45
        char* hexes[] = {
            "894cc5349b7ffff", "894cc534d97ffff", "894cc53682bffff",
            "894cc536b17ffff", "894cc53688bffff", "894cead92cbffff",
            "894cc536537ffff", "894cc5acbabffff", "894cc536597ffff"};
        int numHexes = sizeof(hexes) / sizeof(hexes[0]);
        H3Index h3;
        GeoBoundary b;
        for (int i = 0; i < numHexes; i++) {
            h3 = H3_EXPORT(stringToH3)(hexes[i]);
            H3_EXPORT(h3ToGeoBoundary)(h3, &b);
            t_assert(b.numVerts == 7, "got expected vertex count");
        }
    }

    TEST(h3ToGeoBoundary_classIIIEdgeVertex_exact) {
        // Bug test for https://github.com/uber/h3/issues/45
        H3Index h3 = H3_EXPORT(stringToH3)("894cc536537ffff");
        GeoBoundary boundary;
        boundary.numVerts = 7;
        setGeoDegs(&boundary.verts[0], 18.043333154, -66.27836523500002);
        setGeoDegs(&boundary.verts[1], 18.042238363, -66.27929062800001);
        setGeoDegs(&boundary.verts[2], 18.040818259, -66.27854193899998);
        setGeoDegs(&boundary.verts[3], 18.040492975, -66.27686786700002);
        setGeoDegs(&boundary.verts[4], 18.041040385, -66.27640518300001);
        setGeoDegs(&boundary.verts[5], 18.041757122, -66.27596711500001);
        setGeoDegs(&boundary.verts[6], 18.043007860, -66.27669118199998);
        t_assertBoundary(h3, &boundary);
    }
}
