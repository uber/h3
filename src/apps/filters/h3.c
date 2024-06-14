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

#include "h3api.h"

#ifdef _WIN32

#define strcasecmp _stricmp

#else

#include <strings.h>

#endif

#include "args.h"
#include "h3Index.h"
#include "utility.h"

#define PARSE_SUBCOMMAND(argc, argv, args)                                  \
    if (parseArgs(argc, argv, sizeof(args) / sizeof(Arg *), args, &helpArg, \
                  args[0]->helpText)) {                                     \
        return E_SUCCESS;                                                   \
    }

#define SUBCOMMAND(name, help)                            \
    Arg name##Arg = {.names = {#name}, .helpText = help}; \
    H3Error name##Cmd(int argc, char *argv[])

struct Subcommand {
    char *name;
    Arg *arg;
    H3Error (*subcommand)(int, char **);
};

#define SUBCOMMANDS_INDEX                        \
    H3Error generalHelp(int argc, char *argv[]); \
    struct Subcommand subcommands[] = {
#define SUBCOMMAND_INDEX(s) {.name = #s, .arg = &s##Arg, .subcommand = &s##Cmd},

#define END_SUBCOMMANDS_INDEX                                                  \
    {.name = "--help", .arg = &helpArg, .subcommand = generalHelp}, {          \
        .name = "-h", .arg = &helpArg, .subcommand = generalHelp               \
    }                                                                          \
    }                                                                          \
    ;                                                                          \
                                                                               \
    H3Error generalHelp(int argc, char *argv[]) {                              \
        int arglen = sizeof(subcommands) / sizeof(subcommands[0]) - 1;         \
        Arg **args = calloc(arglen, sizeof(Arg *));                            \
        if (args == NULL) {                                                    \
            fprintf(stderr, "Failed to allocate memory for argument parsing"); \
            exit(1);                                                           \
        }                                                                      \
        args[0] = &helpArg;                                                    \
        for (int i = 0; i < arglen - 1; i++) {                                 \
            args[i + 1] = subcommands[i].arg;                                  \
        }                                                                      \
                                                                               \
        const char *helpText =                                                 \
            "Please use one of the subcommands listed to perform an H3 "       \
            "calculation. Use h3 <SUBCOMMAND> --help for details on the "      \
            "usage of "                                                        \
            "any subcommand.";                                                 \
        if (parseArgs(argc, argv, arglen, args, &helpArg, helpText)) {         \
            free(args);                                                        \
            return E_SUCCESS;                                                  \
        } else {                                                               \
            free(args);                                                        \
            return E_FAILED;                                                   \
        }                                                                      \
    }

#define DISPATCH_SUBCOMMAND()                                                \
    for (int i = 0; i < sizeof(subcommands) / sizeof(subcommands[0]); i++) { \
        if (has(subcommands[i].name, 1, argv)) {                             \
            H3Error err = subcommands[i].subcommand(argc, argv);             \
            if (err != 0) {                                                  \
                fprintf(stderr, "Error %i: %s\n", err,                       \
                        H3_EXPORT(describeH3Error)(err));                    \
            }                                                                \
            return err;                                                      \
        }                                                                    \
    }

bool has(char *subcommand, int level, char *argv[]) {
    return strcasecmp(subcommand, argv[level]) == 0;
}

Arg helpArg = ARG_HELP;

/// Indexing subcommands

SUBCOMMAND(cellToLatLng, "Convert an H3Cell to a WKT POINT coordinate") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellToLatLngArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    LatLng ll;
    int valid = H3_EXPORT(isValidCell)(cell);
    if (valid == 0) {
        return E_CELL_INVALID;
    }
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

