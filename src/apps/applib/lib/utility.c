/*
 * Copyright 2016-2021 Uber Technologies, Inc.
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

#include "h3Index.h"
#include "iterators.h"

void error(const char *msg) {
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
void coordIjkPrint(const CoordIJK *c) {
    printf("[%d, %d, %d]", c->i, c->j, c->k);
}

/**
 * Assumes `str` is big enough to hold the result.
 */
void geoToStringRads(const LatLng *p, char *str) {
    sprintf(str, "(%.4lf, %.4lf)", p->lat, p->lng);
}

/**
 * Assumes `str` is big enough to hold the result.
 */
void geoToStringDegs(const LatLng *p, char *str) {
    sprintf(str, "(%.9lf, %.9lf)", H3_EXPORT(radsToDegs)(p->lat),
            H3_EXPORT(radsToDegs)(p->lng));
}

/**
 * Assumes `str` is big enough to hold the result.
 */
void geoToStringDegsNoFmt(const LatLng *p, char *str) {
    sprintf(str, "%.9lf %.9lf", H3_EXPORT(radsToDegs)(p->lat),
            H3_EXPORT(radsToDegs)(p->lng));
}

void geoPrint(const LatLng *p) {
    char buff[BUFF_SIZE];
    geoToStringDegs(p, buff);
    printf("%s", buff);
}

void geoPrintln(const LatLng *p) {
    geoPrint(p);
    printf("\n");
}

void geoPrintNoFmt(const LatLng *p) {
    char buff[BUFF_SIZE];
    geoToStringDegsNoFmt(p, buff);
    printf("%s", buff);
}

void geoPrintlnNoFmt(const LatLng *p) {
    geoPrintNoFmt(p);
    printf("\n");
}

void cellBoundaryPrint(const CellBoundary *b) {
    char buff[BUFF_SIZE];
    printf("{");
    for (int v = 0; v < b->numVerts; v++) {
        geoToStringDegsNoFmt(&b->verts[v], buff);
        printf("%s ", buff);
    }
    printf("}");
}

void cellBoundaryPrintln(const CellBoundary *b) {
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
void iterateAllDirectedEdgesAtRes(int res, void (*callback)(H3Index)) {
    for (IterCellsResolution iter = iterInitRes(res); iter.h;
         iterStepRes(&iter)) {
        H3Index edges[6] = {H3_NULL};
        H3_EXPORT(originToDirectedEdges)(iter.h, edges);

        for (int i = 0; i < 6; i++) {
            if (edges[i] != H3_NULL) {
                (*callback)(edges[i]);
            }
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
    for (IterCellsChildren iter = iterInitBaseCellNum(baseCell, res); iter.h;
         iterStepChild(&iter)) {
        (*callback)(iter.h);
    }
}

/**
 * Generates a random lat/lng pair.
 *
 * @param g Lat/lng will be placed here.
 */
void randomGeo(LatLng *g) {
    static int init = 0;
    if (!init) {
        srand((unsigned int)time(0));
        init = 1;
    }

    g->lat = H3_EXPORT(degsToRads)(
        (((float)rand() / (float)(RAND_MAX)) * 180.0) - 90.0);
    g->lng = H3_EXPORT(degsToRads)((float)rand() / (float)(RAND_MAX)) * 360.0;
}

/**
 * Returns the number of non-invalid indexes in the array.
 */
int64_t countNonNullIndexes(H3Index *indexes, int64_t numCells) {
    int64_t nonNullIndexes = 0;
    for (int i = 0; i < numCells; i++) {
        if (indexes[i] != H3_NULL) {
            nonNullIndexes++;
        }
    }
    return nonNullIndexes;
}
