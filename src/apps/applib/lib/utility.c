/*
 * Copyright 2016-2020 Uber Technologies, Inc.
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
/** @file utility.c
 * @brief   Miscellaneous useful functions.
 */

#include "utility.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "coordijk.h"
#include "h3Index.h"
#include "h3api.h"

void error(const char* msg) {
    fflush(stdout);
    fflush(stderr);
    fprintf(stderr, "ERROR: %s.\n", msg);
    exit(1);
}

/**
 * Prints the H3Index
 */
void h3Print(H3Index h) { printf("%" PRIx64, h); }

/**
 * Prints the H3Index and a newline
 */
void h3Println(H3Index h) { printf("%" PRIx64 "\n", h); }

/**
 * Prints the CoordIJK
 */
void coordIjkPrint(const CoordIJK* c) {
    printf("[%d, %d, %d]", c->i, c->j, c->k);
}

/**
 * Assumes `str` is big enough to hold the result.
 */
void geoToStringRads(const GeoCoord* p, char* str) {
    sprintf(str, "(%.4lf, %.4lf)", p->lat, p->lon);
}

/**
 * Assumes `str` is big enough to hold the result.
 */
void geoToStringDegs(const GeoCoord* p, char* str) {
    sprintf(str, "(%.9lf, %.9lf)", H3_EXPORT(radsToDegs)(p->lat),
            H3_EXPORT(radsToDegs)(p->lon));
}

/**
 * Assumes `str` is big enough to hold the result.
 */
void geoToStringDegsNoFmt(const GeoCoord* p, char* str) {
    sprintf(str, "%.9lf %.9lf", H3_EXPORT(radsToDegs)(p->lat),
            H3_EXPORT(radsToDegs)(p->lon));
}

void geoPrint(const GeoCoord* p) {
    char buff[BUFF_SIZE];
    geoToStringDegs(p, buff);
    printf("%s", buff);
}

void geoPrintln(const GeoCoord* p) {
    geoPrint(p);
    printf("\n");
}

void geoPrintNoFmt(const GeoCoord* p) {
    char buff[BUFF_SIZE];
    geoToStringDegsNoFmt(p, buff);
    printf("%s", buff);
}

void geoPrintlnNoFmt(const GeoCoord* p) {
    geoPrintNoFmt(p);
    printf("\n");
}

void geoBoundaryPrint(const GeoBoundary* b) {
    char buff[BUFF_SIZE];
    printf("{");
    for (int v = 0; v < b->numVerts; v++) {
        geoToStringDegsNoFmt(&b->verts[v], buff);
        printf("%s ", buff);
    }
    printf("}");
}

void geoBoundaryPrintln(const GeoBoundary* b) {
    char buff[BUFF_SIZE];
    printf("{\n");
    for (int v = 0; v < b->numVerts; v++) {
        geoToStringDegsNoFmt(&b->verts[v], buff);
        printf("   %s\n", buff);
    }
    printf("}\n");
}

/**
 * Assumes `f` is open and ready for reading.
 * @return 0 on success, EOF on EOF
 */
int readBoundary(FILE* f, GeoBoundary* b) {
    char buff[BUFF_SIZE];

    // get the first line, which should be a "{"
    if (!fgets(buff, BUFF_SIZE, f)) {
        if (feof(stdin)) {
            return EOF;
        } else {
            printf("reading GeoBoundary from input");
            return -1;
        }
    }

    if (buff[0] != '{') {
        printf("missing GeoBoundary {");
        return -2;
    }

    // now read the verts

    b->numVerts = 0;
    while (1) {
        if (!fgets(buff, BUFF_SIZE, f)) {
            printf("reading GeoBoundary from input");
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
            printf("too many vertices in GeoBoundary from input");
            return -5;
        }

        double latDegs, lonDegs;
        if (sscanf(buff, "%lf %lf", &latDegs, &lonDegs) != 2) {
            printf("parsing GeoBoundary from input");
            return -6;
        }

        setGeoDegs(&b->verts[b->numVerts], latDegs, lonDegs);
        b->numVerts++;
    }

    return 0;
}

/**
 * Move nonzero elements to the front of array `a` of length `n`.
 *
 * Loop invariant: Everything *before* `i` or *after* `j` is "done".
 * Move `i` and `j` inwards until they equal, and exit.
 * You can move `i` forward until there's a zero in front of it.
 * You can move `j` backward until there's a nonzero to the left of it.
 * Anything to the right of `j` is "junk" that can be reallocated.
 *
 * Before:
 *   | a | b | 0 | c | d | ... |
 *           ^           ^
 *           i           j
 * After:
 *   | a | b | d | c | d | ... |
 *           ^       ^
 *           i       j
 *
 * todo: should this function be in the public API?
 * todo: add tests for this function
 *
 * @param   a  H3Index array to whose elements will be moved
 * @param   n  length of the input array
 * @return     number of nonzero elements (length of new array); can reallocate
 *             memory after this point
 */
