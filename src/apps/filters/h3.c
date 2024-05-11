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

#define END_SUBCOMMANDS_INDEX                                             \
    {.name = "--help", .arg = &helpArg, .subcommand = generalHelp}, {     \
        .name = "-h", .arg = &helpArg, .subcommand = generalHelp          \
    }                                                                     \
    }                                                                     \
    ;                                                                     \
                                                                          \
    H3Error generalHelp(int argc, char *argv[]) {                         \
        int arglen = sizeof(subcommands) / sizeof(subcommands[0]) - 1;    \
        Arg **args = calloc(arglen, sizeof(Arg *));                       \
        args[0] = &helpArg;                                               \
        for (int i = 0; i < arglen - 1; i++) {                            \
            args[i + 1] = subcommands[i].arg;                             \
        }                                                                 \
                                                                          \
        const char *helpText =                                            \
            "Please use one of the subcommands listed to perform an H3 "  \
            "calculation. Use h3 <SUBCOMMAND> --help for details on the " \
            "usage of "                                                   \
            "any subcommand.";                                            \
        if (parseArgs(argc, argv, arglen, args, &helpArg, helpText)) {    \
            free(args);                                                   \
            return E_SUCCESS;                                             \
        } else {                                                          \
            free(args);                                                   \
            return E_FAILED;                                              \
        }                                                                 \
    }

#define DISPATCH_SUBCOMMAND()                                                \
    for (int i = 0; i < sizeof(subcommands) / sizeof(subcommands[0]); i++) { \
        if (has(subcommands[i].name, 1, argv)) {                             \
            return subcommands[i].subcommand(argc, argv);                    \
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
    err = H3_EXPORT(gridDisk)(cell, k, out);
    if (err) {
        free(out);
        return err;
    }
    printf(
        "[ \"%" PRIx64 "\"",
        out[0]);  // TODO: Theoretically this may be zero, but I know it isn't
    for (int64_t i = 1; i < len; i++) {
        if (out[i] != 0) {
            printf(", \"%" PRIx64 "\"", out[i]);
        }
    }
    free(out);
    printf(" ]");
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
    int *distances = calloc(len, sizeof(int));
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
        int *distances = calloc(templen, sizeof(int));
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
    printf(" ]");
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
    printf(" ]");
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
    printf("%" PRIx64, distance);
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
