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
/** @file h3IndexFat.c
 * @brief   H3IndexFat functions including conversion from lat/lon.
 */

#include "h3IndexFat.h"
#include "baseCells.h"

/** @brief An H3IndexFat with all digits set to 7. */
static const H3IndexFat emptyH3IndexFat = {
    -1, -1, -1, {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}};

/**
 * Encodes a coordinate on the sphere to the H3Fat index of the containing cell
 * at the specified resolution.
 *
 * @param g The spherical coordinates to encode.
 * @param res The desired H3 resolution for the encoding.
 * @param c The encoded H3Fat index.
 */
void geoToH3Fat(const GeoCoord* g, int res, H3IndexFat* c) {
    FaceIJK h;
    _geoToFaceIjk(g, res, &h);
    _faceIjkToH3Fat(&h, res, c);
}

/**
 * Determines the spherical coordinates of the center point of an H3Fat index.
 *
 * @param c The H3Fat index.
 * @param g The spherical coordinates of the H3 cell center.
 */
void h3FatToGeo(const H3IndexFat* c, GeoCoord* g) {
    FaceIJK fijk;
    _h3FatToFaceIjk(c, &fijk);
    _faceIjkToGeo(&fijk, c->res, g);
}

/**
 * Determines the cell boundary in spherical coordinates for an H3Fat index.
 *
 * @param c The H3Fat index.
 * @param gp The boundary of the H3 cell in spherical coordinates.
 */
void h3FatToGeoBoundary(const H3IndexFat* c, GeoBoundary* gp) {
    FaceIJK fijk;
    _h3FatToFaceIjk(c, &fijk);
    _faceIjkToGeoBoundary(&fijk, c->res, isPentagon(c), gp);
}

/**
 * Initialize an H3Fat index to default values.
 *
 * @param c The H3Fat index to initialize.
 * @param res The desired H3 resolution.
 */
void initH3IndexFat(H3IndexFat* c, int res) {
    *c = emptyH3IndexFat;
    c->res = res;
    c->mode = H3_HEXAGON_MODE;
}

/**
 * Initialize an H3Fat index to specified values.
 *
 * @param c The H3Fat index to initialize.
 * @param res The desired H3 resolution.
 * @param baseCell The desired H3 base cell.
 * @param digit The desired H3 digit to initialize all resolutions to.
 */
void setH3IndexFat(H3IndexFat* c, int res, int baseCell, int digit) {
    initH3IndexFat(c, res);
    c->baseCell = baseCell;
    for (int r = 0; r < c->res; r++) c->index[r] = digit;
}

/**
 * Returns whether or not an H3Fat index is valid.
 * @param c The H3Fat index to validate.
 * @return 1 if the H3Fat index if valid, and 0 if it is not.
 */
int h3FatIsValid(const H3IndexFat* c) {
    if (c->mode != H3_HEXAGON_MODE) return 0;

    if (c->baseCell < 0 || c->baseCell >= NUM_BASE_CELLS) return 0;

    if (c->res < 0 || c->res > MAX_H3_RES) return 0;

    for (int r = 0; r < c->res; r++)
        if (c->index[r] < 0 || c->index[r] > 6) return 0;

    for (int r = c->res; r < MAX_H3_RES; r++)
        if (c->index[r] != 7) return 0;

    return 1;
}

/**
 * Make a copy of an H3Fat index.
 * @param orig The H3Fat source index.
 * @param copy The H3Fat index copy.
 */
void copyH3IndexFat(const H3IndexFat* orig, H3IndexFat* copy) {
    initH3IndexFat(copy, orig->res);
    copy->baseCell = orig->baseCell;
    for (int r = 0; r < orig->res; r++) copy->index[r] = orig->index[r];
}

/**
 * Convert an FaceIJK address to the corresponding H3Fat index.
 * @param fijk The FaceIJK address.
 * @param res The cell resolution.
 * @param c The corresponding H3Fat index.
 */
