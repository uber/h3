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
/** @file utility.h
 * @brief   Miscellaneous functions and constants.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>

#include "coordijk.h"
#include "h3api.h"

#define BUFF_SIZE 256

/** Macro: Get the size of a fixed-size array */
#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

void error(const char* msg);
void h3Print(H3Index h);    // prints as integer
void h3Println(H3Index h);  // prints as integer

void coordIjkPrint(const CoordIJK* c);

void geoToStringRads(const GeoCoord* p, char* str);
void geoToStringDegs(const GeoCoord* p, char* str);
void geoToStringDegsNoFmt(const GeoCoord* p, char* str);

void geoPrint(const GeoCoord* p);
void geoPrintln(const GeoCoord* p);
void geoPrintNoFmt(const GeoCoord* p);
void geoPrintlnNoFmt(const GeoCoord* p);
void geoBoundaryPrint(const GeoBoundary* b);
void geoBoundaryPrintln(const GeoBoundary* b);
int readBoundary(FILE* f, GeoBoundary* b);

void randomGeo(GeoCoord* p);

void iterateAllIndexesAtRes(int res, void (*callback)(H3Index));
void iterateAllIndexesAtResPartial(int res, void (*callback)(H3Index),
                                   int maxBaseCell);
void iterateBaseCellIndexesAtRes(int res, void (*callback)(H3Index),
                                 int baseCell);
void iterateAllUnidirectionalEdgesAtRes(int res, void (*callback)(H3Index));
double mapSumAllCells_double(int res, double (*callback)(H3Index));
size_t packNonzeros(H3Index* a, size_t n);

int countActualHexagons(H3Index* hexagons, int numHexagons);

#endif
