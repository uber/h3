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
 *  See `kRing --help` for usage.
 *
 *  The program reads H3 indexes from stdin until EOF and outputs
 *  the H3 indexes within k-ring `k` to stdout.
 *
 *  --print-distances may be specified to also print the grid distance
 *  from the origin index.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "h3api.h"
#include "utility.h"

void doCell(H3Index h, int k, int printDistances) {
    int maxSize = H3_EXPORT(maxKringSize)(k);
    H3Index* rings = calloc(maxSize, sizeof(H3Index));
    int* distances = calloc(maxSize, sizeof(int));
    H3_EXPORT(kRingDistances)(h, k, rings, distances);

    for (int i = 0; i < maxSize; i++) {
        if (rings[i] != 0) {
            h3Print(rings[i]);
            if (printDistances) {
                printf(" %d\n", distances[i]);
            } else {
                printf("\n");
            }
        }
    }

    free(distances);
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
    Arg printDistancesArg = {
        .names = {"-d", "--print-distances"},
        .helpText = "Print distance from origin after each index."};
    Arg originArg = {
        .names = {"-o", "--origin"},
        .scanFormat = "%" PRIx64,
        .valueName = "origin",
        .value = &origin,
        .helpText =
            "Origin, or not specified to read origins from standard input."};

    Arg* args[] = {&helpArg, &kArg, &printDistancesArg, &originArg};

    if (parseArgs(argc, argv, 4, args, &helpArg,
                  "Print indexes k distance away from the origin")) {
        return helpArg.found ? 0 : 1;
    }

    if (originArg.found) {
        doCell(origin, k, printDistancesArg.found);
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
            doCell(h3, k, printDistancesArg.found);
        }
    }
}
