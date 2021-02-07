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
#include "childIter.h"

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
 * Apply callback for every unidirectional edge at the given resolution.
 */
void iterateAllUnidirectionalEdgesAtRes(int res, void (*callback)(H3Index)) {
    H3Index baseCell;
    ChildIter CI;

    for (int cellNum = 0; cellNum < 122; cellNum++) {
        setH3Index(&baseCell, 0, cellNum, 0);
        setup(&CI, baseCell, res);

        for (int i = 0; CI.h; i++) {
            H3Index edges[6] = {H3_NULL};
            int isPentagon = H3_EXPORT(h3IsPentagon)(CI.h);
            H3_EXPORT(getH3UnidirectionalEdgesFromHexagon)(CI.h, edges);

            for (int j = 0; j < 6; j++) {  // do we skip 0 or do we skip 1?
                if (isPentagon && j == 0) continue;
                (*callback)(edges[j]);
            }

            step(&CI);
        }
    }
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
    H3Index bc = H3_INIT;
    H3_SET_MODE(bc, H3_HEXAGON_MODE);
    H3_SET_RESOLUTION(bc, 0);
    H3_SET_BASE_CELL(bc, baseCell);
    int64_t childrenSz = H3_EXPORT(uncompactSize)(&bc, 1, res);
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