size_t packNonzeros(H3Index* a, size_t n) {
    size_t i = 0;
    size_t j = n;

    while (i < j) {
        // move j to the left until the first nonzero
        if (a[j - 1] == 0) {
            j -= 1;
            continue;
        }

        // move i to the right until the first zero
        if (a[i] != 0) {
            i += 1;
            continue;
        }

        // if we get to this point, we know:
        // a[i] == 0
        // a[j-1] != 0
        // i < j
        // so we can swap! (actually, move a[j-1] -> a[i])
        a[i] = a[j - 1];
        j -= 1;
    }

    return i;
}

/**
 * Array of all cells at a given resolution.
 *
 * @param   res  resolution
 *
 * @return       array of H3 cells at resolution res
 */
H3Index* getCellsAtRes(int res) {
    int num0 = H3_EXPORT(res0IndexCount)();
    H3Index* cells0 = calloc(num0, sizeof(H3Index));
    H3_EXPORT(getRes0Indexes)(cells0);

    int numRes = H3_EXPORT(maxUncompactSize)(cells0, num0, res);

    H3Index* cellsRes = calloc(numRes, sizeof(H3Index));
    H3_EXPORT(uncompact)(cells0, num0, cellsRes, numRes, res);

    free(cells0);

    numRes = packNonzeros(cellsRes, numRes);
    cellsRes = realloc(cellsRes, numRes * sizeof(H3Index));

    return cellsRes;
}

/**
 * Apply callback to every cell for a given resolution, and sum the results.
 */
double mapSumAllCells_double(int res, double (*callback)(H3Index)) {
    H3Index* cells = getCellsAtRes(res);
    int N = H3_EXPORT(numHexagons)(res);

    double total = 0.0;
    for (int i = 0; i < N; i++) {
        total += (*callback)(cells[i]);
    }
    free(cells);

    return total;
}

/**
 * Apply callback for every unidirectional edge at the given resolution.
 */
void iterateAllUnidirectionalEdgesAtRes(int res, void (*callback)(H3Index)) {
    H3Index* cells = getCellsAtRes(res);
    int N = H3_EXPORT(numHexagons)(res);

    for (int i = 0; i < N; i++) {
        H3Index edges[6] = {H3_NULL};
        int isPentagon = H3_EXPORT(h3IsPentagon)(cells[i]);
        H3_EXPORT(getH3UnidirectionalEdgesFromHexagon)(cells[i], edges);

        for (int j = 0; j < 6; j++) {
            if (isPentagon && j == 0) continue;
            (*callback)(edges[j]);
        }
    }
    free(cells);
}

/**
 * Call the callback for every index at the given resolution.
 */
void iterateAllIndexesAtRes(int res, void (*callback)(H3Index)) {
    iterateAllIndexesAtResPartial(res, callback, NUM_BASE_CELLS);
}

/**
 * Call the callback for every index at the given resolution in base
 * cell 0 up to the given base cell number.
 */
void iterateAllIndexesAtResPartial(int res, void (*callback)(H3Index),
                                   int baseCells) {
    assert(baseCells <= NUM_BASE_CELLS);
    for (int i = 0; i < baseCells; i++) {
        iterateBaseCellIndexesAtRes(res, callback, i);
    }
}

/**
 * Call the callback for every index at the given resolution in a
 * specific base cell
 */
void iterateBaseCellIndexesAtRes(int res, void (*callback)(H3Index),
                                 int baseCell) {
    H3Index bc;
    setH3Index(&bc, 0, baseCell, 0);
    int childrenSz = H3_EXPORT(maxUncompactSize)(&bc, 1, res);
    H3Index* children = calloc(childrenSz, sizeof(H3Index));
    H3_EXPORT(uncompact)(&bc, 1, children, childrenSz, res);

    for (int j = 0; j < childrenSz; j++) {
        if (children[j] == H3_NULL) {
            continue;
        }

        (*callback)(children[j]);
    }

    free(children);
}

/**
 * Generates a random lat/lon pair.
 *
 * @param g Lat/lon will be placed here.
 */
void randomGeo(GeoCoord* g) {
    static int init = 0;
    if (!init) {
        srand((unsigned int)time(0));
        init = 1;
    }

    g->lat = H3_EXPORT(degsToRads)(
        (((float)rand() / (float)(RAND_MAX)) * 180.0) - 90.0);
    g->lon = H3_EXPORT(degsToRads)((float)rand() / (float)(RAND_MAX)) * 360.0;
}

/**
 * Returns the number of non-invalid indexes in the array.
 */
int countActualHexagons(H3Index* hexagons, int numHexagons) {
    int actualNumHexagons = 0;
    for (int i = 0; i < numHexagons; i++) {
        if (hexagons[i] != H3_NULL) {
            actualNumHexagons++;
        }
    }
    return actualNumHexagons;
}