void _faceIjkToH3Fat(const FaceIJK* fijk, int res, H3IndexFat* c) {
    // initialize the index
    initH3IndexFat(c, res);

    // check for res 0/base cell
    if (res == 0) {
        c->baseCell = _faceIjkToBaseCell(fijk);
        c->res = 0;
        return;
    }

    // we need to find the correct base cell FaceIJK for this H3 index;
    // start with the passed in face and resolution res ijk coordinates
    // in that face's coordinate system
    FaceIJK fijkBC = *fijk;

    // build the H3Fat index from finest res up
    // adjust r for the fact that the res 0 base cell offsets the index array
    CoordIJK* ijk = &fijkBC.coord;
    for (int r = res - 1; r >= 0; r--) {
        CoordIJK lastIJK = *ijk;
        CoordIJK lastCenter;
        if (isResClassIII(r + 1))  // Class III == rotate ccw
        {
            _upAp7(ijk);
            lastCenter = *ijk;
            _downAp7(&lastCenter);
        } else  // Class II == rotate cw
        {
            _upAp7r(ijk);
            lastCenter = *ijk;
            _downAp7r(&lastCenter);
        }

        CoordIJK diff;
        _ijkSub(&lastIJK, &lastCenter, &diff);
        _ijkNormalize(&diff);

        c->index[r] = _unitIjkToDigit(&diff);
    }

    // fijkBC should now hold the IJK of the base cell in the
    // coordinate system of the current face

    // lookup the correct base cell
    c->baseCell = _faceIjkToBaseCell(&fijkBC);

    // rotate if necessary to get canonical base cell orientation
    // for this base cell
    int numRots = _faceIjkToBaseCellCCWrot60(&fijkBC);
    if (_isBaseCellPentagon(c->baseCell)) {
        // force rotation out of missing k-axes sub-sequence
        if (_leadingNonZeroDigit(c) == K_AXES_DIGIT) {
            // check for a cw/ccw offset face; default is ccw
            if (baseCellData[c->baseCell].cwOffsetPent[0] == fijkBC.face ||
                baseCellData[c->baseCell].cwOffsetPent[1] == fijkBC.face)
                _h3FatRotate60cw(c);
            else
                _h3FatRotate60ccw(c);
        }

        for (int i = 0; i < numRots; i++) _h3FatRotatePent60ccw(c);
    } else {
        for (int i = 0; i < numRots; i++) _h3FatRotate60ccw(c);
    }
}

/**
 * Convert an H3Fat index to the FaceIJK address on a specified icosahedral
 * face.
 * @param c The H3Fat index.
 * @param fijk The FaceIJK address, initialized with the desired face
 *        and normalized base cell coordinates.
 * @return Returns 1 if the possibility of overage exists, otherwise 0.
 */
int _h3FatToFaceIjkWithInitializedFijk(const H3IndexFat* c, FaceIJK* fijk) {
    CoordIJK* ijk = &fijk->coord;

    // center base cell hierarchy is entirely on this face
    int possibleOverage = 1;
    if (!_isBaseCellPentagon(c->baseCell) &&
        (c->res == 0 ||
         (fijk->coord.i == 0 && fijk->coord.j == 0 && fijk->coord.k == 0)))
        possibleOverage = 0;

    for (int r = 0; r < c->res; r++) {
        if (isResClassIII(r + 1))  // Class III == rotate ccw
            _downAp7(ijk);
        else  // Class II == rotate cw
            _downAp7r(ijk);

        _neighbor(ijk, c->index[r]);
    }

    return possibleOverage;
}

/**
 * Returns the highest resolution non-zero digit in an H3Fat index.
 * @param c The H3Fat index.
 * @return The highest resolution non-zero digit in the H3Fat index.
 */
int _leadingNonZeroDigit(const H3IndexFat* c) {
    for (int r = 0; r < c->res; r++)
        if (c->index[r]) return c->index[r];

    // if we're here it's all 0's
    return 0;
}

/**
 * Returns whether or not a resolution is a Class III grid. Note that odd
 * resolutions are Class III and even resolutions are Class II.
 * @param res The H3 resolution.
 * @return 1 if the resolution is a Class III grid, and 0 if the resolution is
 *         a Class II grid.
 */
int isResClassIII(int res) { return res % 2; }

/**
 * Convert an H3Fat index to a FaceIJK address.
 * @param cIn The H3Fat index.
 * @param fijk The corresponding FaceIJK address.
 */