SUBCOMMAND(latLngToCell,
           "Convert degrees latitude/longitude coordinate to an H3 cell") {
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
    PARSE_SUBCOMMAND(argc, argv, args);
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

SUBCOMMAND(cellToBoundary,
           "Convert an H3 cell to a WKT POLYGON defining its boundary") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellToBoundaryArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    CellBoundary cb;
    int valid = H3_EXPORT(isValidCell)(cell);
    if (valid == 0) {
        return E_CELL_INVALID;
    }
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

/// Inspection subcommands

SUBCOMMAND(getResolution, "Extracts the resolution (0 - 15) from the H3 cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getResolutionArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // TODO: Should there be a general `isValidIndex`?
    H3Error cellErr = H3_EXPORT(isValidCell)(cell);
    H3Error edgeErr = H3_EXPORT(isValidDirectedEdge)(cell);
    H3Error vertErr = H3_EXPORT(isValidVertex)(cell);
    if (cellErr && edgeErr && vertErr) {
        return cellErr;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    int res = H3_EXPORT(getResolution)(cell);
    printf("%i", res);
    return E_SUCCESS;
}

SUBCOMMAND(getBaseCellNumber,
           "Extracts the base cell number (0 - 121) from the H3 cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getBaseCellNumberArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // TODO: Should there be a general `isValidIndex`?
    H3Error cellErr = H3_EXPORT(isValidCell)(cell);
    H3Error edgeErr = H3_EXPORT(isValidDirectedEdge)(cell);
    H3Error vertErr = H3_EXPORT(isValidVertex)(cell);
    if (cellErr && edgeErr && vertErr) {
        return cellErr;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    int baseCell = H3_EXPORT(getBaseCellNumber)(cell);
    printf("%i", baseCell);
    return E_SUCCESS;
}

SUBCOMMAND(stringToInt, "Converts an H3 index in string form to integer form") {
    char *rawCell = calloc(16, sizeof(char));
    if (rawCell == NULL) {
        fprintf(stderr, "Failed to allocate memory for the H3 index");
        exit(1);
    }
    Arg rawCellArg = {.names = {"-c", "--cell"},
                      .required = true,
                      .scanFormat = "%s",
                      .valueName = "cell",
                      .value = rawCell,
                      .helpText = "H3 Cell Index"};
    Arg *args[] = {&stringToIntArg, &helpArg, &rawCellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index c;
    H3Error err = H3_EXPORT(stringToH3)(rawCell, &c);
    if (err) {
        free(rawCell);
        return err;
    }
    printf("%" PRIu64, c);
    free(rawCell);
    return E_SUCCESS;
}

SUBCOMMAND(intToString, "Converts an H3 index in int form to string form") {
    H3Index rawCell;
    Arg rawCellArg = {.names = {"-c", "--cell"},
                      .required = true,
                      .scanFormat = "%" PRIu64,
                      .valueName = "cell",
                      .value = &rawCell,
                      .helpText = "H3 Cell Index"};
    Arg *args[] = {&intToStringArg, &helpArg, &rawCellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    h3Println(rawCell);
    return E_SUCCESS;
}

SUBCOMMAND(isValidCell, "Checks if the provided H3 index is actually valid") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isValidCellArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    bool isValid = H3_EXPORT(isValidCell)(cell);
    printf("%s", isValid ? "true" : "false");
    return E_SUCCESS;
}

SUBCOMMAND(isResClassIII,
           "Checks if the provided H3 index has a Class III orientation") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isResClassIIIArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // TODO: Should there be a general `isValidIndex`?
    H3Error cellErr = H3_EXPORT(isValidCell)(cell);
    H3Error edgeErr = H3_EXPORT(isValidDirectedEdge)(cell);
    H3Error vertErr = H3_EXPORT(isValidVertex)(cell);
    if (cellErr && edgeErr && vertErr) {
        return cellErr;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    bool isClassIII = H3_EXPORT(isResClassIII)(cell);
    printf("%s", isClassIII ? "true" : "false");
    return E_SUCCESS;
}

SUBCOMMAND(
    isPentagon,
    "Checks if the provided H3 index is a pentagon instead of a hexagon") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isPentagonArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // TODO: Should there be a general `isValidIndex`?
    H3Error cellErr = H3_EXPORT(isValidCell)(cell);
    H3Error edgeErr = H3_EXPORT(isValidDirectedEdge)(cell);
    H3Error vertErr = H3_EXPORT(isValidVertex)(cell);
    if (cellErr && edgeErr && vertErr) {
        return cellErr;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    bool is = H3_EXPORT(isPentagon)(cell);
    printf("%s", is ? "true" : "false");
    return E_SUCCESS;
}

SUBCOMMAND(getIcosahedronFaces,
           "Returns the icosahedron face numbers (0 - 19) that the H3 index "
           "intersects") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getIcosahedronFacesArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int faceCount;
    H3Error err = H3_EXPORT(maxFaceCount)(cell, &faceCount);
    if (err) {
        return err;
    }
    int *faces = calloc(faceCount, sizeof(int));
    if (faces == NULL) {
        fprintf(stderr, "Failed to allocate memory for the icosahedron faces");
        exit(1);
    }
    err = H3_EXPORT(getIcosahedronFaces)(cell, faces);
    if (err) {
        free(faces);
        return err;
    }
    bool hasPrinted = false;
    for (int i = 0; i < faceCount - 1; i++) {
        if (faces[i] != -1) {
            if (hasPrinted) {
                printf(", ");
            }
            printf("%i", faces[i]);
            hasPrinted = true;
        }
    }
    if (faces[faceCount - 1] != -1) {
        if (hasPrinted) {
            printf(", ");
        }
        printf("%i", faces[faceCount - 1]);
    }
    free(faces);
    return E_SUCCESS;
}

/// Traversal subcommands

SUBCOMMAND(
    gridDisk,
    "Returns a JSON array of a H3 cells within 'k' steps of the origin cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    int k = 0;
    Arg kArg = {.names = {"-k"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "distance",
                .value = &k,
                .helpText = "Maximum grid distance for the output set"};
    Arg *args[] = {&gridDiskArg, &helpArg, &cellArg, &kArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t len = 0;
    H3Error err = H3_EXPORT(maxGridDiskSize)(k, &len);
    if (err) {
        return err;
    }
    H3Index *out = calloc(len, sizeof(H3Index));
    if (out == NULL) {
        fprintf(stderr, "Failed to allocate memory for the output H3 cells");
        exit(1);
    }
    err = H3_EXPORT(gridDisk)(cell, k, out);
    if (err) {
        free(out);
        return err;
    }
    // Since we don't know *actually* how many cells are in the output (usually
    // the max, but sometimes not), we need to do a quick scan to figure out the
    // true length in order to properly serialize to a JSON array
    int64_t trueLen = 0;
    for (int64_t i = 0; i < len; i++) {
        if (out[i] != 0) {
            trueLen++;
        }
    }
    printf("[ ");
    for (int64_t i = 0, j = 0; i < len; i++) {
        if (out[i] != 0) {
            j++;
            printf("\"%" PRIx64 "\"%s", out[i], j == trueLen ? "" : ", ");
        }
    }
    free(out);
    printf(" ]\n");
    return E_SUCCESS;
}

SUBCOMMAND(
    gridDiskDistances,
    "Returns a JSON array of arrays of H3 cells, each array containing cells "
    "'k' steps away from the origin cell, based on the outer array index") {
    DEFINE_CELL_ARG(cell, cellArg);
    int k = 0;
    Arg kArg = {.names = {"-k"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "distance",
                .value = &k,
                .helpText = "Maximum grid distance for the output set"};
    Arg prettyArg = {
        .names = {"-p", "--pretty-print"},
        .required = false,
        .helpText =
            "Determine if the JSON output should be pretty printed or not"};
    Arg *args[] = {&gridDiskDistancesArg, &helpArg, &cellArg, &kArg,
                   &prettyArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    bool pretty = prettyArg.found;
    int64_t len = 0;
    H3Error err = H3_EXPORT(maxGridDiskSize)(k, &len);
    if (err) {
        return err;
    }
    H3Index *out = calloc(len, sizeof(H3Index));
    if (out == NULL) {
        fprintf(stderr, "Failed to allocate memory for the H3 cells");
        exit(1);
    }
    int *distances = calloc(len, sizeof(int));
    if (distances == NULL) {
        fprintf(stderr, "Failed to allocate memory for the distances");
        exit(1);
    }
    err = H3_EXPORT(gridDiskDistances)(cell, k, out, distances);
    if (err) {
        free(out);
        free(distances);
        return err;
    }
    // Man, I wish JSON allowed trailing commas
    printf("[%s", pretty ? "\n" : "");
    for (int i = 0; i <= k; i++) {
        printf("%s[%s", /* prefix */ pretty ? "  " : "",
               /* suffix */ pretty ? "\n" : "");
        // We need to figure out how many cells are in each ring. Because of
        // pentagons, we can't hardwire this, unfortunately
        int count = 0;
        for (int j = 0; j < len; j++) {
            if (distances[j] == i && out[j] != 0) {
                count++;
            }
        }
        // On the second loop, we output cells with a comma except for the last
        // one, which we now know
        int cellNum = 0;
        for (int j = 0; j < len; j++) {
            if (distances[j] == i && out[j] != 0) {
                cellNum++;
                printf("%s\"%" PRIx64 "\"", pretty ? "    " : "", out[j]);
                if (cellNum == count) {
                    if (pretty) {
                        printf("\n");
                    }
                } else {
                    printf(",%s", pretty ? "\n" : "");
                }
            }
        }
        // Similarly, we need to check which iteration of the outer array we're
        // on and include a comma or not
        if (i == k) {
            printf("%s]%s", /* prefix */ pretty ? "  " : "",
                   /* suffix */ pretty ? "\n" : "");
        } else {
            printf("%s],%s", /* prefix */ pretty ? "  " : "",
                   /* suffix */ pretty ? "\n" : "");
        }
    }
    printf("]\n");  // Always print the newline here so the terminal prompt gets
                    // its own line
    free(out);
    free(distances);
    return E_SUCCESS;
}

SUBCOMMAND(gridRing,
           "Returns a JSON array of H3 cells, each cell 'k' steps away from "
           "the origin cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    int k = 0;
    Arg kArg = {.names = {"-k"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "distance",
                .value = &k,
                .helpText = "Maximum grid distance for the output set"};
    Arg *args[] = {&gridRingArg, &helpArg, &cellArg, &kArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t len = k == 0 ? 1 : 6 * k;  // The length is fixed for gridRingUnsafe
                                       // since it doesn't support pentagons
    H3Index *out = calloc(len, sizeof(H3Index));
    if (out == NULL) {
        fprintf(stderr, "Failed to allocate memory for the output H3 indexes");
        exit(1);
    }
    H3Error err = H3_EXPORT(gridRingUnsafe)(cell, k, out);
    if (err) {
        // For the CLI, we'll just do things less efficiently if there's an
        // error here. If you use `gridDiskDistances` and only pay attention to
        // the last array, it's equivalent to a "safe" gridRing call, but
        // consumes a lot more temporary memory to do it
        int64_t templen = 0;
        err = H3_EXPORT(maxGridDiskSize)(k, &templen);
        if (err) {
            // But we abort if anything fails in here
            free(out);
            return err;
        }
        H3Index *temp = calloc(templen, sizeof(H3Index));
        if (temp == NULL) {
            fprintf(stderr,
                    "Failed to allocate memory for a temporary hashset of H3 "
                    "indexes");
            exit(1);
        }
        int *distances = calloc(templen, sizeof(int));
        if (distances == NULL) {
            fprintf(stderr,
                    "Failed to allocate memory for the distances of the H3 "
                    "indexes");
            exit(1);
        }
        err = H3_EXPORT(gridDiskDistances)(cell, k, temp, distances);
        if (err) {
            free(out);
            free(temp);
            free(distances);
            return err;
        }
        // Now, we first re-zero the `out` array in case there's garbage
        // anywhere in it from the failed computation. Then we scan through the
        // gridDisk output and copy the indexes that are the correct distance
        // in. We *should* only be in this path when there's a pentagon
        // involved, so we expect the true length of the array to be less than
        // what was allocated for `out` in this scenario.
        for (int i = 0; i < len; i++) {
            out[i] = 0;
        }
        int64_t count = 0;
        for (int64_t i = 0; i < templen; i++) {
            if (distances[i] == k && temp[i] != 0) {
                out[count] = temp[i];
                count++;
            }
        }
        len = count;
        free(temp);
        free(distances);
    }
    // Now that we have the correct data, however we got it, we can print it out
    printf("[ \"%" PRIx64 "\"", out[0]);
    for (int64_t i = 1; i < len; i++) {
        if (out[i] != 0) {
            printf(", \"%" PRIx64 "\"", out[i]);
        }
    }
    free(out);
    printf(" ]\n");
    return E_SUCCESS;
}

SUBCOMMAND(gridPathCells,
           "Returns a JSON array of H3 cells from the origin cell to the "
           "destination cell (inclusive)") {
    H3Index origin = 0;
    Arg originArg = {.names = {"-o", "--origin"},
                     .required = true,
                     .scanFormat = "%" PRIx64,
                     .valueName = "cell",
                     .value = &origin,
                     .helpText = "The origin H3 cell"};
    H3Index destination = 0;
    Arg destinationArg = {.names = {"-d", "--destination"},
                          .required = true,
                          .scanFormat = "%" PRIx64,
                          .valueName = "cell",
                          .value = &destination,
                          .helpText = "The destination H3 cell"};
    Arg *args[] = {&gridPathCellsArg, &helpArg, &originArg, &destinationArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t len = 0;
    H3Error err = H3_EXPORT(gridPathCellsSize)(origin, destination, &len);
    if (err) {
        return err;
    }
    H3Index *out = calloc(len, sizeof(H3Index));
    if (out == NULL) {
        fprintf(stderr, "Failed to allocate memory for the output H3 indexes");
        exit(1);
    }
    err = H3_EXPORT(gridPathCells)(origin, destination, out);
    if (err) {
        free(out);
        return err;
    }
    printf("[ \"%" PRIx64 "\"", out[0]);
    for (int64_t i = 1; i < len; i++) {
        if (out[i] != 0) {
            printf(", \"%" PRIx64 "\"", out[i]);
        }
    }
    free(out);
    printf(" ]\n");
    return E_SUCCESS;
}

SUBCOMMAND(gridDistance,
           "Returns the number of steps along the grid to move from the origin "
           "cell to the destination cell") {
    H3Index origin = 0;
    Arg originArg = {.names = {"-o", "--origin"},
                     .required = true,
                     .scanFormat = "%" PRIx64,
                     .valueName = "cell",
                     .value = &origin,
                     .helpText = "The origin H3 cell"};
    H3Index destination = 0;
    Arg destinationArg = {.names = {"-d", "--destination"},
                          .required = true,
                          .scanFormat = "%" PRIx64,
                          .valueName = "cell",
                          .value = &destination,
                          .helpText = "The destination H3 cell"};
    Arg *args[] = {&gridDistanceArg, &helpArg, &originArg, &destinationArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t distance = 0;
    H3Error err = H3_EXPORT(gridDistance)(origin, destination, &distance);
    if (err) {
        return err;
    }
    printf("%" PRIx64 "\n", distance);
    return E_SUCCESS;
}

SUBCOMMAND(cellToLocalIj,
           "Returns the IJ coordinate for a cell anchored to an origin cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    H3Index origin = 0;
    Arg originArg = {.names = {"-o", "--origin"},
                     .required = true,
                     .scanFormat = "%" PRIx64,
                     .valueName = "cell",
                     .value = &origin,
                     .helpText = "The origin H3 cell"};
    Arg *args[] = {&cellToLocalIjArg, &helpArg, &cellArg, &originArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    CoordIJ out = {0};
    H3Error err = H3_EXPORT(cellToLocalIj)(origin, cell, 0, &out);
    if (err) {
        return err;
    }
    printf("[%i, %i]\n", out.i, out.j);
    return E_SUCCESS;
}

SUBCOMMAND(localIjToCell,
           "Returns the H3 index from a local IJ coordinate anchored to an "
           "origin cell") {
    H3Index origin = 0;
    Arg originArg = {.names = {"-o", "--origin"},
                     .required = true,
                     .scanFormat = "%" PRIx64,
                     .valueName = "cell",
                     .value = &origin,
                     .helpText = "The origin H3 cell"};
    int i, j;
    Arg iArg = {.names = {"-i"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "i",
                .value = &i,
                .helpText = "The I dimension of the IJ coordinate"};
    Arg jArg = {.names = {"-j"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "j",
                .value = &j,
                .helpText = "The J dimension of the IJ coordinate"};
    Arg *args[] = {&localIjToCellArg, &helpArg, &originArg, &iArg, &jArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    CoordIJ in = {.i = i, .j = j};
    H3Index out = 0;
    H3Error err = H3_EXPORT(localIjToCell)(origin, &in, 0, &out);
    if (err) {
        return err;
    }
    h3Println(out);
    return E_SUCCESS;
}

/// Hierarchical subcommands

SUBCOMMAND(cellToParent,
           "Returns the H3 index that is the parent (or higher) of the "
           "provided cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 0-14 inclusive, excluding 15 as it can "
                      "never be a parent"};
    Arg *args[] = {&cellToParentArg, &helpArg, &cellArg, &resArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index parent;
    int valid = H3_EXPORT(isValidCell)(cell);
    if (valid == 0) {
        return E_CELL_INVALID;
    }
    H3Error err = H3_EXPORT(cellToParent)(cell, res, &parent);
    if (err) {
        return err;
    }
    h3Println(parent);
    return E_SUCCESS;
}

SUBCOMMAND(cellToChildren,
           "Returns a JSON array of H3 indexes that are the children (or "
           "lower) of the provided cell") {
    // TODO: This function contains a lot of code that is very similar to
    // `gridDisk`. If this happens again we should DRY them.
    DEFINE_CELL_ARG(cell, cellArg);
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 1-15 inclusive, excluding 0 as it can "
                      "never be a child"};
    Arg *args[] = {&cellToChildrenArg, &helpArg, &cellArg, &resArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t len = 0;
    H3Error err = H3_EXPORT(cellToChildrenSize)(cell, res, &len);
    if (err) {
        return err;
    }
    H3Index *out = calloc(len, sizeof(H3Index));
    if (out == NULL) {
        fprintf(stderr, "Failed to allocate memory for the output H3 cells");
        exit(1);
    }
    err = H3_EXPORT(cellToChildren)(cell, res, out);
    if (err) {
        free(out);
        return err;
    }
    // Since we don't know *actually* how many cells are in the output (usually
    // the max, but sometimes not), we need to do a quick scan to figure out the
    // true length in order to properly serialize to a JSON array
    int64_t trueLen = 0;
    for (int64_t i = 0; i < len; i++) {
        if (out[i] != 0) {
            trueLen++;
        }
    }
    printf("[ ");
    for (int64_t i = 0, j = 0; i < len; i++) {
        if (out[i] != 0) {
            j++;
            printf("\"%" PRIx64 "\"%s", out[i], j == trueLen ? "" : ", ");
        }
    }
    free(out);
    printf(" ]\n");
    return E_SUCCESS;
}

SUBCOMMAND(cellToChildrenSize,
           "Returns the maximum number of children for a given cell at the "
           "specified child resolution") {
    // TODO: Do we want to include this subcommand or no? It can be useful to
    // let someone decide for themselves if they really want to run the command
    // and get a potentially massive number of cells as the output, but is that
    // a concern a CLI user would have? They'd probably just ^C it.
    DEFINE_CELL_ARG(cell, cellArg);
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 1-15 inclusive, excluding 0 as it can "
                      "never be a child"};
    Arg *args[] = {&cellToChildrenSizeArg, &helpArg, &cellArg, &resArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t len = 0;
    H3Error err = H3_EXPORT(cellToChildrenSize)(cell, res, &len);
    if (err) {
        return err;
    }
    printf("%" PRId64, len);
    return E_SUCCESS;
}

SUBCOMMAND(
    cellToCenterChild,
    "Returns the H3 index that is the centrally-placed child (or lower) of the "
    "provided cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 1-15 inclusive, excluding 0 as it can "
                      "never be a child"};
    Arg *args[] = {&cellToCenterChildArg, &helpArg, &cellArg, &resArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index centerChild;
    int valid = H3_EXPORT(isValidCell)(cell);
    if (valid == 0) {
        return E_CELL_INVALID;
    }
    H3Error err = H3_EXPORT(cellToCenterChild)(cell, res, &centerChild);
    if (err) {
        return err;
    }
    h3Println(centerChild);
    return E_SUCCESS;
}

SUBCOMMAND(
    cellToChildPos,
    "Returns the position of the child cell within an ordered list of all "
    "children of the cell's parent at the specified child resolution") {
    DEFINE_CELL_ARG(cell, cellArg);
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 1-15 inclusive, excluding 0 as it can "
                      "never be a child"};
    Arg *args[] = {&cellToChildPosArg, &helpArg, &cellArg, &resArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t len = 0;
    H3Error err = H3_EXPORT(cellToChildPos)(cell, res, &len);
    if (err) {
        return err;
    }
    printf("%" PRId64, len);
    return E_SUCCESS;
}

SUBCOMMAND(childPosToCell,
           "Returns the child cell at a given position and resolution within "
           "an ordered list of all children of the parent cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 1-15 inclusive, excluding 0 as it can "
                      "never be a child"};
    int64_t pos;
    Arg posArg = {
        .names = {"-p", "--position"},
        .required = true,
        .scanFormat = "%" PRIi64,
        .valueName = "pos",
        .value = &pos,
        .helpText =
            "The child position within the set of children of the parent cell"};
    Arg *args[] = {&childPosToCellArg, &helpArg, &cellArg, &resArg, &posArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index child;
    int valid = H3_EXPORT(isValidCell)(cell);
    if (valid == 0) {
        return E_CELL_INVALID;
    }
    H3Error err = H3_EXPORT(childPosToCell)(pos, cell, res, &child);
    if (err) {
        return err;
    }
    h3Println(child);
    return E_SUCCESS;
}

SUBCOMMAND(compactCells,
           "Compacts the provided set of cells as best as possible. The set of "
           "input cells must all share the same resolution. The compacted "
           "cells will be printed one per line to stdout.") {
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {
        .names = {"-f", "--file"},
        .scanFormat = "%1023c",
        .valueName = "FILENAME",
        .value = &filename,
        .helpText =
            "The file to load the cells from. Use -- to read from stdin."};
    char cellStrs[1501] = {
        0};  // Supports up to 100 cells at a time with zero padding
    Arg cellStrsArg = {.names = {"-c", "--cells"},
                       .scanFormat = "%1500c",
                       .valueName = "CELLS",
                       .value = &cellStrs,
                       .helpText =
                           "The cells to compact. Up to 100 cells if provided "
                           "as hexadecimals with zero padding."};
    Arg *args[] = {&compactCellsArg, &helpArg, &filenameArg, &cellStrsArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!filenameArg.found && !cellStrsArg.found) {
        fprintf(stderr,
                "You must provide either a file to read from or a set of cells "
                "to compact to use compactCells");
        exit(1);
    }
    // We use the same consumption logic for both kinds of input, and in fact
    // use the cellStrs char array as out input buffer for the file path. The
    // only difference between the two is when we reach the end of the buffer.
    // If there's a non-null file pointer, we start from the last successful
    // consumption from the buffer and move the data following it to the
    // beginning of the buffer, then we read some of the file after that and
    // slap it on after that and continue the consumption loops, while the other
    // path just ends at that point. As we have no idea how many cells we're
    // going to load, we allocate enough for 128 cells, but if that's not
    // enough, we also have to re-allocate double the number of cells, copy them
    // over, and free the old buffer of cells. Doing this manually since we want
    // to keep the build process for H3 simple and C's stdlib is pretty bare.
    H3Index *cells = calloc(128, sizeof(H3Index));
    FILE *fp = 0;
    int cellStrsOffset = 0;
    int cellsOffset = 0;
    int cellsLen = 128;
    bool isStdin = false;
    if (filenameArg.found) {
        if (strcmp(filename, "--") == 0) {
            fp = stdin;
            isStdin = true;
        } else {
            fp = fopen(filename, "r");
        }
        if (fp == 0) {
            fprintf(stderr, "The specified file does not exist.");
            exit(1);
        }
        // Do the initial population of data from the file
        if (fread(cellStrs, 1, 1500, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    do {
        // Always start from the beginning of the buffer
        cellStrsOffset = 0;
        int lastGoodOffset = 0;
        while (cellStrsOffset < 1485) {  // Keep consuming as much as possible
            H3Index cell = 0;
            while (cell == 0 && cellStrsOffset < 1485) {
                // Grab 15 characters and then scan to see if it has an H3 index
                // in it
                char possibleIndex[16] = {0};
                bool badChar = false;
                for (int i = 0; i < 15; i++) {
                    char c = cellStrs[i + cellStrsOffset];
                    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                        (c >= 'A' && c <= 'F')) {
                        possibleIndex[i] = c;
                    } else {
                        // Encountered a bad character, set the offset after
                        // this character as the next to check
                        cellStrsOffset += i + 1;
                        badChar = true;
                        break;
                    }
                    possibleIndex[i] = cellStrs[i + cellStrsOffset];
                }
                if (!badChar) {
                    if (sscanf(possibleIndex, "%" PRIx64, &cell) != 0) {
                        // We can jump ahead 15 chars. The while loop check will
                        // make sure we don't overwrite this cell with another
                        // one
                        cellStrsOffset += 15;
                        lastGoodOffset = cellStrsOffset;
                    } else {
                        // This *shouldn't* happen, I think? But try again if
                        // necessary
                        cellStrsOffset++;
                    }
                }
            }
            // If we still don't have a cell and we've reached the end, we reset
            // the offset and `continue` to trigger another read
            if (cell == 0 && cellStrsOffset == 1500) {
                cellStrsOffset = 0;
                continue;
            }
            // Otherwise, we have a cell to shove into the cells array.
            cells[cellsOffset] = cell;
            cellsOffset++;
            // Potentially grow our array
            if (cellsOffset == cellsLen) {
                cellsLen *= 2;
                H3Index *newCells = calloc(cellsLen, sizeof(H3Index));
                for (int i = 0; i < cellsOffset; i++) {
                    newCells[i] = cells[i];
                }
                free(cells);
                cells = newCells;
            }
        }
        // In case there's a valid H3 index that was unfortunately split between
        // buffer reads, we take from the lastGoodOffset and copy the rest to
        // the beginning of the buffer so it can be re-assembled on the next
        // file read. However, we also know that a valid H3 index is 15
        // characters long, so if the lastGoodOffset is 15 or more characters
        // away from 1500, we only need to copy those final 14 bytes from the
        // end, so if lastGoodOffset is 1485 or less, we force it to 1485 and
        // then move the chunk as specified to the beginning and adjust the
        // cellStrsOffset.
        if (lastGoodOffset < 1485) {
            lastGoodOffset = 1485;
        }
        for (int i = 0; i < 1500 - lastGoodOffset; i++) {
            cellStrs[i] = cellStrs[i + lastGoodOffset];
        }
        cellStrsOffset = 1500 - lastGoodOffset;
    } while (fp != 0 && fread(cellStrs + cellStrsOffset, 1,
                              1500 - cellStrsOffset, fp) != 0);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    // Now that we have the cells in a buffer and the actual cell count in
    // cellsOffset, we can feed this to the H3 C API
    H3Index *compactedSet = calloc(cellsOffset, sizeof(H3Index));
    H3Error err = H3_EXPORT(compactCells)(cells, compactedSet, cellsOffset);
    if (err) {
        free(cells);
        free(compactedSet);
        return err;
    }
    // We have a compacted set! Let's print them out
    for (int i = 0; i < cellsOffset; i++) {
        if (compactedSet[i] == 0) {
            continue;
        }
        h3Println(compactedSet[i]);
    }
    free(cells);
    free(compactedSet);
    return E_SUCCESS;
}

SUBCOMMAND(uncompactCells,
           "Unompacts the provided set of compacted cells."
           "The uncompacted "
           "cells will be printed one per line to stdout.") {
    // TODO: *Most* of this logic is shared with compactCells. See about DRYing
    // it.
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {
        .names = {"-f", "--file"},
        .scanFormat = "%1023c",
        .valueName = "FILENAME",
        .value = &filename,
        .helpText =
            "The file to load the cells from. Use -- to read from stdin."};
    char cellStrs[1501] = {
        0};  // Supports up to 100 cells at a time with zero padding
    Arg cellStrsArg = {
        .names = {"-c", "--cells"},
        .scanFormat = "%1500c",
        .valueName = "CELLS",
        .value = &cellStrs,
        .helpText =
            "The cells to uncompact. Up to 100 cells if provided "
            "as hexadecimals with zero padding."};
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 0-15 inclusive, that the compacted set "
                      "should be uncompacted to. Must be greater than or equal "
                      "to the highest resolution within the compacted set."};
    Arg *args[] = {&uncompactCellsArg, &helpArg, &filenameArg, &cellStrsArg,
                   &resArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!filenameArg.found && !cellStrsArg.found) {
        fprintf(stderr,
                "You must provide either a file to read from or a set of cells "
                "to compact to use uncompactCells");
        exit(1);
    }
    // We use the same consumption logic for both kinds of input, and in fact
    // use the cellStrs char array as out input buffer for the file path. The
    // only difference between the two is when we reach the end of the buffer.
    // If there's a non-null file pointer, we start from the last successful
    // consumption from the buffer and move the data following it to the
    // beginning of the buffer, then we read some of the file after that and
    // slap it on after that and continue the consumption loops, while the other
    // path just ends at that point. As we have no idea how many cells we're
    // going to load, we allocate enough for 128 cells, but if that's not
    // enough, we also have to re-allocate double the number of cells, copy them
    // over, and free the old buffer of cells. Doing this manually since we want
    // to keep the build process for H3 simple and C's stdlib is pretty bare.
    H3Index *cells = calloc(128, sizeof(H3Index));
    FILE *fp = 0;
    int cellStrsOffset = 0;
    int cellsOffset = 0;
    int cellsLen = 128;
    bool isStdin = false;
    if (filenameArg.found) {
        if (strcmp(filename, "--") == 0) {
            fp = stdin;
            isStdin = true;
        } else {
            fp = fopen(filename, "r");
        }
        if (fp == 0) {
            fprintf(stderr, "The specified file does not exist.");
            exit(1);
        }
        // Do the initial population of data from the file
        if (fread(cellStrs, 1, 1500, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    do {
        // Always start from the beginning of the buffer
        cellStrsOffset = 0;
        int lastGoodOffset = 0;
        while (cellStrsOffset < 1485) {  // Keep consuming as much as possible
            H3Index cell = 0;
            while (cell == 0 && cellStrsOffset < 1485) {
                // Grab 15 characters and then scan to see if it has an H3 index
                // in it
                char possibleIndex[16] = {0};
                bool badChar = false;
                for (int i = 0; i < 15 && !badChar; i++) {
                    char c = cellStrs[i + cellStrsOffset];
                    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                        (c >= 'A' && c <= 'F')) {
                        possibleIndex[i] = c;
                    } else {
                        // Encountered a bad character, set the offset after
                        // this character as the next to check
                        cellStrsOffset += i + 1;
                        badChar = true;
                    }
                    possibleIndex[i] = cellStrs[i + cellStrsOffset];
                }
                if (!badChar) {
                    if (sscanf(possibleIndex, "%" PRIx64, &cell) != 0) {
                        // We can jump ahead 15 chars. The while loop check will
                        // make sure we don't overwrite this cell with another
                        // one
                        cellStrsOffset += 15;
                        lastGoodOffset = cellStrsOffset;
                    }
                } else {
                    cell = 0;
                }
            }
            // If we still don't have a cell and we've reached the end, we reset
            // the offset and `continue` to trigger another read
            if (cell == 0 && cellStrsOffset == 1500) {
                cellStrsOffset = 0;
                continue;
            }
            // Otherwise, we have a cell to shove into the cells array.
            cells[cellsOffset] = cell;
            cellsOffset++;
            // Potentially grow our array
            if (cellsOffset == cellsLen) {
                cellsLen *= 2;
                H3Index *newCells = calloc(cellsLen, sizeof(H3Index));
                for (int i = 0; i < cellsOffset; i++) {
                    newCells[i] = cells[i];
                }
                free(cells);
                cells = newCells;
            }
        }
        // In case there's a valid H3 index that was unfortunately split between
        // buffer reads, we take from the lastGoodOffset and copy the rest to
        // the beginning of the buffer so it can be re-assembled on the next
        // file read. However, we also know that a valid H3 index is 15
        // characters long, so if the lastGoodOffset is 15 or more characters
        // away from 1500, we only need to copy those final 14 bytes from the
        // end, so if lastGoodOffset is 1485 or less, we force it to 1485 and
        // then move the chunk as specified to the beginning and adjust the
        // cellStrsOffset.
        if (lastGoodOffset < 1485) {
            lastGoodOffset = 1485;
        }
        for (int i = 0; i < 1500 - lastGoodOffset; i++) {
            cellStrs[i] = cellStrs[i + lastGoodOffset];
        }
        cellStrsOffset = 1500 - lastGoodOffset;
    } while (fp != 0 && fread(cellStrs + cellStrsOffset, 1,
                              1500 - cellStrsOffset, fp) != 0);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    // Now that we have the cells in a buffer and the actual cell count in
    // cellsOffset, we can feed this to the H3 C API
    int64_t uncompactedSize = 0;
    H3Error err = H3_EXPORT(uncompactCellsSize)(cells, cellsOffset, res,
                                                &uncompactedSize);
    if (err) {
        free(cells);
        return err;
    }
    H3Index *uncompactedSet = calloc(uncompactedSize, sizeof(H3Index));
    err = H3_EXPORT(uncompactCells)(cells, cellsOffset, uncompactedSet,
                                    uncompactedSize, res);
    if (err) {
        free(cells);
        free(uncompactedSet);
        return err;
    }
    // We have a compacted set! Let's print them out
    for (int i = 0; i < uncompactedSize; i++) {
        if (uncompactedSet[i] == 0) {
            continue;
        }
        h3Println(uncompactedSet[i]);
    }
    free(cells);
    free(uncompactedSet);
    return E_SUCCESS;
}

// TODO: Is there any way to avoid this particular piece of duplication?
SUBCOMMANDS_INDEX

/// Indexing subcommands
SUBCOMMAND_INDEX(cellToLatLng)
SUBCOMMAND_INDEX(latLngToCell)
SUBCOMMAND_INDEX(cellToBoundary)

/// Inspection subcommands
SUBCOMMAND_INDEX(getResolution)
SUBCOMMAND_INDEX(getBaseCellNumber)
SUBCOMMAND_INDEX(stringToInt)
SUBCOMMAND_INDEX(intToString)
SUBCOMMAND_INDEX(isValidCell)
SUBCOMMAND_INDEX(isResClassIII)
SUBCOMMAND_INDEX(isPentagon)
SUBCOMMAND_INDEX(getIcosahedronFaces)

/// Traversal subcommands
SUBCOMMAND_INDEX(gridDisk)
SUBCOMMAND_INDEX(gridDiskDistances)
SUBCOMMAND_INDEX(gridRing)
SUBCOMMAND_INDEX(gridPathCells)
SUBCOMMAND_INDEX(gridDistance)
SUBCOMMAND_INDEX(cellToLocalIj)
SUBCOMMAND_INDEX(localIjToCell)

/// Hierarchical subcommands
SUBCOMMAND_INDEX(cellToParent)
SUBCOMMAND_INDEX(cellToChildren)
SUBCOMMAND_INDEX(cellToChildrenSize)
SUBCOMMAND_INDEX(cellToCenterChild)
SUBCOMMAND_INDEX(cellToChildPos)
SUBCOMMAND_INDEX(childPosToCell)
SUBCOMMAND_INDEX(compactCells)
SUBCOMMAND_INDEX(uncompactCells)

END_SUBCOMMANDS_INDEX

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("Please use h3 --help to see how to use this command.\n");
        return 1;
    }
    DISPATCH_SUBCOMMAND();
    printf("Please use h3 --help to see how to use this command.\n");
    return 1;
}
