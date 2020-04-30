/*
 * Copyright 2016-2018 Uber Technologies, Inc.
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
 * @brief tests H3 function `geoToH3`
 *
 *  usage: `testGeoToH3`
 *
 *  The program reads lines containing H3 indexes and lat/lon  pairs from
 *  stdin until EOF is encountered. For each input line, it calls `geoToH3`
 *  to convert the input lat/lon to an H3 index, and then validates the
 *  index against the original input index.
 */

#include <stdio.h>
#include <stdlib.h>

#include "geoCoord.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

static void assertExpected(H3Index h1, const GeoCoord* g1) {
    // convert lat/lon to H3 and verify
    int res = H3_EXPORT(h3GetResolution)(h1);
    H3Index h2 = H3_EXPORT(geoToH3)(g1, res);
    t_assert(h1 == h2, "got expected geoToH3 output");
}

int main(int argc, char* argv[]) {
    // check command line args
    if (argc > 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(1);
    }

    // process the indexes and lat/lons on stdin
    char buff[BUFF_SIZE];
    char h3Str[BUFF_SIZE];
    while (1) {
        // get an index from stdin
        if (!fgets(buff, BUFF_SIZE, stdin)) {
            if (feof(stdin))
                break;
            else
                error("reading input from stdin");
        }

        double latDegs, lonDegs;
        if (sscanf(buff, "%s %lf %lf", h3Str, &latDegs, &lonDegs) != 3)
            error("parsing input (should be \"H3Index lat lon\")");

        H3Index h3;
        h3 = H3_EXPORT(stringToH3)(h3Str);

        GeoCoord coord;
        setGeoDegs(&coord, latDegs, lonDegs);

        assertExpected(h3, &coord);
    }
}
