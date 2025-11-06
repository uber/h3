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

#define BUFFER_SIZE 1500
#define BUFFER_SIZE_LESS_CELL 1485
#define BUFFER_SIZE_WITH_NULL 1501

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

#define DEFINE_FORMAT_ARG(desc)                   \
    char format[8] = {0};                         \
    Arg formatArg = {.names = {"-f", "--format"}, \
                     .scanFormat = "%7s",         \
                     .value = format,             \
                     .valueName = "FMT",          \
                     .helpText = desc}

/// Indexing subcommands

SUBCOMMAND(cellToLatLng, "Convert an H3Cell to a coordinate") {
    DEFINE_FORMAT_ARG(
        "'json' for [lat, lng], 'wkt' for a WKT POINT, 'newline' for "
        "lat\\nlng\\n (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellToLatLngArg, &helpArg, &cellArg, &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[%.10lf, %.10lf]\n", H3_EXPORT(radsToDegs)(ll.lat),
               H3_EXPORT(radsToDegs)(ll.lng));
    } else if (strcmp(format, "wkt") == 0) {
        // Using WKT formatting for the output. TODO: Add support for JSON
        // formatting
        printf("POINT(%.10lf %.10lf)\n", H3_EXPORT(radsToDegs)(ll.lng),
               H3_EXPORT(radsToDegs)(ll.lat));
    } else if (strcmp(format, "newline") == 0) {
        printf("%.10lf\n%.10lf\n", H3_EXPORT(radsToDegs)(ll.lat),
               H3_EXPORT(radsToDegs)(ll.lng));
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(latLngToCell,
           "Convert degrees latitude/longitude coordinate to an H3 cell") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n "
        "(Default: json)");
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

    Arg *args[] = {&latLngToCellArg, &helpArg, &resArg,
                   &latArg,          &lngArg,  &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    LatLng ll = {.lat = H3_EXPORT(degsToRads)(lat),
                 .lng = H3_EXPORT(degsToRads)(lng)};

    H3Index c;
    H3Error err = H3_EXPORT(latLngToCell)(&ll, res, &c);
    if (err) {
        return err;
    }

    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", c);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(c);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(cellToBoundary,
           "Convert an H3 cell to a polygon defining its boundary") {
    DEFINE_FORMAT_ARG(
        "'json' for [[lat, lng], ...], 'wkt' for a WKT POLYGON, 'newline' for "
        "lat\\nlng\\n... (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellToBoundaryArg, &helpArg, &cellArg, &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[");
        for (int i = 0; i < cb.numVerts - 1; i++) {
            LatLng *ll = &cb.verts[i];
            printf("[%.10lf, %.10lf], ", H3_EXPORT(radsToDegs)(ll->lat),
                   H3_EXPORT(radsToDegs)(ll->lng));
        }
        printf("[%.10lf, %.10lf]]\n",
               H3_EXPORT(radsToDegs)(cb.verts[cb.numVerts - 1].lat),
               H3_EXPORT(radsToDegs)(cb.verts[cb.numVerts - 1].lng));
    } else if (strcmp(format, "wkt") == 0) {
        printf("POLYGON((");
        for (int i = 0; i < cb.numVerts; i++) {
            LatLng *ll = &cb.verts[i];
            printf("%.10lf %.10lf, ", H3_EXPORT(radsToDegs)(ll->lng),
                   H3_EXPORT(radsToDegs)(ll->lat));
        }
        // WKT has the first and last points match, so re-print the first one
        printf("%.10lf %.10lf))\n", H3_EXPORT(radsToDegs)(cb.verts[0].lng),
               H3_EXPORT(radsToDegs)(cb.verts[0].lat));
    } else if (strcmp(format, "newline") == 0) {
        for (int i = 0; i < cb.numVerts; i++) {
            LatLng *ll = &cb.verts[i];
            printf("%.10lf\n%.10lf\n", H3_EXPORT(radsToDegs)(ll->lat),
                   H3_EXPORT(radsToDegs)(ll->lng));
        }
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

/// Inspection subcommands

SUBCOMMAND(getResolution, "Extracts the resolution (0 - 15) from the H3 cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getResolutionArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!H3_EXPORT(isValidIndex)(cell)) {
        return E_INDEX_INVALID;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    int res = H3_EXPORT(getResolution)(cell);
    printf("%i\n", res);
    return E_SUCCESS;
}

SUBCOMMAND(getBaseCellNumber,
           "Extracts the base cell number (0 - 121) from the H3 cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getBaseCellNumberArg, &helpArg, &cellArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!H3_EXPORT(isValidIndex)(cell)) {
        return E_INDEX_INVALID;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    int baseCell = H3_EXPORT(getBaseCellNumber)(cell);
    printf("%i\n", baseCell);
    return E_SUCCESS;
}

SUBCOMMAND(getIndexDigit,
           "Extracts the indexing digit (0 - 7) from the H3 cell") {
    DEFINE_CELL_ARG(cell, cellArg);
    int res = 0;
    Arg digitArg = {.names = {"-r", "--res"},
                    .required = true,
                    .scanFormat = "%d",
                    .valueName = "res",
                    .value = &res,
                    .helpText = "Indexing resolution (1 - 15)"};
    Arg *args[] = {&getIndexDigitArg, &helpArg, &cellArg, &digitArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!H3_EXPORT(isValidIndex)(cell)) {
        return E_INDEX_INVALID;
    }
    int value;
    H3Error err = H3_EXPORT(getIndexDigit)(cell, res, &value);
    if (err) {
        return err;
    }
    printf("%i\n", value);
    return E_SUCCESS;
}

SUBCOMMAND(constructCell,
           "Construct an H3 cell from resolution, base cell, and digits") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n (Default: json)");
    int res = 0;
    int bc = 0;
    char digitsStr[1024] = {0};

    Arg resArg = {
        .names = {"-r", "--resolution"},
        .required = false,
        .scanFormat = "%d",
        .valueName = "res",
        .value = &res,
        .helpText =
            "Resolution, 0-15 inclusive. Inferred from digits if not provided. "
            "If provided, an error will be returned if there is a mismatch "
            "with the number of digits provided."};
    Arg bcArg = {.names = {"-b", "--baseCell"},
                 .required = true,
                 .scanFormat = "%d",
                 .valueName = "base",
                 .value = &bc,
                 .helpText = "Base cell number, 0-121 inclusive."};
    Arg digitsArg = {.names = {"-d", "--digits"},
                     .required = false,
                     .scanFormat = "%1023c",
                     .valueName = "d0,d1,...",
                     .value = &digitsStr,
                     .helpText =
                         "Comma-separated list of child digits (0-6) of length "
                         "resolution."};

    Arg *args[] = {&constructCellArg, &helpArg,  &resArg, &bcArg,
                   &digitsArg,        &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);

    // We expect `res` digits. We add one more to make sure that the user didn't
    // pass more.
    int count = 0;
    int digits[MAX_H3_RES + 1] = {0};

    if (digitsArg.found) {
        char *p = digitsStr;
        while (*p && count < MAX_H3_RES + 1) {
            bool isValid = *p >= '0' && *p <= '6';
            if (!isValid) {
                return E_DIGIT_DOMAIN;
            }

            digits[count++] = *p - '0';
            p++;

            if (!*p) {
                break;
            }

            // We expect each digit to be followed by ','
            if (*p != ',') {
                return E_DIGIT_DOMAIN;
            }

            p++;
        }
    }

    if (!resArg.found) {
        // Infer resolution
        res = count;
    }

    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }

    if (count != res) {
        return E_DIGIT_DOMAIN;
    }

    H3Index out;
    H3Error err =
        H3_EXPORT(constructCell)(res, bc, res > 0 ? digits : NULL, &out);
    if (err) {
        return err;
    }

    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", out);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(out);
    } else {
        return E_FAILED;
    }

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
                      .scanFormat = "%15s",
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
    printf("%" PRIu64 "\n", c);
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
    DEFINE_FORMAT_ARG(
        "'json' for true or false, 'numeric' for 1 or 0 (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isValidCellArg, &helpArg, &cellArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    bool isValid = H3_EXPORT(isValidCell)(cell);
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("%s\n", isValid ? "true" : "false");
    } else if (strcmp(format, "numeric") == 0) {
        printf("%d\n", isValid);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(isResClassIII,
           "Checks if the provided H3 index has a Class III orientation") {
    DEFINE_FORMAT_ARG(
        "'json' for true or false, 'numeric' for 1 or 0 (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isResClassIIIArg, &helpArg, &cellArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!H3_EXPORT(isValidIndex)(cell)) {
        return E_INDEX_INVALID;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    bool isClassIII = H3_EXPORT(isResClassIII)(cell);
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("%s\n", isClassIII ? "true" : "false");
    } else if (strcmp(format, "numeric") == 0) {
        printf("%d\n", isClassIII);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(
    isPentagon,
    "Checks if the provided H3 index is a pentagon instead of a hexagon") {
    DEFINE_FORMAT_ARG(
        "'json' for true or false, 'numeric' for 1 or 0 (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isPentagonArg, &helpArg, &cellArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!H3_EXPORT(isValidIndex)(cell)) {
        return E_INDEX_INVALID;
    }
    // If we got here, we can use `getResolution` safely, as this is one of the
    // few functions that doesn't do any error handling (for some reason? I
    // don't see how this would ever be in a hot loop anywhere.
    bool is = H3_EXPORT(isPentagon)(cell);
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("%s\n", is ? "true" : "false");
    } else if (strcmp(format, "numeric") == 0) {
        printf("%d\n", is);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(getIcosahedronFaces,
           "Returns the icosahedron face numbers (0 - 19) that the H3 index "
           "intersects") {
    DEFINE_FORMAT_ARG(
        "'json' for [faceNum, ...], 'newline' for faceNum\\n... (Default: "
        "json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getIcosahedronFacesArg, &helpArg, &cellArg, &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        bool hasPrinted = false;
        printf("[");
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
        printf("]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int i = 0; i < faceCount; i++) {
            if (faces[i] != -1) {
                printf("%i\n", faces[i]);
            }
        }
    } else {
        free(faces);
        return E_FAILED;
    }
    free(faces);
    return E_SUCCESS;
}

/// Traversal subcommands

SUBCOMMAND(
    gridDisk,
    "Returns an array of a H3 cells within 'k' steps of the origin cell") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    int k = 0;
    Arg kArg = {.names = {"-k"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "distance",
                .value = &k,
                .helpText = "Maximum grid distance for the output set"};
    Arg *args[] = {&gridDiskArg, &helpArg, &cellArg, &kArg, &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
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
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < len; i++) {
            if (out[i] != 0) {
                h3Println(out[i]);
            }
        }
    } else {
        free(out);
        return E_FAILED;
    }

    free(out);
    return E_SUCCESS;
}

SUBCOMMAND(
    gridDiskDistances,
    "Returns an array of arrays of H3 cells, each array containing cells "
    "'k' steps away from the origin cell, based on the outer array index") {
    DEFINE_FORMAT_ARG(
        "'json' for [[\"CELL\", ...], ...], 'newline' for CELL\\n with an "
        "extra newline between rings (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    int k = 0;
    Arg kArg = {.names = {"-k"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "distance",
                .value = &k,
                .helpText = "Maximum grid distance for the output set"};
    Arg *args[] = {&gridDiskDistancesArg, &helpArg, &cellArg, &kArg,
                   &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Man, I wish JSON allowed trailing commas
        printf("[");
        for (int i = 0; i <= k; i++) {
            printf("[");
            // We need to figure out how many cells are in each ring. Because of
            // pentagons, we can't hardwire this, unfortunately
            int count = 0;
            for (int j = 0; j < len; j++) {
                if (distances[j] == i && out[j] != 0) {
                    count++;
                }
            }
            // On the second loop, we output cells with a comma except for the
            // last one, which we now know
            int cellNum = 0;
            for (int j = 0; j < len; j++) {
                if (distances[j] == i && out[j] != 0) {
                    cellNum++;
                    printf("\"%" PRIx64 "\"", out[j]);
                    if (cellNum != count) {
                        printf(", ");
                    }
                }
            }
            // Similarly, we need to check which iteration of the outer array
            // we're on and include a comma or not
            if (i == k) {
                printf("]");
            } else {
                printf("], ");
            }
        }
        printf("]\n");  // Always print the newline here so the terminal prompt
                        // gets its own line
    } else if (strcmp(format, "newline") == 0) {
        for (int i = 0; i <= k; i++) {
            for (int j = 0; j < len; j++) {
                if (distances[j] == i && out[j] != 0) {
                    h3Println(out[j]);
                }
            }
            if (i < k) {
                printf("\n");
            }
        }
    } else {
        free(out);
        free(distances);
        return E_FAILED;
    }
    free(out);
    free(distances);
    return E_SUCCESS;
}

SUBCOMMAND(gridRing,
           "Returns an array of H3 cells, each cell 'k' steps away from "
           "the origin cell") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    int k = 0;
    Arg kArg = {.names = {"-k"},
                .required = true,
                .scanFormat = "%d",
                .valueName = "distance",
                .value = &k,
                .helpText = "Maximum grid distance for the output set"};
    Arg *args[] = {&gridRingArg, &helpArg, &cellArg, &kArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t len;
    H3Error err = H3_EXPORT(maxGridRingSize)(k, &len);
    if (err) {
        fprintf(stderr, "Invalid k");
        exit(1);
    }
    H3Index *out = calloc(len, sizeof(H3Index));
    if (out == NULL) {
        fprintf(stderr, "Failed to allocate memory for the output H3 indexes");
        exit(1);
    }
    err = H3_EXPORT(gridRing)(cell, k, out);
    if (err) {
        free(out);
        return err;
    }
    // Now that we have the correct data, however we got it, we can print it out
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[ \"%" PRIx64 "\"", out[0]);
        for (int64_t i = 1; i < len; i++) {
            if (out[i] != 0) {
                printf(", \"%" PRIx64 "\"", out[i]);
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < len; i++) {
            h3Println(out[i]);
        }
    } else {
        free(out);
        return E_FAILED;
    }
    free(out);
    return E_SUCCESS;
}

SUBCOMMAND(gridPathCells,
           "Returns an array of H3 cells from the origin cell to the "
           "destination cell (inclusive)") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
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
    Arg *args[] = {&gridPathCellsArg, &helpArg, &originArg, &destinationArg,
                   &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[ \"%" PRIx64 "\"", out[0]);
        for (int64_t i = 1; i < len; i++) {
            if (out[i] != 0) {
                printf(", \"%" PRIx64 "\"", out[i]);
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < len; i++) {
            h3Println(out[i]);
        }
    } else {
        free(out);
        return E_FAILED;
    }
    free(out);
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
    DEFINE_FORMAT_ARG(
        "'json' for [I, J], 'newline' for I\\nJ\\n (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    H3Index origin = 0;
    Arg originArg = {.names = {"-o", "--origin"},
                     .required = true,
                     .scanFormat = "%" PRIx64,
                     .valueName = "cell",
                     .value = &origin,
                     .helpText = "The origin H3 cell"};
    Arg *args[] = {&cellToLocalIjArg, &helpArg, &cellArg, &originArg,
                   &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    CoordIJ out = {0};
    H3Error err = H3_EXPORT(cellToLocalIj)(origin, cell, 0, &out);
    if (err) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[%i, %i]\n", out.i, out.j);
    } else if (strcmp(format, "newline") == 0) {
        printf("%i\n%i\n", out.i, out.j);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(localIjToCell,
           "Returns the H3 index from a local IJ coordinate anchored to an "
           "origin cell") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n (Default: json)");
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
    Arg *args[] = {&localIjToCellArg, &helpArg, &originArg, &iArg, &jArg,
                   &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    CoordIJ in = {.i = i, .j = j};
    H3Index out = 0;
    H3Error err = H3_EXPORT(localIjToCell)(origin, &in, 0, &out);
    if (err) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", out);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(out);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

/// Hierarchical subcommands

SUBCOMMAND(cellToParent,
           "Returns the H3 index that is the parent (or higher) of the "
           "provided cell") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n (Default: json)");
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
    Arg *args[] = {&cellToParentArg, &helpArg, &cellArg, &resArg, &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", parent);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(parent);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(cellToChildren,
           "Returns an array of H3 indexes that are the children (or "
           "lower) of the provided cell") {
    // TODO: This function contains a lot of code that is very similar to
    // `gridDisk`. If this happens again we should DRY them.
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
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
    Arg *args[] = {&cellToChildrenArg, &helpArg, &cellArg, &resArg, &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
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
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < len; i++) {
            if (out[i] != 0) {
                h3Println(out[i]);
            }
        }
    } else {
        free(out);
        return E_FAILED;
    }

    free(out);
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
    printf("%" PRId64 "\n", len);
    return E_SUCCESS;
}

SUBCOMMAND(
    cellToCenterChild,
    "Returns the H3 index that is the centrally-placed child (or lower) of the "
    "provided cell") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n (Default: json)");
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
    Arg *args[] = {&cellToCenterChildArg, &helpArg, &cellArg, &resArg,
                   &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", centerChild);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(centerChild);
    } else {
        return E_FAILED;
    }
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
    printf("%" PRId64 "\n", len);
    return E_SUCCESS;
}

SUBCOMMAND(childPosToCell,
           "Returns the child cell at a given position and resolution within "
           "an ordered list of all children of the parent cell") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n (Default: json)");
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
    Arg *args[] = {&childPosToCellArg, &helpArg, &cellArg, &resArg, &posArg,
                   &formatArg};
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", child);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(child);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

H3Index *readCellsFromFile(FILE *fp, char *buffer, size_t *totalCells) {
    // It's assumed the buffer is a character array of size 1501 to support up
    // to 100 cells at a time. If the file pointer is a null pointer, we assume
    // the buffer has all of the possible cells already stored in it and go from
    // there. Otherwise we continue reading from the file until it's fully
    // consumed. On an error, we de-allocate the output buffer and then return
    // 0. It's the responsibility of the caller to free the returned buffer and
    // the file pointer otherwise. The output array's filled length is set on
    // the cellsOffset pointer so it can be used by the caller.
    H3Index *cells = calloc(128, sizeof(H3Index));
    int cellsLen = 128;
    int bufferOffset = 0;
    int cellsOffset = 0;
    do {
        // Always start from the beginning of the buffer
        bufferOffset = 0;
        int lastGoodOffset = 0;
        H3Index cell = 0;
        while (bufferOffset <
               BUFFER_SIZE_LESS_CELL) {  // Keep consuming as much as possible
            // A valid H3 cell is exactly 15 hexadecomical characters.
            // Determine if we have a match, otherwise increment
            int scanlen = 0;
            sscanf(buffer + bufferOffset, "%" PRIx64 "%n", &cell, &scanlen);
            if (scanlen != 15) {
                cell = 0;
                bufferOffset += 1;
            } else {
                bufferOffset += 16;
                lastGoodOffset = bufferOffset;
            }
            // If we still don't have a cell and we've reached the end, we reset
            // the offset and `continue` to trigger another read
            if (cell == 0 && bufferOffset == BUFFER_SIZE) {
                bufferOffset = 0;
                continue;
            } else if (cell != 0) {
                // Otherwise, we have a cell to shove into the cells array.
                cells[cellsOffset] = cell;
                cellsOffset += 1;
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
        if (lastGoodOffset < BUFFER_SIZE_LESS_CELL) {
            lastGoodOffset = BUFFER_SIZE_LESS_CELL;
        }
        for (int i = 0; i < BUFFER_SIZE - lastGoodOffset; i++) {
            buffer[i] = buffer[i + lastGoodOffset];
        }
        bufferOffset = BUFFER_SIZE - lastGoodOffset;
    } while (fp != 0 && fread(buffer + bufferOffset, 1,
                              BUFFER_SIZE - bufferOffset, fp) != 0);
    *totalCells = cellsOffset;
    return cells;
}

SUBCOMMAND(compactCells,
           "Compacts the provided set of cells as best as possible. The set of "
           "input cells must all share the same resolution.") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {
        .names = {"-i", "--file"},
        .scanFormat = "%1023c",
        .valueName = "FILENAME",
        .value = &filename,
        .helpText =
            "The file to load the cells from. Use -- to read from stdin."};
    char cellStrs[BUFFER_SIZE_WITH_NULL] = {
        0};  // Up to 100 cells with zero padding
    Arg cellStrsArg = {.names = {"-c", "--cells"},
                       // TODO: Can I use `BUFFER_SIZE` here somehow?
                       .scanFormat = "%1500c",
                       .valueName = "CELLS",
                       .value = &cellStrs,
                       .helpText =
                           "The cells to compact. Up to 100 cells if provided "
                           "as hexadecimals with zero padding."};
    Arg *args[] = {&compactCellsArg, &helpArg, &filenameArg, &cellStrsArg,
                   &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (!filenameArg.found && !cellStrsArg.found) {
        fprintf(stderr,
                "You must provide either a file to read from or a set of cells "
                "to compact to use compactCells");
        exit(1);
    }
    FILE *fp = 0;
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
        if (fread(cellStrs, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    size_t cellsOffset = 0;
    H3Index *cells = readCellsFromFile(fp, cellStrs, &cellsOffset);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (cells == NULL) {
        return E_FAILED;
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
        int64_t trueLen = 0;
        for (int64_t i = 0; i < cellsOffset; i++) {
            if (compactedSet[i] != 0) {
                trueLen++;
            }
        }
        printf("[ ");
        for (int64_t i = 0, j = 0; i < cellsOffset; i++) {
            if (compactedSet[i] != 0) {
                j++;
                printf("\"%" PRIx64 "\"%s", compactedSet[i],
                       j == trueLen ? "" : ", ");
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < cellsOffset; i++) {
            if (compactedSet[i] != 0) {
                h3Println(compactedSet[i]);
            }
        }
    } else {
        free(cells);
        free(compactedSet);
        return E_FAILED;
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
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {
        .names = {"-i", "--file"},
        .scanFormat = "%1023c",
        .valueName = "FILENAME",
        .value = &filename,
        .helpText =
            "The file to load the cells from. Use -- to read from stdin."};
    char cellStrs[BUFFER_SIZE_WITH_NULL] = {
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
    Arg *args[] = {&uncompactCellsArg, &helpArg, &filenameArg,
                   &cellStrsArg,       &resArg,  &formatArg};
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
    FILE *fp = 0;
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
        if (fread(cellStrs, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    size_t cellsOffset = 0;
    H3Index *cells = readCellsFromFile(fp, cellStrs, &cellsOffset);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (cells == NULL) {
        return E_FAILED;
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
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
        int64_t trueLen = 0;
        for (int64_t i = 0; i < uncompactedSize; i++) {
            if (uncompactedSet[i] != 0) {
                trueLen++;
            }
        }
        printf("[ ");
        for (int64_t i = 0, j = 0; i < uncompactedSize; i++) {
            if (uncompactedSet[i] != 0) {
                j++;
                printf("\"%" PRIx64 "\"%s", uncompactedSet[i],
                       j == trueLen ? "" : ", ");
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < uncompactedSize; i++) {
            if (uncompactedSet[i] != 0) {
                h3Println(uncompactedSet[i]);
            }
        }
    } else {
        free(cells);
        free(uncompactedSet);
        return E_FAILED;
    }

    free(cells);
    free(uncompactedSet);
    return E_SUCCESS;
}

/// Region subcommands

H3Error polygonStringToGeoPolygon(FILE *fp, char *polygonString,
                                  GeoPolygon *out) {
    // There are two kinds of valid input, an array of arrays of lat, lng values
    // defining a singular polygon loop, or an array of array of arrays of lat,
    // lng values defining a polygon and zero or more holes. Which kind of input
    // this is can be determined by the `[` depth reached before the first
    // floating point number is found. This means either 2 or 3 `[`s at the
    // beginning are valid, and nothing more than that.
    int8_t maxDepth = 0;
    int8_t curDepth = 0;
    int64_t numVerts = 6;
    int64_t curVert = 0;
    int64_t curLoop = 0;
    LatLng *verts = calloc(numVerts, sizeof(LatLng));
    int strPos = 0;
    while (polygonString[strPos] != 0) {
        // Load more of the file if we've hit our buffer limit
        if (strPos >= BUFFER_SIZE && fp != 0) {
            int result = fread(polygonString, 1, BUFFER_SIZE, fp);
            strPos = 0;
            // If we didn't get any data from the file, we're done.
            if (result == 0) {
                break;
            }
        }
        // Try to match `[` first
        if (polygonString[strPos] == '[') {
            curDepth++;
            if (curDepth > maxDepth) {
                maxDepth = curDepth;
            }
            if (curDepth > 4) {
                // This is beyond the depth for a valid input, so we abort early
                free(verts);
                return E_FAILED;
            }
            strPos++;
            continue;
        }
        // Similar matching for `]`
        if (polygonString[strPos] == ']') {
            curDepth--;
            if (curDepth < 0) {
                break;
            }
            strPos++;
            // We may need to set up a new geoloop at this point. If the
            // curDepth <= maxDepth - 2 then we increment the curLoop and get a
            // new one set up.
            if (curDepth <= maxDepth - 2) {
                if (curLoop == 0) {
                    out->geoloop.verts = verts;
                    out->geoloop.numVerts = curVert;
                } else {
                    GeoLoop *holes = calloc(out->numHoles + 1, sizeof(GeoLoop));
                    if (holes == 0) {
                        return E_MEMORY_ALLOC;
                    }
                    for (int i = 0; i < out->numHoles; i++) {
                        holes[i].numVerts = out->holes[i].numVerts;
                        holes[i].verts = out->holes[i].verts;
                    }
                    free(out->holes);
                    holes[out->numHoles].verts = verts;
                    holes[out->numHoles].numVerts = curVert;
                    out->holes = holes;
                    out->numHoles++;
                }
                curLoop++;
                curVert = 0;
                numVerts = 6;
                verts = calloc(numVerts, sizeof(LatLng));
            }
            continue;
        }
        // Try to grab a floating point value followed by optional whitespace, a
        // comma, and more optional whitespace, and a second floating point
        // value, then store the lat, lng pair
        double lat, lng;
        int count = 0;
        // Must grab the closing ] or we might accidentally parse a partial
        // float across buffer boundaries
        char closeBracket[2] = "]";
        int result = sscanf(polygonString + strPos, "%lf%*[, \n]%lf%1[]]%n",
                            &lat, &lng, &closeBracket[0], &count);
        if (count > 0 && result == 3) {
            verts[curVert].lat = H3_EXPORT(degsToRads)(lat);
            verts[curVert].lng = H3_EXPORT(degsToRads)(lng);
            curVert++;
            // Create a new vert buffer, copy the old buffer, and free it, if
            // necessary
            if (curVert == numVerts) {
                LatLng *newVerts = calloc(numVerts * 2, sizeof(LatLng));
                for (int i = 0; i < numVerts; i++) {
                    newVerts[i].lat = verts[i].lat;
                    newVerts[i].lng = verts[i].lng;
                }
                free(verts);
                verts = newVerts;
                numVerts *= 2;
            }
            strPos += count;
            curDepth--;
            continue;
        }
        // Check for whitespace and skip it if we reach this point.
        if (polygonString[strPos] == ',' || polygonString[strPos] == ' ' ||
            polygonString[strPos] == '\n' || polygonString[strPos] == '\t' ||
            polygonString[strPos] == '\r') {
            strPos++;
            continue;
        }
        if (strPos != 0 && fp != 0) {
            // Scan the remaining of the current buffer for `0`. If not
            // found, then we grab a new chunk and append to the remainder
            // of the existing buffer. Otherwise, just skip unknown
            // characters.
            bool endOfFile = false;
            for (int i = strPos; i <= BUFFER_SIZE; i++) {
                if (polygonString[strPos] == 0) {
                    endOfFile = true;
                    break;
                }
            }
            if (endOfFile) {
                strPos++;
            } else {
                // Move the end of this buffer to the beginning
                for (int i = strPos; i <= BUFFER_SIZE; i++) {
                    polygonString[i - strPos] = polygonString[i];
                }
                // Set the string position to the end of the buffer
                strPos = BUFFER_SIZE - strPos;
                // Grab up to the remaining size of the buffer and fill it
                // into the file
                // Did you know that if the amount you want to read from
                // the file buffer is larger than the buffer itself,
                // fread can choose to give you squat and not the
                // remainder of the file as you'd expect from the
                // documentation? This was a surprise to me and a
                // significant amount of wasted time to figure out how
                // to tackle. C leaves *way* too much as undefined
                // behavior to be nice to work with...
                int64_t i = 0;
                int result;
                do {
                    result = fread(polygonString + strPos + i, 1, 1, fp);
                    i++;
                } while (i < BUFFER_SIZE - strPos && result != 0);
                if (result == 0) {
                    polygonString[strPos + i - 1] = 0;
                }
                strPos = 0;
            }
        } else {
            strPos++;
        }
    }
    free(verts);
    return E_SUCCESS;
}

SUBCOMMAND(
    polygonToCells,
    "Converts a polygon (array of lat, lng points, or array of arrays of lat, "
    "lng points) into a set of covering cells at the specified resolution") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {
        .names = {"-i", "--file"},
        .scanFormat = "%1023c",
        .valueName = "FILENAME",
        .value = &filename,
        .helpText =
            "The file to load the polygon from. Use -- to read from stdin."};
    char polygonStr[BUFFER_SIZE_WITH_NULL] = {
        0};  // Up to 100 cells with zero padding
    Arg polygonStrArg = {
        .names = {"-p", "--polygon"},
        .scanFormat = "%1500c",
        .valueName = "POLYGON",
        .value = &polygonStr,
        .helpText = "The polygon to convert. Up to 1500 characters."};
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 0-15 inclusive, that the polygon "
                      "should be converted to."};
    Arg *args[] = {&polygonToCellsArg, &helpArg, &filenameArg,
                   &polygonStrArg,     &resArg,  &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (filenameArg.found == polygonStrArg.found) {
        fprintf(stderr,
                "You must provide either a file to read from or a polygon "
                "to cover to use polygonToCells");
        exit(1);
    }
    FILE *fp = 0;
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
        if (fread(polygonStr, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    GeoPolygon polygon = {0};
    H3Error err = polygonStringToGeoPolygon(fp, polygonStr, &polygon);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (err != E_SUCCESS) {
        return err;
    }
    int64_t cellsSize = 0;
    err = H3_EXPORT(maxPolygonToCellsSize)(&polygon, res, 0, &cellsSize);
    if (err != E_SUCCESS) {
        return err;
    }
    H3Index *cells = calloc(cellsSize, sizeof(H3Index));
    err = H3_EXPORT(polygonToCells)(&polygon, res, 0, cells);
    for (int i = 0; i < polygon.numHoles; i++) {
        free(polygon.holes[i].verts);
    }
    free(polygon.holes);
    free(polygon.geoloop.verts);
    if (err != E_SUCCESS) {
        free(cells);
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
        int64_t trueLen = 0;
        for (int64_t i = 0; i < cellsSize; i++) {
            if (cells[i] != 0) {
                trueLen++;
            }
        }
        printf("[ ");
        for (int64_t i = 0, j = 0; i < cellsSize; i++) {
            if (cells[i] != 0) {
                j++;
                printf("\"%" PRIx64 "\"%s", cells[i], j == trueLen ? "" : ", ");
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < cellsSize; i++) {
            if (cells[i] != 0) {
                h3Println(cells[i]);
            }
        }
    } else {
        free(cells);
        return E_FAILED;
    }

    free(cells);
    return E_SUCCESS;
}

SUBCOMMAND(
    maxPolygonToCellsSize,
    "Returns the maximum number of cells that could be needed to cover "
    "the polygon. Will always be equal or more than actually necessary") {
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {
        .names = {"-i", "--file"},
        .scanFormat = "%1023c",
        .valueName = "FILENAME",
        .value = &filename,
        .helpText =
            "The file to load the polygon from. Use -- to read from stdin."};
    char polygonStr[BUFFER_SIZE_WITH_NULL] = {
        0};  // Up to 100 cells with zero padding
    Arg polygonStrArg = {
        .names = {"-p", "--polygon"},
        .scanFormat = "%1500c",
        .valueName = "POLYGON",
        .value = &polygonStr,
        .helpText = "The polygon to convert. Up to 1500 characters."};
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, 0-15 inclusive, that the polygon "
                      "should be converted to."};
    Arg *args[] = {&maxPolygonToCellsSizeArg, &helpArg, &filenameArg,
                   &polygonStrArg, &resArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (filenameArg.found == polygonStrArg.found) {
        fprintf(stderr,
                "You must provide either a file to read from or a polygon "
                "to cover to use maxPolygonToCellsSize");
        exit(1);
    }
    FILE *fp = 0;
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
        if (fread(polygonStr, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    GeoPolygon polygon = {0};
    H3Error err = polygonStringToGeoPolygon(fp, polygonStr, &polygon);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (err != E_SUCCESS) {
        return err;
    }
    int64_t cellsSize = 0;
    err = H3_EXPORT(maxPolygonToCellsSize)(&polygon, res, 0, &cellsSize);
    for (int i = 0; i < polygon.numHoles; i++) {
        free(polygon.holes[i].verts);
    }
    free(polygon.holes);
    free(polygon.geoloop.verts);
    if (err != E_SUCCESS) {
        return err;
    }
    printf("%" PRId64 "\n", cellsSize);
    return E_SUCCESS;
}

SUBCOMMAND(cellsToMultiPolygon,
           "Returns a polygon (array of arrays of "
           "lat, lng points) for a set of cells") {
    DEFINE_FORMAT_ARG(
        "'json' for [[[[lat, lng],...],...],...] 'wkt' for a WKT MULTIPOLYGON");
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {
        .names = {"-i", "--file"},
        .scanFormat = "%1023c",
        .valueName = "FILENAME",
        .value = &filename,
        .helpText =
            "The file to load the cells from. Use -- to read from stdin."};
    char cellStrs[BUFFER_SIZE_WITH_NULL] = {
        0};  // Up to 100 cells with zero padding
    Arg cellStrsArg = {.names = {"-c", "--cells"},
                       .scanFormat = "%1500c",
                       .valueName = "CELLS",
                       .value = &cellStrs,
                       .helpText =
                           "The cells to convert. Up to 100 cells if provided "
                           "as hexadecimals with zero padding."};
    Arg *args[] = {&cellsToMultiPolygonArg, &helpArg, &filenameArg,
                   &cellStrsArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (filenameArg.found == cellStrsArg.found) {
        fprintf(stderr,
                "You must provide either a file to read from or a set of cells "
                "to convert to use cellsToMultiPolygon");
        exit(1);
    }
    FILE *fp = 0;
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
        if (fread(cellStrs, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    size_t cellsOffset = 0;
    H3Index *cells = readCellsFromFile(fp, cellStrs, &cellsOffset);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (cells == NULL) {
        return E_FAILED;
    }
    LinkedGeoPolygon out = {0};
    H3Error err =
        H3_EXPORT(cellsToLinkedMultiPolygon)(cells, cellsOffset, &out);
    if (err) {
        free(cells);
        H3_EXPORT(destroyLinkedMultiPolygon)(&out);
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[");
        LinkedGeoPolygon *currPoly = &out;
        while (currPoly) {
            printf("[");
            LinkedGeoLoop *currLoop = currPoly->first;
            while (currLoop) {
                printf("[");
                LinkedLatLng *currLatLng = currLoop->first;
                while (currLatLng) {
                    printf("[%f, %f]",
                           H3_EXPORT(radsToDegs)(currLatLng->vertex.lat),
                           H3_EXPORT(radsToDegs)(currLatLng->vertex.lng));
                    currLatLng = currLatLng->next;
                    if (currLatLng) {
                        printf(", ");
                    }
                }
                currLoop = currLoop->next;
                if (currLoop) {
                    printf("], ");
                } else {
                    printf("]");
                }
            }
            currPoly = currPoly->next;
            if (currPoly) {
                printf("], ");
            } else {
                printf("]");
            }
        }
        printf("]\n");
    } else if (strcmp(format, "wkt") == 0) {
        printf("MULTIPOLYGON (");
        LinkedGeoPolygon *currPoly = &out;
        while (currPoly) {
            printf("(");
            LinkedGeoLoop *currLoop = currPoly->first;
            while (currLoop) {
                printf("(");
                LinkedLatLng *currLatLng = currLoop->first;
                while (currLatLng) {
                    printf("%f %f",
                           H3_EXPORT(radsToDegs)(currLatLng->vertex.lng),
                           H3_EXPORT(radsToDegs)(currLatLng->vertex.lat));
                    currLatLng = currLatLng->next;
                    if (currLatLng) {
                        printf(", ");
                    }
                }
                currLoop = currLoop->next;
                if (currLoop) {
                    printf("), ");
                } else {
                    printf(")");
                }
            }
            currPoly = currPoly->next;
            if (currPoly) {
                printf("), ");
            } else {
                printf(")");
            }
        }
        printf(")\n");
    } else {
        free(cells);
        H3_EXPORT(destroyLinkedMultiPolygon)(&out);
        return E_FAILED;
    }
    free(cells);
    H3_EXPORT(destroyLinkedMultiPolygon)(&out);
    return E_SUCCESS;
}

/// Directed edge subcommands

SUBCOMMAND(areNeighborCells,
           "Determines if the provided H3 cells are neighbors (have a shared "
           "border)") {
    DEFINE_FORMAT_ARG(
        "'json' for true or false, 'numeric' for 1 or 0 (Default: json)");
    H3Index origin, destination;
    Arg originCellArg = {.names = {"-o", "--origin"},
                         .required = true,
                         .scanFormat = "%" PRIx64,
                         .valueName = "CELL",
                         .value = &origin,
                         .helpText = "Origin H3 Cell"};
    Arg destinationCellArg = {.names = {"-d", "--destination"},
                              .required = true,
                              .scanFormat = "%" PRIx64,
                              .valueName = "CELL",
                              .value = &destination,
                              .helpText = "Destination H3 Cell"};
    Arg *args[] = {&areNeighborCellsArg, &originCellArg, &destinationCellArg,
                   &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int areNeighbors = 0;
    H3Error err =
        H3_EXPORT(areNeighborCells)(origin, destination, &areNeighbors);
    if (err != E_SUCCESS) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("%s\n", areNeighbors ? "true" : "false");
    } else if (strcmp(format, "numeric") == 0) {
        printf("%d\n", areNeighbors);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(cellsToDirectedEdge,
           "Converts neighboring cells into a directed edge index (or errors "
           "if they are not neighbors)") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n "
        "(Default: json)");
    H3Index origin, destination;
    Arg originCellArg = {.names = {"-o", "--origin"},
                         .required = true,
                         .scanFormat = "%" PRIx64,
                         .valueName = "CELL",
                         .value = &origin,
                         .helpText = "Origin H3 Cell"};
    Arg destinationCellArg = {.names = {"-d", "--destination"},
                              .required = true,
                              .scanFormat = "%" PRIx64,
                              .valueName = "CELL",
                              .value = &destination,
                              .helpText = "Destination H3 Cell"};
    Arg *args[] = {&cellsToDirectedEdgeArg, &originCellArg, &destinationCellArg,
                   &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index out = 0;
    H3Error err = H3_EXPORT(cellsToDirectedEdge)(origin, destination, &out);
    if (err != E_SUCCESS) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", out);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(out);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(isValidDirectedEdge,
           "Checks if the provided H3 directed edge is actually valid") {
    DEFINE_FORMAT_ARG(
        "'json' for true or false, 'numeric' for 1 or 0 (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isValidDirectedEdgeArg, &helpArg, &cellArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    bool isValid = H3_EXPORT(isValidDirectedEdge)(cell);
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("%s\n", isValid ? "true" : "false");
    } else if (strcmp(format, "numeric") == 0) {
        printf("%d\n", isValid);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(getDirectedEdgeOrigin,
           "Returns the origin cell from the directed edge") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n "
        "(Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getDirectedEdgeOriginArg, &cellArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index out = 0;
    H3Error err = H3_EXPORT(getDirectedEdgeOrigin)(cell, &out);
    if (err != E_SUCCESS) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", out);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(out);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(getDirectedEdgeDestination,
           "Returns the destination cell from the directed edge") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n "
        "(Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&getDirectedEdgeDestinationArg, &cellArg, &helpArg,
                   &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index out = 0;
    H3Error err = H3_EXPORT(getDirectedEdgeDestination)(cell, &out);
    if (err != E_SUCCESS) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", out);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(out);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(
    directedEdgeToCells,
    "Returns the origin, destination pair of cells from the directed edge") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&directedEdgeToCellsArg, &cellArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index out[2] = {0};
    H3Error err = H3_EXPORT(directedEdgeToCells)(cell, &out[0]);
    if (err != E_SUCCESS) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[\"%" PRIx64 "\", \"%" PRIx64 "\"]\n", out[0], out[1]);
    } else if (strcmp(format, "newline") == 0) {
        printf("%" PRIx64 "\n%" PRIx64 "\n", out[0], out[1]);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(originToDirectedEdges,
           "Returns all of the directed edges from the specified origin cell") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&originToDirectedEdgesArg, &cellArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index out[6] = {0};
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidCell)(cell);
    if (!isValid) {
        return E_CELL_INVALID;
    }
    H3Error err = H3_EXPORT(originToDirectedEdges)(cell, &out[0]);
    if (err != E_SUCCESS) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[");
        bool hasPrinted = false;
        for (int i = 0; i < 6; i++) {
            if (out[i] > 0) {
                if (hasPrinted) {
                    printf(", ");
                }
                printf("\"%" PRIx64 "\"", out[i]);
                hasPrinted = true;
            }
        }
        printf("]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int i = 0; i < 6; i++) {
            if (out[i] != 0) {
                h3Println(out[i]);
            }
        }
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(directedEdgeToBoundary,
           "Provides the coordinates defining the directed edge") {
    DEFINE_FORMAT_ARG(
        "'json' for [[lat, lng], ...], 'wkt' for a WKT POLYGON, 'newline' for "
        "lat\\nlng\\n... (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&directedEdgeToBoundaryArg, &cellArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    CellBoundary cb = {0};
    H3Error err = H3_EXPORT(directedEdgeToBoundary)(cell, &cb);
    if (err) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[");
        for (int i = 0; i < cb.numVerts - 1; i++) {
            LatLng *ll = &cb.verts[i];
            printf("[%.10lf, %.10lf], ", H3_EXPORT(radsToDegs)(ll->lat),
                   H3_EXPORT(radsToDegs)(ll->lng));
        }
        printf("[%.10lf, %.10lf]]\n",
               H3_EXPORT(radsToDegs)(cb.verts[cb.numVerts - 1].lat),
               H3_EXPORT(radsToDegs)(cb.verts[cb.numVerts - 1].lng));
    } else if (strcmp(format, "wkt") == 0) {
        printf("LINESTRING (");
        for (int i = 0; i < cb.numVerts - 1; i++) {
            LatLng *ll = &cb.verts[i];
            printf("%.10lf %.10lf, ", H3_EXPORT(radsToDegs)(ll->lng),
                   H3_EXPORT(radsToDegs)(ll->lat));
        }
        printf("%.10lf %.10lf)\n",
               H3_EXPORT(radsToDegs)(cb.verts[cb.numVerts - 1].lng),
               H3_EXPORT(radsToDegs)(cb.verts[cb.numVerts - 1].lat));
    } else if (strcmp(format, "newline") == 0) {
        for (int i = 0; i < cb.numVerts; i++) {
            LatLng *ll = &cb.verts[i];
            printf("%.10lf\n%.10lf\n", H3_EXPORT(radsToDegs)(ll->lat),
                   H3_EXPORT(radsToDegs)(ll->lng));
        }
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

/// Vertex subcommands

SUBCOMMAND(cellToVertex,
           "Returns the vertex for the specified cell and vertex index. Must "
           "be 0-5 for hexagons, 0-4 for pentagons") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n "
        "(Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    int vertIndex = 0;
    Arg vertIndexArg = {
        .names = {"-v", "--vertex"},
        .required = true,
        .scanFormat = "%d",
        .valueName = "INDEX",
        .value = &vertIndex,
        .helpText = "Vertex index number. 0-5 for hexagons, 0-4 for pentagons"};
    Arg *args[] = {&cellToVertexArg, &cellArg, &vertIndexArg, &helpArg,
                   &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // This function also doesn't sanitize its inputs correctly
    bool isValid = H3_EXPORT(isValidCell)(cell);
    if (!isValid) {
        return E_CELL_INVALID;
    }
    H3Index out = 0;
    H3Error err = H3_EXPORT(cellToVertex)(cell, vertIndex, &out);
    if (err) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", out);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(out);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(cellToVertexes,
           "Returns all of the vertexes from the specified cell") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellToVertexesArg, &cellArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index out[6] = {0};
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidCell)(cell);
    if (!isValid) {
        return E_CELL_INVALID;
    }
    H3Error err = H3_EXPORT(cellToVertexes)(cell, &out[0]);
    if (err != E_SUCCESS) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
        int64_t trueLen = 0;
        for (int64_t i = 0; i < 6; i++) {
            if (out[i] != 0) {
                trueLen++;
            }
        }
        printf("[ ");
        for (int64_t i = 0, j = 0; i < 6; i++) {
            if (out[i] != 0) {
                j++;
                printf("\"%" PRIx64 "\"%s", out[i], j == trueLen ? "" : ", ");
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < 6; i++) {
            if (out[i] != 0) {
                h3Println(out[i]);
            }
        }
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(vertexToLatLng, "Returns the lat, lng pair for the given vertex") {
    DEFINE_FORMAT_ARG(
        "'json' for [lat, lng], 'wkt' for a WKT POINT, 'newline' for "
        "lat\\nlng\\n (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&vertexToLatLngArg, &cellArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    bool isValid = H3_EXPORT(isValidVertex)(cell);
    if (!isValid) {
        return E_VERTEX_INVALID;
    }
    LatLng ll;
    H3Error err = H3_EXPORT(vertexToLatLng)(cell, &ll);
    if (err) {
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("[%.10lf, %.10lf]\n", H3_EXPORT(radsToDegs)(ll.lat),
               H3_EXPORT(radsToDegs)(ll.lng));
    } else if (strcmp(format, "wkt") == 0) {
        // Using WKT formatting for the output. TODO: Add support for JSON
        // formatting
        printf("POINT(%.10lf %.10lf)\n", H3_EXPORT(radsToDegs)(ll.lng),
               H3_EXPORT(radsToDegs)(ll.lat));
    } else if (strcmp(format, "newline") == 0) {
        printf("%.10lf\n%.10lf\n", H3_EXPORT(radsToDegs)(ll.lat),
               H3_EXPORT(radsToDegs)(ll.lng));
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(isValidVertex,
           "Checks if the provided H3 vertex is actually valid") {
    DEFINE_FORMAT_ARG(
        "'json' for true or false, 'numeric' for 1 or 0 (Default: json)");
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&isValidVertexArg, &helpArg, &cellArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    bool isValid = H3_EXPORT(isValidVertex)(cell);
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("%s\n", isValid ? "true" : "false");
    } else if (strcmp(format, "numeric") == 0) {
        printf("%d\n", isValid);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

/// Miscellaneous subcommands

SUBCOMMAND(degsToRads, "Converts degrees to radians") {
    double deg = 0;
    Arg degArg = {.names = {"-d", "--degree"},
                  .required = true,
                  .scanFormat = "%lf",
                  .valueName = "DEG",
                  .value = &deg,
                  .helpText = "Angle in degrees"};
    Arg *args[] = {&degsToRadsArg, &degArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    printf("%.10lf\n", H3_EXPORT(degsToRads)(deg));
    return E_SUCCESS;
}

SUBCOMMAND(radsToDegs, "Converts radians to degrees") {
    double rad = 0;
    Arg radArg = {.names = {"-r", "--radian"},
                  .required = true,
                  .scanFormat = "%lf",
                  .valueName = "RAD",
                  .value = &rad,
                  .helpText = "Angle in radians"};
    Arg *args[] = {&radsToDegsArg, &radArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    printf("%.10lf\n", H3_EXPORT(radsToDegs)(rad));
    return E_SUCCESS;
}

SUBCOMMAND(getHexagonAreaAvgKm2,
           "The average area in square kilometers for a hexagon of a given "
           "resolution (excludes pentagons)") {
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg *args[] = {&getHexagonAreaAvgKm2Arg, &resArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    double area = 0;
    H3Error err = H3_EXPORT(getHexagonAreaAvgKm2)(res, &area);
    if (err) {
        return err;
    }
    printf("%.10lf\n", area);
    return E_SUCCESS;
}

SUBCOMMAND(getHexagonAreaAvgM2,
           "The average area in square meters for a hexagon of a given "
           "resolution (excludes pentagons)") {
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg *args[] = {&getHexagonAreaAvgM2Arg, &resArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    double area = 0;
    H3Error err = H3_EXPORT(getHexagonAreaAvgM2)(res, &area);
    if (err) {
        return err;
    }
    printf("%.10lf\n", area);
    return E_SUCCESS;
}

SUBCOMMAND(cellAreaRads2,
           "The exact area of a specific cell in square radians") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellAreaRads2Arg, &cellArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidCell)(cell);
    if (!isValid) {
        return E_CELL_INVALID;
    }
    double area = 0;
    H3Error err = H3_EXPORT(cellAreaRads2)(cell, &area);
    if (err) {
        return err;
    }
    printf("%.10lf\n", area);
    return E_SUCCESS;
}

SUBCOMMAND(cellAreaKm2,
           "The exact area of a specific cell in square kilometers") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellAreaKm2Arg, &cellArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidCell)(cell);
    if (!isValid) {
        return E_CELL_INVALID;
    }
    double area = 0;
    H3Error err = H3_EXPORT(cellAreaKm2)(cell, &area);
    if (err) {
        return err;
    }
    printf("%.10lf\n", area);
    return E_SUCCESS;
}

SUBCOMMAND(cellAreaM2, "The exact area of a specific cell in square meters") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&cellAreaM2Arg, &cellArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidCell)(cell);
    if (!isValid) {
        return E_CELL_INVALID;
    }
    double area = 0;
    H3Error err = H3_EXPORT(cellAreaM2)(cell, &area);
    if (err) {
        return err;
    }
    printf("%.10lf\n", area);
    return E_SUCCESS;
}

SUBCOMMAND(getHexagonEdgeLengthAvgKm,
           "The average hexagon edge length in kilometers of a given "
           "resolution (excludes pentagons)") {
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg *args[] = {&getHexagonEdgeLengthAvgKmArg, &resArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    double area = 0;
    H3Error err = H3_EXPORT(getHexagonEdgeLengthAvgKm)(res, &area);
    if (err) {
        return err;
    }
    printf("%.10lf\n", area);
    return E_SUCCESS;
}

SUBCOMMAND(getHexagonEdgeLengthAvgM,
           "The average hexagon edge length in meters of a given "
           "resolution (excludes pentagons)") {
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg *args[] = {&getHexagonEdgeLengthAvgMArg, &resArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    double area = 0;
    H3Error err = H3_EXPORT(getHexagonEdgeLengthAvgM)(res, &area);
    if (err) {
        return err;
    }
    printf("%.10lf\n", area);
    return E_SUCCESS;
}

SUBCOMMAND(edgeLengthRads,
           "The exact edge length of a specific directed edge in radians") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&edgeLengthRadsArg, &cellArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidDirectedEdge)(cell);
    if (!isValid) {
        return E_DIR_EDGE_INVALID;
    }
    double length = 0;
    H3Error err = H3_EXPORT(edgeLengthRads)(cell, &length);
    if (err) {
        return err;
    }
    printf("%.10lf\n", length);
    return E_SUCCESS;
}

SUBCOMMAND(edgeLengthKm,
           "The exact edge length of a specific directed edge in kilometers") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&edgeLengthKmArg, &cellArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidDirectedEdge)(cell);
    if (!isValid) {
        return E_DIR_EDGE_INVALID;
    }
    double length = 0;
    H3Error err = H3_EXPORT(edgeLengthKm)(cell, &length);
    if (err) {
        return err;
    }
    printf("%.10lf\n", length);
    return E_SUCCESS;
}

SUBCOMMAND(edgeLengthM,
           "The exact edge length of a specific directed edge in meters") {
    DEFINE_CELL_ARG(cell, cellArg);
    Arg *args[] = {&edgeLengthMArg, &cellArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    // This one is pretty loose about the inputs it accepts, so let's validate
    // for it
    bool isValid = H3_EXPORT(isValidDirectedEdge)(cell);
    if (!isValid) {
        return E_DIR_EDGE_INVALID;
    }
    double length = 0;
    H3Error err = H3_EXPORT(edgeLengthM)(cell, &length);
    if (err) {
        return err;
    }
    printf("%.10lf\n", length);
    return E_SUCCESS;
}

SUBCOMMAND(getNumCells,
           "The number of unique H3 cells for a specified resolution") {
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg *args[] = {&getNumCellsArg, &resArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int64_t numCells = 0;
    H3Error err = H3_EXPORT(getNumCells)(res, &numCells);
    if (err) {
        return err;
    }
    printf("%" PRId64 "\n", numCells);
    return E_SUCCESS;
}

SUBCOMMAND(getRes0Cells, "Returns all of the resolution 0 cells") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    Arg *args[] = {&getRes0CellsArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index *out = calloc(122, sizeof(H3Index));
    H3Error err = H3_EXPORT(getRes0Cells)(out);
    if (err != E_SUCCESS) {
        free(out);
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
        int64_t trueLen = 0;
        for (int64_t i = 0; i < 122; i++) {
            if (out[i] != 0) {
                trueLen++;
            }
        }
        printf("[ ");
        for (int64_t i = 0, j = 0; i < 122; i++) {
            if (out[i] != 0) {
                j++;
                printf("\"%" PRIx64 "\"%s", out[i], j == trueLen ? "" : ", ");
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < 122; i++) {
            if (out[i] != 0) {
                h3Println(out[i]);
            }
        }
    } else {
        free(out);
        return E_FAILED;
    }
    free(out);
    return E_SUCCESS;
}

SUBCOMMAND(getPentagons,
           "Returns all of the pentagons at the specified resolution") {
    DEFINE_FORMAT_ARG(
        "'json' for [\"CELL\", ...], 'newline' for CELL\\n... (Default: json)");
    int res = 0;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg *args[] = {&getPentagonsArg, &resArg, &helpArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index *out = calloc(12, sizeof(H3Index));
    H3Error err = H3_EXPORT(getPentagons)(res, out);
    if (err != E_SUCCESS) {
        free(out);
        return err;
    }
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        // Since we don't know *actually* how many cells are in the output
        // (usually the max, but sometimes not), we need to do a quick scan to
        // figure out the true length in order to properly serialize to a JSON
        // array
        int64_t trueLen = 0;
        for (int64_t i = 0; i < 12; i++) {
            if (out[i] != 0) {
                trueLen++;
            }
        }
        printf("[ ");
        for (int64_t i = 0, j = 0; i < 12; i++) {
            if (out[i] != 0) {
                j++;
                printf("\"%" PRIx64 "\"%s", out[i], j == trueLen ? "" : ", ");
            }
        }
        printf(" ]\n");
    } else if (strcmp(format, "newline") == 0) {
        for (int64_t i = 0; i < 12; i++) {
            if (out[i] != 0) {
                h3Println(out[i]);
            }
        }
    } else {
        free(out);
        return E_FAILED;
    }
    free(out);
    return E_SUCCESS;
}

SUBCOMMAND(pentagonCount, "Returns 12") {
    Arg *args[] = {&pentagonCountArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    printf("12\n");
    return E_SUCCESS;
}

SUBCOMMAND(greatCircleDistanceRads,
           "Calculates the 'great circle' or 'haversine' distance between two "
           "lat, lng points, in radians") {
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {.names = {"-i", "--file"},
                       .scanFormat = "%1023c",
                       .valueName = "FILENAME",
                       .value = &filename,
                       .helpText =
                           "The file to load the coordinates from. Use -- to "
                           "read from stdin."};
    char coordinateStr[1501] = {0};
    Arg coordinateStrArg = {
        .names = {"-c", "--coordinates"},
        .scanFormat = "%1500c",
        .valueName = "ARRAY",
        .value = &coordinateStr,
        .helpText =
            "The array of coordinates to convert. Up to 1500 characters."};
    Arg *args[] = {&greatCircleDistanceRadsArg, &filenameArg, &coordinateStrArg,
                   &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (filenameArg.found == coordinateStrArg.found) {
        fprintf(
            stderr,
            "You must provide either a file to read from or a coordinate array "
            "to use greatCircleDistanceRads");
        exit(1);
    }
    FILE *fp = 0;
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
        if (fread(coordinateStr, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    GeoPolygon polygon = {0};
    H3Error err = polygonStringToGeoPolygon(fp, coordinateStr, &polygon);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (err != E_SUCCESS) {
        return err;
    }
    if (polygon.numHoles > 0 || polygon.geoloop.numVerts != 2) {
        fprintf(stderr, "Only two pairs of coordinates should be provided.");
        exit(1);
    }
    double distance = H3_EXPORT(greatCircleDistanceRads)(
        &polygon.geoloop.verts[0], &polygon.geoloop.verts[1]);
    printf("%.10lf\n", distance);
    for (int i = 0; i < polygon.numHoles; i++) {
        free(polygon.holes[i].verts);
    }
    free(polygon.holes);
    free(polygon.geoloop.verts);
    return E_SUCCESS;
}

SUBCOMMAND(greatCircleDistanceKm,
           "Calculates the 'great circle' or 'haversine' distance between two "
           "lat, lng points, in kilometers") {
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {.names = {"-i", "--file"},
                       .scanFormat = "%1023c",
                       .valueName = "FILENAME",
                       .value = &filename,
                       .helpText =
                           "The file to load the coordinates from. Use -- to "
                           "read from stdin."};
    char coordinateStr[1501] = {0};
    Arg coordinateStrArg = {
        .names = {"-c", "--coordinates"},
        .scanFormat = "%1500c",
        .valueName = "ARRAY",
        .value = &coordinateStr,
        .helpText =
            "The array of coordinates to convert. Up to 1500 characters."};
    Arg *args[] = {&greatCircleDistanceKmArg, &filenameArg, &coordinateStrArg,
                   &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (filenameArg.found == coordinateStrArg.found) {
        fprintf(
            stderr,
            "You must provide either a file to read from or a coordinate array "
            "to use greatCircleDistanceKm");
        exit(1);
    }
    FILE *fp = 0;
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
        if (fread(coordinateStr, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    GeoPolygon polygon = {0};
    H3Error err = polygonStringToGeoPolygon(fp, coordinateStr, &polygon);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (err != E_SUCCESS) {
        return err;
    }
    if (polygon.numHoles > 0 || polygon.geoloop.numVerts != 2) {
        fprintf(stderr, "Only two pairs of coordinates should be provided.");
        exit(1);
    }
    double distance = H3_EXPORT(greatCircleDistanceKm)(
        &polygon.geoloop.verts[0], &polygon.geoloop.verts[1]);
    printf("%.10lf\n", distance);
    for (int i = 0; i < polygon.numHoles; i++) {
        free(polygon.holes[i].verts);
    }
    free(polygon.holes);
    free(polygon.geoloop.verts);
    return E_SUCCESS;
}

SUBCOMMAND(greatCircleDistanceM,
           "Calculates the 'great circle' or 'haversine' distance between two "
           "lat, lng points, in meters") {
    char filename[1024] = {0};  // More than Windows, lol
    Arg filenameArg = {.names = {"-i", "--file"},
                       .scanFormat = "%1023c",
                       .valueName = "FILENAME",
                       .value = &filename,
                       .helpText =
                           "The file to load the coordinates from. Use -- to "
                           "read from stdin."};
    char coordinateStr[1501] = {0};
    Arg coordinateStrArg = {
        .names = {"-c", "--coordinates"},
        .scanFormat = "%1500c",
        .valueName = "ARRAY",
        .value = &coordinateStr,
        .helpText =
            "The array of coordinates to convert. Up to 1500 characters."};
    Arg *args[] = {&greatCircleDistanceMArg, &filenameArg, &coordinateStrArg,
                   &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    if (filenameArg.found == coordinateStrArg.found) {
        fprintf(
            stderr,
            "You must provide either a file to read from or a coordinate array "
            "to use greatCircleDistanceM");
        exit(1);
    }
    FILE *fp = 0;
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
        if (fread(coordinateStr, 1, BUFFER_SIZE, fp) == 0) {
            fprintf(stderr, "The specified file is empty.");
            exit(1);
        }
    }
    GeoPolygon polygon = {0};
    H3Error err = polygonStringToGeoPolygon(fp, coordinateStr, &polygon);
    if (fp != 0 && !isStdin) {
        fclose(fp);
    }
    if (err != E_SUCCESS) {
        return err;
    }
    if (polygon.numHoles > 0 || polygon.geoloop.numVerts != 2) {
        fprintf(stderr, "Only two pairs of coordinates should be provided.");
        exit(1);
    }
    double distance = H3_EXPORT(greatCircleDistanceM)(
        &polygon.geoloop.verts[0], &polygon.geoloop.verts[1]);
    printf("%.10lf\n", distance);
    for (int i = 0; i < polygon.numHoles; i++) {
        free(polygon.holes[i].verts);
    }
    free(polygon.holes);
    free(polygon.geoloop.verts);
    return E_SUCCESS;
}

SUBCOMMAND(describeH3Error,
           "Returns a description of the provided H3 error code number, or "
           "indicates the number is itself invalid.") {
    H3Error err = E_SUCCESS;
    Arg errArg = {.names = {"-e", "--error"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "CODE",
                  .value = &err,
                  .helpText = "H3 Error code to describe"};
    Arg *args[] = {&describeH3ErrorArg, &errArg, &helpArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    printf("%s\n", H3_EXPORT(describeH3Error)(err));
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
SUBCOMMAND_INDEX(getIndexDigit)
SUBCOMMAND_INDEX(constructCell)
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

/// Region subcommands
SUBCOMMAND_INDEX(polygonToCells)
SUBCOMMAND_INDEX(maxPolygonToCellsSize)
SUBCOMMAND_INDEX(cellsToMultiPolygon)

/// Directed Edge subcommands
SUBCOMMAND_INDEX(areNeighborCells)
SUBCOMMAND_INDEX(cellsToDirectedEdge)
SUBCOMMAND_INDEX(isValidDirectedEdge)
SUBCOMMAND_INDEX(getDirectedEdgeOrigin)
SUBCOMMAND_INDEX(getDirectedEdgeDestination)
SUBCOMMAND_INDEX(directedEdgeToCells)
SUBCOMMAND_INDEX(originToDirectedEdges)
SUBCOMMAND_INDEX(directedEdgeToBoundary)

/// Vertex subcommands
SUBCOMMAND_INDEX(cellToVertex)
SUBCOMMAND_INDEX(cellToVertexes)
SUBCOMMAND_INDEX(vertexToLatLng)
SUBCOMMAND_INDEX(isValidVertex)

/// Miscellaneous subcommands
SUBCOMMAND_INDEX(degsToRads)
SUBCOMMAND_INDEX(radsToDegs)
SUBCOMMAND_INDEX(getHexagonAreaAvgKm2)
SUBCOMMAND_INDEX(getHexagonAreaAvgM2)
SUBCOMMAND_INDEX(cellAreaRads2)
SUBCOMMAND_INDEX(cellAreaKm2)
SUBCOMMAND_INDEX(cellAreaM2)
SUBCOMMAND_INDEX(getHexagonEdgeLengthAvgKm)
SUBCOMMAND_INDEX(getHexagonEdgeLengthAvgM)
SUBCOMMAND_INDEX(edgeLengthRads)
SUBCOMMAND_INDEX(edgeLengthKm)
SUBCOMMAND_INDEX(edgeLengthM)
SUBCOMMAND_INDEX(getNumCells)
SUBCOMMAND_INDEX(getRes0Cells)
SUBCOMMAND_INDEX(getPentagons)
SUBCOMMAND_INDEX(pentagonCount)
SUBCOMMAND_INDEX(greatCircleDistanceRads)
SUBCOMMAND_INDEX(greatCircleDistanceKm)
SUBCOMMAND_INDEX(greatCircleDistanceM)
SUBCOMMAND_INDEX(describeH3Error)

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
