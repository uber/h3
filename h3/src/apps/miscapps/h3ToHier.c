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
 * @brief takes an optional H3 index and generates all descendant cells at the
 * specified resolution.
 *
 *  usage: `h3ToHier [resolution H3Index]`
 *
 *  The program generates all cells at the specified resolution, optionally
 *  only the children of the given index.
 *
 *  `resolution` should be a positive integer. The default is 0 (i.e., only the
 *       base cells).
 *
 *  `H3Index` should be an H3Index. By default, all indices at the specified
 *       resolution are generated.
 */

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

#include "algos.h"

void recursiveH3IndexToHier(H3Index h, int res) {
    for (int d = 0; d < 7; d++) {
        H3_SET_INDEX_DIGIT(h, res, d);

        // skip the pentagonal deleted subsequence

        if (_isBaseCellPentagon(H3_GET_BASE_CELL(h)) &&
            _h3LeadingNonZeroDigit(h) == 1) {
            continue;
        }

        if (res == H3_GET_RESOLUTION(h)) {
            h3Println(h);
        } else {
            recursiveH3IndexToHier(h, res + 1);
        }
    }
}

int main(int argc, char* argv[]) {
    // check command line args
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s [resolution H3Index]\n", argv[0]);
        exit(1);
    }

    int res = 0;
    if (!sscanf(argv[1], "%d", &res)) error("resolution must be an integer");

    if (res > MAX_H3_RES) error("specified resolution exceeds max resolution");

    H3Index prefixIndex = 0;
    if (argc > 2) {
        prefixIndex = H3_EXPORT(stringToH3)(argv[2]);
        int h3BaseCell = H3_GET_BASE_CELL(prefixIndex);
        if (h3BaseCell < 0 || h3BaseCell >= NUM_BASE_CELLS) {
            error("invalid base cell number");
        }
    }

    if (prefixIndex == 0) {
        // Generate all
        for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
            H3Index rootCell = H3_INIT;
            H3_SET_MODE(rootCell, H3_HEXAGON_MODE);
            H3_SET_BASE_CELL(rootCell, bc);
            if (res == 0) {
                h3Println(rootCell);
            } else {
                H3_SET_RESOLUTION(rootCell, res);
                recursiveH3IndexToHier(rootCell, 1);
            }
        }
    } else {
        // prefix is the same or higher resolution than
        // the target.
        if (res <= H3_GET_RESOLUTION(prefixIndex)) {
            h3Println(prefixIndex);
        } else {
            int rootRes = H3_GET_RESOLUTION(prefixIndex);
            H3_SET_RESOLUTION(prefixIndex, res);
            recursiveH3IndexToHier(prefixIndex, rootRes + 1);
        }
    }
}
