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
 * @brief takes an H3 index and generates cell boundaries for all descendants
 * at a specified resolution.
 *
 *  usage: `h3ToGeoBoundaryHier --prefix prefix [--resolution res] [--kml
 * [--kml-name name] [--kml-description desc]]`
 *
 *  The program generates the cell boundaries in lat/lon coordinates for all
 *  hierarchical children of H3Index at the specified resolution. If the
 *  specified resolution is less than or equal to the resolution of H3Index
 *  the single cell H3Index is processed.
 *
 *  `resolution` should be a positive integer. The default is 0 (i.e., only the
 *       specified cell H3Index would be processed).
 *
 *  `--kml` indicates KML output format; if not specified plain text output is
 *       the default.
 *
 *  Examples:
 *  ---------
 *
 *     `h3ToGeoBoundaryHier --prefix 836e9bfffffffff`
 *        - outputs the cell boundary in lat/lon for cell `836e9bfffffffff` as
 *          plain text
 *
 *     `h3ToGeoBoundaryHier --prefix 820ceffffffffff --resolution 4 --kml >
 *          cells.kml`
 *        - outputs the cell boundaries of all of the resolution 4 descendants
 *          of cell `820ceffffffffff` as a KML file (redirected to `cells.kml`).
 *
 *     `h3ToGeoBoundaryHier --prefix 86283082fffffff --resolution 9 --kml >
 *          uber9cells.kml`
 *        - creates a KML file containing the cell boundaries of all of the
 *          resolution 9 hexagons covering Uber HQ and the surrounding region of
 *          San Francisco
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "baseCells.h"
#include "coordijk.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "h3api.h"
#include "kml.h"
#include "utility.h"
#include "vec2d.h"

void doCell(H3Index h, int isKmlOut) {
    GeoBoundary b;
    H3_EXPORT(h3ToGeoBoundary)(h, &b);

    char label[BUFF_SIZE];
    H3_EXPORT(h3ToString)(h, label, BUFF_SIZE);

    if (isKmlOut) {
        outputBoundaryKML(&b, label);
    } else {
        printf("%s\n", label);
        geoBoundaryPrintln(&b);
    }
}

void recursiveH3IndexToGeo(H3Index h, int res, int isKmlOut) {
    for (int d = 0; d < 7; d++) {
        H3_SET_INDEX_DIGIT(h, res, d);

        // skip the pentagonal deleted subsequence

        if (_isBaseCellPentagon(H3_GET_BASE_CELL(h)) &&
            _h3LeadingNonZeroDigit(h) == 1) {
            continue;
        }

        if (res == H3_GET_RESOLUTION(h)) {
            doCell(h, isKmlOut);
        } else {
            recursiveH3IndexToGeo(h, res + 1, isKmlOut);
        }
    }
}

int main(int argc, char *argv[]) {
    int res;
    H3Index prefixIndex = 0;
    char userKmlName[BUFF_SIZE] = {0};
    char userKmlDesc[BUFF_SIZE] = {0};

    Arg helpArg = {.names = {"-h", "--help"},
                   .helpText = "Show this help message."};
    Arg resArg = {.names = {"-r", "--resolution"},
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .helpText =
                      "Resolution, if less than the resolution of the prefix "
                      "only the prefix is printed. Default 0."};
    Arg prefixArg = {
        .names = {"-p", "--prefix"},
        .scanFormat = "%" PRIx64,
        .valueName = "prefix",
        .value = &prefixIndex,
        .required = true,
        .helpText = "Print cell boundaries descendent from this index."};
    Arg kmlArg = {.names = {"-k", "--kml"},
                  .helpText = "Print output in KML format."};
    Arg kmlNameArg = {.names = {"--kn", "--kml-name"},
                      .scanFormat = "%255c",  // BUFF_SIZE - 1
                      .valueName = "name",
                      .value = &userKmlName,
                      .helpText = "Text for the KML name tag."};
    Arg kmlDescArg = {.names = {"--kd", "--kml-description"},
                      .scanFormat = "%255c",  // BUFF_SIZE - 1
                      .valueName = "description",
                      .value = &userKmlDesc,
                      .helpText = "Text for the KML description tag."};

    Arg *args[] = {&helpArg, &resArg,     &prefixArg,
                   &kmlArg,  &kmlNameArg, &kmlDescArg};
    const int numArgs = 6;
    const char *helpText = "Print cell boundaries for descendants of an index";

    if (parseArgs(argc, argv, numArgs, args, &helpArg, helpText)) {
        return helpArg.found ? 0 : 1;
    }

    if (res > MAX_H3_RES) {
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "Resolution exceeds maximum resolution.", NULL);
        return 1;
    }

    if (!H3_EXPORT(h3IsValid)(prefixIndex)) {
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "Prefix index is invalid.", NULL);
        return 1;
    }

    int rootRes = H3_GET_RESOLUTION(prefixIndex);

    if (kmlArg.found) {
        char *kmlName;
        if (kmlNameArg.found) {
            kmlName = strdup(userKmlName);
        } else {
            kmlName = calloc(BUFF_SIZE, sizeof(char));

            sprintf(kmlName, "Cell %" PRIx64 " Res %d", prefixIndex,
                    ((res <= rootRes) ? rootRes : res));
        }

        char *kmlDesc = "Generated by h3ToGeoBoundaryHier";
        if (kmlDescArg.found) kmlDesc = userKmlDesc;

        kmlBoundaryHeader(kmlName, kmlDesc);

        free(kmlName);
    }

    // generate the points
    if (res <= rootRes) {
        doCell(prefixIndex, kmlArg.found);
    } else {
        H3_SET_RESOLUTION(prefixIndex, res);
        recursiveH3IndexToGeo(prefixIndex, rootRes + 1, kmlArg.found);
    }

    if (kmlArg.found) kmlBoundaryFooter();
}
