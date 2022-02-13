/*
 * Copyright 2016-2017, 2020-2021 Uber Technologies, Inc.
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
 * @brief tests H3 function `cellToBoundary`
 *
 *  usage: `testCellToBoundary`
 *
 *  The program reads H3 indexes and associated cell boundaries from
 *  stdin until EOF is encountered. For each input cell, the program
 *  uses `cellToBoundary` to generate the boundary and then compares that
 *  generated boundary to the original input boundary.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "baseCells.h"
#include "h3Index.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

/**
 * Assumes `f` is open and ready for reading.
 * @return 0 on success, EOF on EOF
 */
int readBoundary(FILE *f, CellBoundary *b) {
    char buff[BUFF_SIZE];

    // get the first line, which should be a "{"
    if (!fgets(buff, BUFF_SIZE, f)) {
        if (feof(stdin)) {
            return EOF;
        } else {
            printf("reading CellBoundary from input");
            return -1;
        }
    }

    if (buff[0] != '{') {
        printf("missing CellBoundary {");
        return -2;
    }

    // now read the verts

    b->numVerts = 0;
    while (1) {
        if (!fgets(buff, BUFF_SIZE, f)) {
            printf("reading CellBoundary from input");
            return -3;
        }

        if (buff[0] == '}') {
            if (b->numVerts == 0) {
                printf("reading empty cell boundary");
                return -4;
            } else {
                break;
            }
        }

        if (b->numVerts == MAX_CELL_BNDRY_VERTS) {
            printf("too many vertices in CellBoundary from input");
            return -5;
        }

        double latDegs, lngDegs;
        if (sscanf(buff, "%lf %lf", &latDegs, &lngDegs) != 2) {
            printf("parsing CellBoundary from input");
            return -6;
        }

        setGeoDegs(&b->verts[b->numVerts], latDegs, lngDegs);
        b->numVerts++;
    }

    return 0;
}

int main(int argc, char *argv[]) {
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

        H3Index h3;
        t_assertSuccess(H3_EXPORT(stringToH3)(buff, &h3));

        CellBoundary b;
        readBoundary(stdin, &b);

        t_assertBoundary(h3, &b);
    }
}
