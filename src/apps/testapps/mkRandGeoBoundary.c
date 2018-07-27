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
 * @brief generates random cell indexes and the corresponding cell boundaries
 *
 *  usage: `mkRandGeoBoundary numPoints resolution`
 *
 *  The program generates `numPoints` random lat/lon coordinates and outputs
 *  them along with the corresponding H3Index at the specified `resolution`.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "baseCells.h"
#include "h3Index.h"
#include "utility.h"

void randomGeo(GeoCoord* g) {
    static int init = 0;
    if (!init) {
        srand((unsigned int)time(0));
        init = 1;
    }

    g->lat = H3_EXPORT(degsToRads)(
        (((float)rand() / (float)(RAND_MAX)) * 180.0) - 90.0);
    g->lon = H3_EXPORT(degsToRads)((float)rand() / (float)(RAND_MAX)) * 360.0;
}

int main(int argc, char* argv[]) {
    // check command line args
    if (argc > 3) {
        fprintf(stderr, "usage: %s numPoints resolution\n", argv[0]);
        exit(1);
    }

    int numPoints = 0;
    if (!sscanf(argv[1], "%d", &numPoints))
        error("numPoints must be an integer");

    int res = 0;
    if (!sscanf(argv[2], "%d", &res)) error("resolution must be an integer");

    if (res > MAX_H3_RES) error("specified resolution exceeds max resolution");

    for (int i = 0; i < numPoints; i++) {
        GeoCoord g;
        randomGeo(&g);

        H3Index h = H3_EXPORT(geoToH3)(&g, res);
        GeoBoundary b;
        H3_EXPORT(h3ToGeoBoundary)(h, &b);

        h3Println(h);
        geoBoundaryPrintln(&b);
    }
}
