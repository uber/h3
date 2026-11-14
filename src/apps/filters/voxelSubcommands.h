/*
 * Copyright 2026 J Reynolds
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
/** @file voxelSubcommands.h
 * @brief Voxel subcommands for the `h3` CLI.
 *
 * This is not a standalone header and must be included exactly once: `h3.c`
 * includes it after its CLI helpers (`SUBCOMMAND`, `PARSE_SUBCOMMAND`,
 * `helpArg`, `DEFINE_FORMAT_ARG`, ...) and before its first subcommand.
 * Keeping voxel subcommands here means merging upstream changes to `h3.c`
 * never touches the same lines (docs/adr/0006-coexist-with-upstream-h3.md).
 * The hooks in `h3.c` sit away from where upstream adds subcommands, so an
 * upstream addition merges without conflicts. The hooks are:
 *   - the `#include` of this file;
 *   - `VOXEL_SUBCOMMANDS_INDEX`, at the end of the subcommand index;
 *   - `DESCRIBE_CLI_ERROR`, used when the CLI reports an error.
 *
 * To add a voxel subcommand:
 *   1. define it below with `SUBCOMMAND(name, help) { ... }`, and
 *   2. append `SUBCOMMAND_INDEX(name)` to `VOXEL_SUBCOMMANDS_INDEX`, ending
 *      each line but the last with a backslash. Don't put `//` comments on
 *      those lines: they would swallow the backslash and break the macro.
 *
 * Helpers defined later in `h3.c` (e.g. `readCellsFromFile`) are not in
 * scope here.
 */

#ifndef VOXEL_SUBCOMMANDS_H
#define VOXEL_SUBCOMMANDS_H

#include "h3voxel.h"

/**
 * Describes any error the CLI reports, voxel error codes included; all other
 * codes are described exactly as upstream's `describeH3Error` does.
 */
#define DESCRIBE_CLI_ERROR(err) H3_EXPORT(describeVoxelError)(err)

#define DEFINE_VOXEL_ARG(varName, argName)     \
    H3Index varName = 0;                       \
    Arg argName = {.names = {"-v", "--voxel"}, \
                   .required = true,           \
                   .scanFormat = "%" PRIx64,   \
                   .valueName = "index",       \
                   .value = &varName,          \
                   .helpText = "H3 voxel"}

