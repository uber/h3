/*
 * Copyright 2019 Uber Technologies, Inc.
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
 * @brief stdin/stdout filter that converts from local IJ coordinates to
 * H3 indexes. This is experimental.
 *
 *  See `localIjToH3 --help` for usage.
 *
 *  The program reads IJ coordinates (in the format `i j` separated by newlines)
 *  from stdin and outputs the corresponding H3 indexes to stdout, until EOF is
 *  encountered. `NA` is printed if the H3 index could not be obtained.
 *
 *  `origin` indicates the origin (or anchoring) index for the IJ coordinate
 *  space.
 *
 *  This program has the same limitations as the `experimentalLocalIjToH3`
 *  function.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "h3api.h"
#include "utility.h"

void doCell(const CoordIJ *ij, H3Index origin) {
    H3Index h;
    if (H3_EXPORT(experimentalLocalIjToH3)(origin, ij, &h)) {
        printf("NA\n");
    } else {
        h3Println(h);
    }
}

int main(int argc, char *argv[]) {
    H3Index origin = 0;
    CoordIJ ij = {0};

    Arg helpArg = ARG_HELP;
    Arg originArg = {
        .names = {"-o", "--origin"},
        .scanFormat = "%" PRIx64,
        .valueName = "origin",
        .value = &origin,
        .required = true,
        .helpText =
            "Origin (anchoring index) for the local coordinate system."};
    Arg iArg = {.names = {"-i", NULL},
                .scanFormat = "%d",
                .valueName = "i",
                .value = &ij.i,
                .helpText =
                    "I coordinate. If not specified \"i j\" pairs will be read "
                    "from standard input."};
    Arg jArg = {.names = {"-j", NULL},
                .scanFormat = "%d",
                .valueName = "j",
                .value = &ij.j,
                .helpText = "J coordinate."};

    Arg *args[] = {&helpArg, &originArg, &iArg, &jArg};
    const int numArgs = 4;
    const char *helpText = "Converts local IJ coordinates to H3 indexes";

    if (parseArgs(argc, argv, numArgs, args, &helpArg, helpText)) {
        return helpArg.found ? 0 : 1;
    }

    if (!H3_EXPORT(h3IsValid)(origin)) {
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "Origin is invalid.", NULL);
        return 1;
    }

    if (iArg.found != jArg.found) {
        // One is found but the other is not.
        printHelp(stderr, argv[0], helpText, numArgs, args,
                  "I and J must both be specified.", NULL);
        return 1;
    }

    if (iArg.found) {
        doCell(&ij, origin);
    } else {
        // process the coordinates on stdin
        char buff[BUFF_SIZE];
        while (1) {
            // get coordinates from stdin
            if (!fgets(buff, BUFF_SIZE, stdin)) {
                if (feof(stdin))
                    break;
                else
                    error("reading IJ coordinates from stdin");
            }

            if (!sscanf(buff, "%d %d", &ij.i, &ij.j))
                error(
                    "Parsing IJ coordinates. Expected `<integer> <integer>`.");

            doCell(&ij, origin);
        }
    }
}
