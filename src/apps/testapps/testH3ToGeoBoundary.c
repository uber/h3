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
 * @brief tests H3 function `h3ToGeoBoundary`
 *
 *  usage: `testH3ToGeoBoundary`
 *
 *  The program reads H3 indexes and associated cell boundaries from
 *  stdin until EOF is encountered. For each input cell, the program
 *  uses `h3ToGeoBoundary` to generate the boundary and then compares that
 *  generated boundary to the original input boundary.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "baseCells.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

int main(int argc, char* argv[]) {
    // check command line args
    if (argc > 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(1);
    }

    // process the indexes on stdin
    char buff[BUFF_SIZE];
    while (1) {
        // get an index from stdin
        if (!fgets(buff, BUFF_SIZE, stdin)) {
            if (feof(stdin))
                break;
            else
                error("reading input H3 index from stdin");
        }

        H3Index h3 = H3_EXPORT(stringToH3)(buff);

        GeoBoundary b;
        readBoundary(stdin, &b);

        t_assertBoundary(h3, &b);
    }
}
