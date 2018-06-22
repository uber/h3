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
/** @file
 * @brief stdin/stdout filter that converts from H3 indexes to relative IJK
 * coordinates.
 *
 *  usage: `h3ToIjk [origin]`
 *
 *  The program reads H3 indexes from stdin and outputs the corresponding
 *  IJK coordinates to stdout, until EOF is encountered. The H3 indexes
 *  should be in integer form. `-1 -1 -1` is printed if the IJK coordinates
 * could not be obtained.
 *
 *  `origin` indicates the origin (or anchoring) index for the IJK coordinate
 * space.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "h3api.h"
#include "utility.h"

void doCell(H3Index h, H3Index origin) {
    CoordIJK ijk;
    if (H3_EXPORT(h3ToIjk)(origin, h, &ijk)) {
        printf("-1 -1 -1\n");
    } else {
        printf("%d %d %d\n", ijk.i, ijk.j, ijk.k);
    }
}

int main(int argc, char *argv[]) {
    // check command line args
    if (argc != 2) {
        fprintf(stderr, "usage: %s [origin]\n", argv[0]);
        exit(1);
    }

    H3Index origin;

    if (!sscanf(argv[1], "%" PRIx64, &origin))
        error("origin could not be read");

    if (!H3_EXPORT(h3IsValid)(origin)) error("origin is invalid");

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
        doCell(h3, origin);
    }
}
