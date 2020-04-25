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
 * @brief takes an optional H3 index and generates all descendant cells at the
 * specified resolution.
 *
 *  See `h3ToHier --help` for usage.
 *
 *  The program generates all cells at the specified resolution, optionally
 *  only the children of the given index.
 *
 *  `resolution` should be a positive integer. The default is 0 (i.e., only the
 *       base cells).
 *
 *  `parent` should be an H3Index. By default, all indices at the specified
 *       resolution are generated.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "baseCells.h"
#include "h3Index.h"
#include "h3api.h"
#include "kml.h"
#include "utility.h"

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

int main(int argc, char *argv[]) {
    int res;
    H3Index parentIndex = 0;

    Arg helpArg = ARG_HELP;
    Arg resArg = {.names = {"-r", "--resolution"},
                  .scanFormat = "%d",
                  .valueName = "res",
                  .value = &res,
                  .required = true,
                  .helpText = "Resolution, 0-15 inclusive."};
    Arg parentArg = {
        .names = {"-p", "--parent"},
        .scanFormat = "%" PRIx64,
        .valueName = "parent",
        .value = &parentIndex,
        .helpText = "Print only indexes descendent from this index."};

    Arg *args[] = {&helpArg, &resArg, &parentArg};
    const int numArgs = 3;
    const char *helpText = "Print all indexes at the specified resolution";

    if (parseArgs(argc, argv, numArgs, args, &helpArg, helpText)) {
        return helpArg.found ? 0 : 1;
    }

    if (res > MAX_H3_RES) {
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "Resolution exceeds maximum resolution.", NULL);
        return 1;
    }

    if (parentArg.found && !H3_EXPORT(h3IsValid)(parentIndex)) {
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "Parent index is invalid.", NULL);
        return 1;
    }

    if (parentArg.found) {
        // parent is the same or higher resolution than the target.
        if (res <= H3_GET_RESOLUTION(parentIndex)) {
            h3Println(parentIndex);
        } else {
            int rootRes = H3_GET_RESOLUTION(parentIndex);
            H3_SET_RESOLUTION(parentIndex, res);
            recursiveH3IndexToHier(parentIndex, rootRes + 1);
        }
    } else {
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
    }
}
