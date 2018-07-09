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
 * @brief takes an H3 index and generates cell center points for descendants a
 * specified resolution.
 *
 *  usage: `h3ToGeoHier H3Index [resolution outputMode]`
 *
 *  The program generates the cell center points in lat/lon coordinates for all
 *  hierarchical children of H3Index at the specified resolution. If the
 *  specified resolution is less than or equal to the resolution of H3Index
 *  the single cell H3Index is processed.
 *
 *  `resolution` should be a positive integer. The default is 0 (i.e., only the
 *       specified cell H3Index would be processed).
 *
 *  `outputMode` indicates the type of output; currently the choices are 0 for
 *       plain text output (the default) and 1 for KML output.
 *
 *  Examples:
 *  ---------
 *
 *     `h3ToGeoHier 836e9bfffffffff`
 *        - outputs the cell center point in lat/lon for cell
 *          `836e9bfffffffff` as plain text
 *
 *     `h3ToGeoHier 820ceffffffffff 4 1 > pts.kml`
 *        - outputs the cell center points of all of the resolution 4
 *          descendants of cell `820ceffffffffff` as a KML file (redirected to
 *          `pts.kml`).
 *
 *     `h3ToGeoHier 86283082fffffff 9 1 > uber9pts.kml`
 *        - creates a KML file containing the cell center points of all of the
 *          resolution 9 hexagons covering Uber HQ and the surrounding region of
 *          San Francisco.
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
    GeoCoord g;
    H3_EXPORT(h3ToGeo)(h, &g);

    char label[BUFF_SIZE];
    H3_EXPORT(h3ToString)(h, label, BUFF_SIZE);

    if (isKmlOut) {
        outputPointKML(&g, label);
    } else {
        printf("%s ", label);
        geoPrintlnNoFmt(&g);
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

int main(int argc, char* argv[]) {
    // check command line args
    if (argc < 2 || argc > 5) {
        fprintf(stderr, "usage: %s H3Index [resolution outputMode]\n", argv[0]);
        exit(1);
    }

    H3Index rootCell = H3_EXPORT(stringToH3)(argv[1]);
    int baseCell = H3_GET_BASE_CELL(rootCell);
    int rootRes = H3_GET_RESOLUTION(rootCell);
    if (baseCell < 0 || baseCell >= NUM_BASE_CELLS) {
        error("invalid base cell number");
    }

    int res = 0;
    int isKmlOut = 0;
    if (argc > 2) {
        if (!sscanf(argv[2], "%d", &res))
            error("resolution must be an integer");

        if (res > MAX_H3_RES)
            error("specified resolution exceeds max resolution");

        if (argc > 3) {
            if (!sscanf(argv[3], "%d", &isKmlOut))
                error("outputMode must be an integer");

            if (isKmlOut != 0 && isKmlOut != 1)
                error("outputMode must be 0 or 1");

            if (isKmlOut) {
                char name[BUFF_SIZE];

                sprintf(name, "Cell %" PRIx64 " Res %d", rootCell,
                        ((res <= rootRes) ? rootRes : res));

                kmlBoundaryHeader(name, "cell center");
            }
        }
    }

    // generate the points

    if (res <= rootRes) {
        doCell(rootCell, isKmlOut);
    } else {
        H3_SET_RESOLUTION(rootCell, res);
        recursiveH3IndexToGeo(rootCell, rootRes + 1, isKmlOut);
    }

    if (isKmlOut) kmlBoundaryFooter();
}
