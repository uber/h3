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
 * @brief stdin/stdout filter that converts from integer H3 indexes to
 * k-rings
 *
 *  See `hexRange --help` for usage.
 *
 *  The program reads H3 indexes from stdin until EOF and outputs
 *  the H3 indexes within k-ring `k` to stdout. Requires all indexes
 *  being evaluated to be hexagons, and produces output in a spiralling
 *  order.
 *
 *  If a pentagon or pentagon distortion is encountered, 0 is printed
 *  as the only output.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "h3api.h"
#include "utility.h"

void doCell(H3Index h, int k) {
    int maxSize = H3_EXPORT(maxKringSize)(k);
    H3Index* rings = calloc(maxSize, sizeof(H3Index));

    if (!H3_EXPORT(hexRange)(h, k, rings)) {
        for (int i = 0; i < maxSize; i++) {
            h3Println(rings[i]);
        }
    } else {
        printf("0\n");
    }

    free(rings);
}

int main(int argc, char* argv[]) {
    int k = 0;
    H3Index origin = 0;

    Arg helpArg = ARG_HELP;
    Arg kArg = {.names = {"-k", NULL},
                .required = true,
                .scanFormat = "%d",
                .valueName = "k",
                .value = &k,
                .helpText = "Radius in hexagons."};
    Arg originArg = {
        .names = {"-o", "--origin"},
        .scanFormat = "%" PRIx64,
        .valueName = "origin",
        .value = &origin,
        .helpText =
            "Origin, or not specified to read origins from standard input."};
    const int numArgs = 3;
    Arg* args[] = {&helpArg, &kArg, &originArg};

    if (parseArgs(argc, argv, numArgs, args, &helpArg,
                  "Print indexes k distance away from the origin")) {
        return helpArg.found ? 0 : 1;
    }

    if (originArg.found) {
        doCell(origin, k);
    } else {
        // process the indexes on stdin
        char buff[BUFF_SIZE];
        while (1) {
            // get an index from stdin
            if (!fgets(buff, BUFF_SIZE, stdin)) {
                if (feof(stdin))
                    break;
                else
                    error("reading H3 index from stdin");
            }

            H3Index h3 = H3_EXPORT(stringToH3)(buff);
            doCell(h3, k);
        }
    }
}
