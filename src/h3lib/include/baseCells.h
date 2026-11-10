/*
 * Copyright 2016-2018 Uber Technologies, Inc.
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
/** @file baseCells.h
 * @brief   Base cell related lookup tables and access functions.
 */

#ifndef BASECELLS_H
#define BASECELLS_H

#include "constants.h"
#include "coordijk.h"
#include "faceijk.h"

/** @struct BaseCellData
 * @brief information on a single base cell
 */
typedef struct {
    FaceIJK
        homeFijk;  ///< "home" face and normalized ijk coordinates on that face
    int isPentagon;       ///< is this base cell a pentagon?
    int cwOffsetPent[2];  ///< if a pentagon, what are its two clockwise offset
                          /// faces?
} BaseCellData;

#define INVALID_BASE_CELL 127

/** Number of distinct values the 7 bit base cell field of an H3Index can hold.
 * Only the first NUM_BASE_CELLS of them name a real base cell. */
#define NUM_BASE_CELL_VALUES 128

extern const int baseCellNeighbors[NUM_BASE_CELLS][7];
extern const int baseCellNeighbor60CCWRots[NUM_BASE_CELLS][7];

// resolution 0 base cell data lookup-table (global)
extern const BaseCellData baseCellData[NUM_BASE_CELLS];

/** Pentagon flag for each base cell, split out of baseCellData so hot paths
 * touch one contiguous byte array instead of striding through the much larger
 * BaseCellData structs.
 *
 * Sized to the full 7 bit range rather than NUM_BASE_CELLS so that a base cell
 * number taken straight out of an H3Index, which has not necessarily been
 * validated, cannot index out of bounds. Entries at and above NUM_BASE_CELLS
 * are false. */
extern const bool baseCellIsPentagon[NUM_BASE_CELL_VALUES];

/** Maximum input for any component to face-to-base-cell lookup functions */
#define MAX_FACE_COORD 2

/** Invalid number of rotations */
#define INVALID_ROTATIONS -1

// Internal functions
int _isBaseCellPentagon(int baseCell);
bool _isBaseCellPolarPentagon(int baseCell);
int _faceIjkToBaseCell(const FaceIJK *h);
int _faceIjkToBaseCellCCWrot60(const FaceIJK *h);
int _baseCellToCCWrot60(int baseCell, int face);
void _baseCellToFaceIjk(int baseCell, FaceIJK *h);
bool _baseCellIsCwOffset(int baseCell, int testFace);
int _getBaseCellNeighbor(int baseCell, Direction dir);
Direction _getBaseCellDirection(int originBaseCell, int destinationBaseCell);

#endif
