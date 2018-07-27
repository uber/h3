/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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
 * @brief stdin/stdout filter that converts from lat/lon coordinates to integer
 * H3 indexes
 *
 *  usage: `geoToH3 resolution`
 *
 *  The program reads lat/lon pairs from stdin until EOF is encountered. For
 *  each lat/lon the program outputs to stdout the integer H3 index of the
 *  containing cell at the specified resolution.
 *
 *  The stdin input should have the following format (lat/lon in decimal
 *  degrees):
 *
 *       lat0 lon0
 *       lat1 lon1
 *       ...
 *       latN lonN
 */

#include <stdio.h>
#include <stdlib.h>
#include "coordijk.h"
#include "h3Index.h"
#include "utility.h"

int main(int argc, char* argv[]) {
    // get the command line argument resolution
    if (argc != 2) {
        fprintf(stderr, "usage: %s resolution\n", argv[0]);
        exit(1);
    }

    int res;
    if (!sscanf(argv[1], "%d", &res)) error("parsing resolution");

    // process the lat/lon's on stdin
    char buff[BUFF_SIZE];
    double lat, lon;
    while (1) {
        // get a lat/lon from stdin
        if (!fgets(buff, BUFF_SIZE, stdin)) {
            if (feof(stdin))
                break;
            else
                error("reading lat/lon");
        }

        if (sscanf(buff, "%lf %lf", &lat, &lon) != 2) error("parsing lat/lon");

        // convert to H3
        GeoCoord g;
        setGeoDegs(&g, lat, lon);

        H3Index h = H3_EXPORT(geoToH3)(&g, res);

        h3Println(h);
    }
}