/** Prints an index as JSON ("hex") or as bare hex on its own line. */
static H3Error _printVoxelCliIndex(H3Index index, const char *format) {
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("\"%" PRIx64 "\"\n", index);
    } else if (strcmp(format, "newline") == 0) {
        h3Println(index);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(latLngAltToVoxel,
           "Convert degrees latitude/longitude and ellipsoidal altitude in "
           "metres to an H3 voxel") {
    DEFINE_FORMAT_ARG(
        "'json' for \"VOXEL\"\\n, 'newline' for VOXEL\\n "
        "(Default: json)");
    int res = 0;
    double lat = 0;
    double lng = 0;
    double alt = 0;

    Arg resArg = {.names = {"-r", "--resolution"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText = "Resolution, 0-10 inclusive."};
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
    Arg altArg = {.names = {"--alt", "--altitude"},
                  .required = true,
                  .scanFormat = "%lf",
                  .valueName = "alt",
                  .value = &alt,
                  .helpText = "Height above the WGS84 ellipsoid in metres."};

    Arg *args[] = {&latLngAltToVoxelArg,
                   &helpArg,
                   &resArg,
                   &latArg,
                   &lngArg,
                   &altArg,
                   &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    LatLng ll = {.lat = H3_EXPORT(degsToRads)(lat),
                 .lng = H3_EXPORT(degsToRads)(lng)};

    H3Index voxel;
    H3Error err = H3_EXPORT(latLngAltToVoxel)(&ll, alt, res, NULL, &voxel);
    if (err) {
        return err;
    }
    return _printVoxelCliIndex(voxel, format);
}

SUBCOMMAND(isValidVoxel, "Checks if the provided H3 index is a valid voxel") {
    DEFINE_FORMAT_ARG(
        "'json' for true or false, 'numeric' for 1 or 0 (Default: json)");
    DEFINE_VOXEL_ARG(voxel, voxelArg);
    Arg *args[] = {&isValidVoxelArg, &helpArg, &voxelArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    bool isValid = H3_EXPORT(isValidVoxel)(voxel);
    if (strcmp(format, "json") == 0 || strcmp(format, "") == 0) {
        printf("%s\n", isValid ? "true" : "false");
    } else if (strcmp(format, "numeric") == 0) {
        printf("%d\n", isValid);
    } else {
        return E_FAILED;
    }
    return E_SUCCESS;
}

SUBCOMMAND(getVoxelResolution,
           "Extracts the resolution (0 - 10) from the H3 voxel") {
    DEFINE_VOXEL_ARG(voxel, voxelArg);
    Arg *args[] = {&getVoxelResolutionArg, &helpArg, &voxelArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int res;
    H3Error err = H3_EXPORT(getVoxelResolution)(voxel, &res);
    if (err) {
        return err;
    }
    printf("%d\n", res);
    return E_SUCCESS;
}

SUBCOMMAND(voxelToFootprint, "Returns the H3 cell an H3 voxel lies over") {
    DEFINE_FORMAT_ARG(
        "'json' for \"CELL\"\\n, 'newline' for CELL\\n "
        "(Default: json)");
    DEFINE_VOXEL_ARG(voxel, voxelArg);
    Arg *args[] = {&voxelToFootprintArg, &helpArg, &voxelArg, &formatArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    H3Index footprint;
    H3Error err = H3_EXPORT(voxelToFootprint)(voxel, &footprint);
    if (err) {
        return err;
    }
    return _printVoxelCliIndex(footprint, format);
}

SUBCOMMAND(getLayerNumber,
           "Returns the H3 voxel's layer number, counted from altitude zero "
           "(layer 0 is the first above zero, layer -1 the first below)") {
    DEFINE_VOXEL_ARG(voxel, voxelArg);
    Arg *args[] = {&getLayerNumberArg, &helpArg, &voxelArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    int32_t layer;
    H3Error err = H3_EXPORT(getLayerNumber)(voxel, &layer);
    if (err) {
        return err;
    }
    printf("%" PRId32 "\n", layer);
    return E_SUCCESS;
}

SUBCOMMAND(describeVoxelError,
           "Returns a description of the provided H3 or voxel error code "
           "number, or indicates the number is itself invalid.") {
    H3Error err = E_SUCCESS;
    Arg errArg = {.names = {"-e", "--error"},
                  .required = true,
                  .scanFormat = "%d",
                  .valueName = "CODE",
                  .value = &err,
                  .helpText = "H3 or voxel error code to describe"};
    Arg *args[] = {&describeVoxelErrorArg, &helpArg, &errArg};
    PARSE_SUBCOMMAND(argc, argv, args);
    printf("%s\n", H3_EXPORT(describeVoxelError)(err));
    return E_SUCCESS;
}

/**
 * Subcommand index entries for voxel subcommands. Expanded inside `h3.c`'s
 * subcommand index after all upstream entries, so voxel subcommands are
 * listed last in `h3 --help`.
 */
#define VOXEL_SUBCOMMANDS_INDEX          \
    SUBCOMMAND_INDEX(latLngAltToVoxel)   \
    SUBCOMMAND_INDEX(isValidVoxel)       \
    SUBCOMMAND_INDEX(getVoxelResolution) \
    SUBCOMMAND_INDEX(voxelToFootprint)   \
    SUBCOMMAND_INDEX(getLayerNumber)     \
    SUBCOMMAND_INDEX(describeVoxelError)

#endif
