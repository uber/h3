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
/** @file h3IndexFat.h
 * @brief   H3IndexFat functions and definitions.
 */

#ifndef H3INDEXFAT_H
#define H3INDEXFAT_H

#include "constants.h"
#include "coordijk.h"
#include "faceijk.h"
#include "geoCoord.h"
#include "vec2d.h"

/** @struct H3IndexFat
 *  @brief heavyweight H3 index representation
 */
typedef struct {
    int mode;               ///< index mode
    int res;                ///< cell resolution
    int baseCell;           ///< base cell number 0-121
    int index[MAX_H3_RES];  ///< index digits (offset by 1 (0..res-1) due to
                            /// baseCell res 0)
} H3IndexFat;

// find the H3 index h3 of the resolution res cell containing the lat/lon g
void geoToH3Fat(const GeoCoord* g, int res, H3IndexFat* h3);

// find the lat/lon center point g of the cell h3
void h3FatToGeo(const H3IndexFat* h3, GeoCoord* g);

// give the cell boundary in lat/lon coordinates for the cell h3
void h3FatToGeoBoundary(const H3IndexFat* h3, GeoBoundary* gp);

void initH3IndexFat(H3IndexFat* c, int res);
void setH3IndexFat(H3IndexFat* c, int res, int baseCell, int digit);
void copyH3IndexFat(const H3IndexFat* orig, H3IndexFat* copy);
int h3FatIsValid(const H3IndexFat* c);
int isResClassIII(int res);
int isPentagon(const H3IndexFat* c);
int h3FatEquals(const H3IndexFat* c1, const H3IndexFat* c2);

// Internal functions

void _faceIjkToH3Fat(const FaceIJK* ijk, int res, H3IndexFat* c);
int _h3FatToFaceIjkWithInitializedFijk(const H3IndexFat* c, FaceIJK* fijk);
int _leadingNonZeroDigit(const H3IndexFat* c);
void _h3FatToFaceIjk(const H3IndexFat* cOrig, FaceIJK* fijk);
void _h3FatRotatePent60ccw(H3IndexFat* c);
void _h3FatRotate60ccw(H3IndexFat* c);
void _h3FatRotate60cw(H3IndexFat* c);

#endif
