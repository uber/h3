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
 *  usage: `localIjToH3 [origin]`
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

    Arg helpArg = {.names = {"-h", "--help"},
                   .helpText = "Show this help message."};
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
                .helpText = "I coordinate."};
    Arg jArg = {.names = {"-j", NULL},
                .scanFormat = "%d",
                .valueName = "j",
                .value = &ij.j,
                .helpText = "J coordinate."};

    Arg *args[] = {&helpArg, &originArg, &iArg, &jArg};

    if (parseArgs(argc, argv, 4, args, &helpArg,
                  "Converts local IJ coordinates to H3 indexes.")) {
        return helpArg.found ? 0 : 1;
    }

    if (!H3_EXPORT(h3IsValid)(origin)) {
        // TODO print help here
        error("origin is invalid");
    }

    if (iArg.found != jArg.found) {
        // TODO print help here
        error("I and J must both be specified");
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
                error("Parsing IJ coordinates. Expected `i j`.");

            doCell(&ij, origin);
        }
    }
}
