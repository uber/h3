/*
 * Copyright 2018 Uber Technologies, Inc.
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
/** @file generateNumHexagons.c
 * @brief Generates the tables for numHexagons.
 *
 *  usage: `generateNumHexagons`
 *
 *  This program generates the number of unique indexes (not necessarily
 *  hexagons) at each H3 resolution. It assumes aperture 7 and 12 pentagons
 *  per resolution.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"

/**
 * Generates and prints the numHexagons table.
 */
static void generate() {
    printf("static const int64_t nums[] = {\n");
    int64_t count = NUM_BASE_CELLS;
    for (int i = 0; i <= MAX_H3_RES; i++) {
        printf("    %" PRId64 "L,\n", count);
        // Each hexagon has 7 children, and each pentagon has 6 children
        count = ((count - NUM_PENTAGONS) * 7) + (NUM_PENTAGONS * 6);
    }
    printf("};\n");
}

int main(int argc, char* argv[]) {
    // check command line args
    if (argc > 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(1);
    }

    generate();
}