void _h3FatToFaceIjk(const H3IndexFat* cIn, FaceIJK* fijk) {
    // make a mutable copy of the incoming index
    H3IndexFat c;
    copyH3IndexFat(cIn, &c);

    // adjust for the pentagonal missing sequence; all of sub-sequence 5 needs
    // to be adjusted (and some of sub-sequence 4 below)
    if (_isBaseCellPentagon(c.baseCell) && _leadingNonZeroDigit(&c) == 5)
        _h3FatRotate60cw(&c);

    // start with the "home" face and ijk+ coordinates for the base cell of c
    *fijk = baseCellData[c.baseCell].homeFijk;
    if (!_h3FatToFaceIjkWithInitializedFijk(&c, fijk))
        return;  // no overage is possible; c lies on this face

    // if we're here we have the potential for an "overage"; i.e., it is
    // possible that c lies on an adjacent face

    CoordIJK origIJK = fijk->coord;

    // if we're in Class III, drop into the next finer Class II grid
    int res = c.res;
    if (isResClassIII(res))  // Class III
    {
        _downAp7r(&fijk->coord);
        res++;
    }

    // adjust for overage if needed
    // a pentagon base cell with a leading 4 digit requires special handling
    int pentLeading4 =
        (_isBaseCellPentagon(c.baseCell) && _leadingNonZeroDigit(&c) == 4);
    if (_adjustOverageClassII(fijk, res, pentLeading4, 0)) {
        // if the base cell is a pentagon we have the potential for secondary
        // overages
        if (_isBaseCellPentagon(c.baseCell)) {
            while (1) {
                if (!_adjustOverageClassII(fijk, res, 0, 0)) break;
            }
        }

        if (res != c.res) _upAp7r(&fijk->coord);
    } else if (res != c.res)
        fijk->coord = origIJK;
}

/**
 * Rotate an H3Fat index 60 degrees counter-clockwise about a pentagonal center.
 * Works in place.
 * @param c The H3Fat index.
 */
void _h3FatRotatePent60ccw(H3IndexFat* c) {
    // rotate in place; skips any leading 1 digits (k-axis)
    const int rotDigit[] = {
        0,  // original digit 0
        5,  // original digit 1
        3,  // original digit 2
        1,  // original digit 3
        6,  // original digit 4
        4,  // original digit 5
        2   // original digit 6
    };

    int foundFirstNonZeroDigit = 0;
    for (int r = 0; r < c->res; r++) {
        // rotate this digit
        c->index[r] = rotDigit[c->index[r]];

        // look for the first non-zero digit so we
        // can adjust for deleted k-axes sequence
        // if neccessary
        if (!foundFirstNonZeroDigit && c->index[r] != 0) {
            foundFirstNonZeroDigit = 1;

            // adjust for deleted k-axes sequence
            if (_leadingNonZeroDigit(c) == K_AXES_DIGIT) _h3FatRotate60ccw(c);
        }
    }
}

/**
 * Rotate an H3Fat index 60 degrees counter-clockwise. Works in place.
 * @param c The H3Fat index.
 */
void _h3FatRotate60ccw(H3IndexFat* c) {
    const int rotDigit[] = {
        0,  // original digit 0
        5,  // original digit 1
        3,  // original digit 2
        1,  // original digit 3
        6,  // original digit 4
        4,  // original digit 5
        2   // original digit 6
    };

    for (int r = 0; r < c->res; r++) c->index[r] = rotDigit[c->index[r]];
}

/**
 * Determine whether an H3Fat index is a pentagon.
 * @param c The H3Fat index.
 * @return 1 if the H3Fat index is a pentagon, and 0 otherwise.
 */
int isPentagon(const H3IndexFat* c) {
    // a pentagon has a pentagon base cell and all zero digits
    return _isBaseCellPentagon(c->baseCell) && !_leadingNonZeroDigit(c);
}

/**
 * Determine whether two H3Fat indexes are equal.
 * @param c1 The first H3Fat index.
 * @param c2 The second H3Fat index.
 * @return 1 if the two indexes are equal, and 0 otherwise.
 */
int h3FatEquals(const H3IndexFat* c1, const H3IndexFat* c2) {
    if (c1->res != c2->res || c1->baseCell != c2->baseCell) return 0;

    for (int r = 0; r < c1->res; r++)
        if (c1->index[r] != c2->index[r]) return 0;

    return 1;
}

/**
 * Rotate an H3Fat index 60 degrees clockwise. Works in place.
 * @param c The H3Fat index.
 */
void _h3FatRotate60cw(H3IndexFat* c) {
    const int rotDigit[] = {
        0,  // original digit 0
        3,  // original digit 1
        6,  // original digit 2
        2,  // original digit 3
        5,  // original digit 4
        1,  // original digit 5
        4   // original digit 6
    };

    for (int r = 0; r < c->res; r++) c->index[r] = rotDigit[c->index[r]];
}
