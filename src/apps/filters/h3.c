/*
 * Copyright 2021, 2024 Uber Technologies, Inc.
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
 * @brief cli app that exposes most of the H3 C library for scripting
 *
 *  See `h3 --help` for usage.
 */

#include <string.h>

#ifdef _WIN32

#define strcasecmp _stricmp

#else

#include <strings.h>

#endif

#include "args.h"
#include "h3Index.h"
#include "utility.h"

#define PARSE()                                                             \
    if (parseArgs(argc, argv, sizeof(args) / sizeof(Arg *), args, &helpArg, \
                  args[0]->helpText)) {                                     \
        return E_SUCCESS;                                                   \
    }

bool has(char *subcommand, int level, char *argv[]) {
    return strcasecmp(subcommand, argv[level]) == 0;
}

Arg helpArg = ARG_HELP;
Arg cellToLatLngArg = {
    .names = {"cellToLatLng"},
    .helpText = "Convert an H3 cell to a WKT POINT coordinate",
};
Arg latLngToCellArg = {
    .names = {"latLngToCell"},
    .helpText = "Convert degrees latitude/longitude coordinate to an H3 cell.",
};
Arg cellToBoundaryArg = {
    .names = {"cellToBoundary"},
    .helpText = "Convert an H3 cell to a WKT POLYGON defining its boundary",
};

H3Error cellToLatLngCmd(int argc, char *argv[]) {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellToLatLngArg, &helpArg, &cellArg};
    PARSE();
    LatLng ll;
    H3Error err = H3_EXPORT(cellToLatLng)(cell, &ll);
    if (err) {
        return err;
    }
    // Using WKT formatting for the output. TODO: Add support for JSON
    // formatting
    printf("POINT(%.10lf %.10lf)\n", H3_EXPORT(radsToDegs)(ll.lng),
           H3_EXPORT(radsToDegs)(ll.lat));
    return E_SUCCESS;
}

H3Error latLngToCellCmd(int argc, char *argv[]) {
    int res = 0;
    double lat = 0;
    double lng = 0;

    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg latArg = {.names = {"--lat", "--latitude"},
                  .required = true,
                  .scanFormat = "%lf",
                  .valueName = "lat",
                  .value = &lat,
                  .helpText = "Latitude in degrees."};
    Arg lngArg = {.names = {"--lng", "--longitude"},
                  .required = true,
                  .scanFormat = "%lf",
                  .valueName = "lng",
                  .value = &lng,
                  .helpText = "Longitude in degrees."};

    Arg *args[] = {&latLngToCellArg, &helpArg, &resArg, &latArg, &lngArg};
    PARSE();
    LatLng ll = {.lat = H3_EXPORT(degsToRads)(lat),
                 .lng = H3_EXPORT(degsToRads)(lng)};

    H3Index c;
    H3Error e = H3_EXPORT(latLngToCell)(&ll, res, &c);

    // TODO: Add support for JSON formatting
    if (e == E_SUCCESS) {
        h3Println(c);
    } else {
        h3Println(
            H3_NULL);  // TODO: Should we print a better error message here?
    }
    return e;
}

H3Error cellToBoundaryCmd(int argc, char *argv[]) {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellToBoundaryArg, &helpArg, &cellArg};
    PARSE();
    CellBoundary cb;
    H3Error err = H3_EXPORT(cellToBoundary)(cell, &cb);
    if (err) {
        return err;
    }
    // Using WKT formatting for the output. TODO: Add support for JSON
    // formatting
    printf("POLYGON((");
    for (int i = 0; i < cb.numVerts; i++) {
        LatLng *ll = &cb.verts[i];
        printf("%.10lf %.10lf, ", H3_EXPORT(radsToDegs)(ll->lng),
               H3_EXPORT(radsToDegs)(ll->lat));
    }
    // WKT has the first and last points match, so re-print the first one
    printf("%.10lf %.10lf))\n", H3_EXPORT(radsToDegs)(cb.verts[0].lng),
           H3_EXPORT(radsToDegs)(cb.verts[0].lat));
    return E_SUCCESS;
}

bool generalHelp(int argc, char *argv[]) {
    Arg *args[] = {&helpArg, &cellToLatLngArg, &latLngToCellArg,
                   &cellToBoundaryArg};

    const char *helpText =
        "Please use one of the subcommands listed to perform an H3 "
        "calculation. Use h3 <SUBCOMMAND> --help for details on the usage of "
        "any subcommand.";
    return parseArgs(argc, argv, sizeof(args) / sizeof(Arg *), args, &helpArg,
                     helpText);
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("Please use h3 --help to see how to use this command.\n");
        return 1;
    }
    if (has("cellToLatLng", 1, argv)) {
        return cellToLatLngCmd(argc, argv);
    }
    if (has("latLngToCell", 1, argv)) {
        return latLngToCellCmd(argc, argv);
    }
    if (has("cellToBoundary", 1, argv)) {
        return cellToBoundaryCmd(argc, argv);
    }
    if (generalHelp(argc, argv)) {
        return 0;
    }
    printf("Please use h3 --help to see how to use this command.\n");
    return 1;
}
