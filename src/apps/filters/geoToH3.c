/*
 * Copyright 2016-2017, 2019 Uber Technologies, Inc.
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
 *  See `geoToH3 --help` for usage.
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

#include "args.h"
#include "h3Index.h"
#include "utility.h"

/**
 * Convert coordinates to cell and print it.
 *
 * @param lat Degrees latitude
 * @param lon Degrees longitude
 * @param res Resolution
 */
void doCoords(double lat, double lon, int res) {
    GeoCoord g;
    setGeoDegs(&g, lat, lon);

    H3Index h = H3_EXPORT(geoToH3)(&g, res);

    h3Println(h);
}

int main(int argc, char* argv[]) {
    int res = 0;
    double lat = 0;
    double lon = 0;

    Arg helpArg = ARG_HELP;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg latArg = {.names = {"--lat", "--latitude"},
                  .scanFormat = "%lf",
                  .valueName = "lat",
                  .value = &lat,
                  .helpText =
                      "Latitude in degrees. If not specified, \"latitude "
                      "longitude\" pairs will be read from standard input."};
    Arg lonArg = {.names = {"--lon", "--longitude"},
                  .scanFormat = "%lf",
                  .valueName = "lon",
                  .value = &lon,
                  .helpText = "Longitude in degrees."};

    Arg* args[] = {&helpArg, &resArg, &latArg, &lonArg};
    const int numArgs = 4;
    const char* helpText =
        "Convert degrees latitude/longitude coordinates to H3 indexes.";

    if (parseArgs(argc, argv, numArgs, args, &helpArg, helpText)) {
        return helpArg.found ? 0 : 1;
    }

    if (latArg.found != lonArg.found) {
        // One is found but the other is not.
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "Latitude and longitude must both be specified.", NULL);
        return 1;
    }

    if (latArg.found) {
        doCoords(lat, lon, res);
    } else {
        // process the lat/lon's on stdin
        char buff[BUFF_SIZE];
        while (1) {
            // get a lat/lon from stdin
            if (!fgets(buff, BUFF_SIZE, stdin)) {
                if (feof(stdin))
                    break;
                else
                    error("reading lat/lon");
            }

            if (sscanf(buff, "%lf %lf", &lat, &lon) != 2)
                error("parsing lat/lon");

            // convert to H3
            doCoords(lat, lon, res);
        }
    }
}
