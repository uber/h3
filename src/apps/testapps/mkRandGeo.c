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
 * @brief generates random lat/lon pairs and bins them at the specified
 * resolution
 *
 *  See `mkRandGeo --help` for usage.
 *
 *  The program generates `numPoints` random lat/lon coordinates and outputs
 *  them along with the corresponding H3Index at the specified `resolution`.
 */

#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "utility.h"

int main(int argc, char* argv[]) {
    int res = 0;
    int numPoints = 0;

    Arg helpArg = ARG_HELP;
    Arg numPointsArg = {
        .names = {"-n", "--num-points"},
        .required = true,
        .scanFormat = "%d",
        .valueName = "num",
        .value = &numPoints,
        .helpText = "Number of random lat/lon pairs to generate."};
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};

    Arg* args[] = {&helpArg, &numPointsArg, &resArg};
    const int numArgs = 3;
    const char* helpText =
        "Generates random lat/lon pairs and indexes them at the specified "
        "resolution.";

    if (parseArgs(argc, argv, numArgs, args, &helpArg, helpText)) {
        return helpArg.found ? 0 : 1;
    }

    if (res > MAX_H3_RES) {
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "Resolution exceeds maximum resolution.", NULL);
        return 1;
    }

    for (int i = 0; i < numPoints; i++) {
        GeoCoord g;
        randomGeo(&g);

        H3Index h = H3_EXPORT(geoToH3)(&g, res);

        h3Print(h);
        printf(" ");
        geoPrintlnNoFmt(&g);
    }
}
