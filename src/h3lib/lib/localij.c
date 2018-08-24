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
/** @file localij.c
 * @brief   Local IJ coordinate space functions
 *
 * These functions try to provide a useful coordinate space in the vicinity of
 * an origin index.
 */
#include <faceijk.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "baseCells.h"
#include "faceijk.h"
#include "h3Index.h"
#include "mathExtensions.h"
#include "stackAlloc.h"

/**
 * Produces ijk+ coordinates for an index anchored by an origin.
 *
 * The coordinate space used by this function may have deleted
 * regions or warping due to pentagonal distortion.
 *
 * Coordinates are only comparable if they come from the same
 * origin index.
 *
 * Failure may occur if the index is too far away from the origin
 * or if the index is on the other side of a pentagon.
 *
 * @param origin An anchoring index for the ijk+ coordinate system.
 * @param index Index to find the coordinates of
 * @param out ijk+ coordinates of the index will be placed here on success
 * @return 0 on success, or another value on failure.
 */
int h3ToLocalIjk(H3Index origin, H3Index h3, CoordIJK *out) {
    if (H3_GET_MODE(origin) != H3_HEXAGON_MODE ||
        H3_GET_MODE(h3) != H3_HEXAGON_MODE) {
        // Only hexagon mode is relevant, since we can't
        // encode directionality in CoordIJK.
        return 1;
    }

    int res = H3_GET_RESOLUTION(origin);

    if (res != H3_GET_RESOLUTION(h3)) {
        return 1;
    }

    int originBaseCell = H3_GET_BASE_CELL(origin);
    int baseCell = H3_GET_BASE_CELL(h3);

    // Direction from origin base cell to index base cell
    Direction dir = 0;
    Direction revDir = 0;
    if (originBaseCell != baseCell) {
        dir = _getBaseCellDirection(originBaseCell, baseCell);
        if (dir == INVALID_DIGIT) {
            // Base cells are not neighbors, can't unfold.
            return 2;
        }
        revDir = _getBaseCellDirection(baseCell, originBaseCell);
        assert(revDir != INVALID_DIGIT);
    }

    int originOnPent = _isBaseCellPentagon(originBaseCell);
    int indexOnPent = _isBaseCellPentagon(baseCell);

    FaceIJK indexFijk = {0};
    if (dir != CENTER_DIGIT) {
        // Rotate index into the orientation of the origin base cell.
        // cw because we are undoing the rotation into that base cell.
        int baseCellRotations = baseCellNeighbor60CCWRots[originBaseCell][dir];
        if (indexOnPent) {
            for (int i = 0; i < baseCellRotations; i++) {
                h3 = _h3RotatePent60cw(h3);

                revDir = _rotate60cw(revDir);
                if (revDir == K_AXES_DIGIT) revDir = _rotate60cw(revDir);
            }
        } else {
            for (int i = 0; i < baseCellRotations; i++) {
                h3 = _h3Rotate60cw(h3);

                revDir = _rotate60cw(revDir);
            }
        }
    }
    // Face is unused. This produces coordinates in base cell coordinate space.
    _h3ToFaceIjkWithInitializedFijk(h3, &indexFijk);

    // Origin leading digit -> index leading digit -> rotations 60 cw
    // Either being 1 (K axis) is invalid.
    // No good default at 0.
    const int PENTAGON_ROTATIONS[7][7] = {
        {0, -1, 0, 0, 0, 0, 0},        // 0
        {-1, -1, -1, -1, -1, -1, -1},  // 1
        {0, -1, 0, 0, 0, 1, 0},        // 2
        {0, -1, 0, 0, 1, 1, 0},        // 3
        {0, -1, 0, 5, 0, 0, 0},        // 4
        {0, -1, 5, 5, 0, 0, 0},        // 5
        {0, -1, 0, 0, 0, 0, 0},        // 6
    };
    // Simply prohibit many pentagon distortion cases rather than handling them.
    const bool FAILED_DIRECTIONS_II[7][7] = {
        {false, false, false, false, false, false, false},  // 0
        {false, false, false, false, false, false, false},  // 1
        {false, false, false, false, true, false, false},   // 2
        {false, false, false, false, false, false, true},   // 3
        {false, false, false, true, false, false, false},   // 4
        {false, false, true, false, false, false, false},   // 5
        {false, false, false, false, false, true, false},   // 6
    };
    const bool FAILED_DIRECTIONS_III[7][7] = {
        {false, false, false, false, false, false, false},  // 0
        {false, false, false, false, false, false, false},  // 1
        {false, false, false, false, false, true, false},   // 2
        {false, false, false, false, true, false, false},   // 3
        {false, false, true, false, false, false, false},   // 4
        {false, false, false, false, false, false, true},   // 5
        {false, false, false, true, false, false, false},   // 6
    };

    if (dir != CENTER_DIGIT) {
        assert(baseCell != originBaseCell);
        assert(!(originOnPent && indexOnPent));

        int pentagonRotations = 0;
        int directionRotations = 0;

        if (originOnPent) {
            int originLeadingDigit = _h3LeadingNonZeroDigit(origin);

            if ((isResClassIII(res) &&
                 FAILED_DIRECTIONS_III[originLeadingDigit][dir]) ||
                (!isResClassIII(res) &&
                 FAILED_DIRECTIONS_II[originLeadingDigit][dir])) {
                // TODO this part of the pentagon might not be unfolded
                // correctly.
                return 3;
            }

            directionRotations = PENTAGON_ROTATIONS[originLeadingDigit][dir];
            pentagonRotations = directionRotations;
        } else if (indexOnPent) {
            int indexLeadingDigit = _h3LeadingNonZeroDigit(h3);

            if ((isResClassIII(res) &&
                 FAILED_DIRECTIONS_III[indexLeadingDigit][revDir]) ||
                (!isResClassIII(res) &&
                 FAILED_DIRECTIONS_II[indexLeadingDigit][revDir])) {
                // TODO this part of the pentagon might not be unfolded
                // correctly.
                return 4;
            }

            pentagonRotations = PENTAGON_ROTATIONS[revDir][indexLeadingDigit];
        }

        assert(pentagonRotations >= 0);
        assert(directionRotations >= 0);

        for (int i = 0; i < pentagonRotations; i++) {
            _ijkRotate60cw(&indexFijk.coord);
        }

        CoordIJK offset = {0};
        _neighbor(&offset, dir);
        // Scale offset based on resolution
        for (int r = res - 1; r >= 0; r--) {
            if (isResClassIII(r + 1)) {
                // rotate ccw
                _downAp7(&offset);
            } else {
                // rotate cw
                _downAp7r(&offset);
            }
        }

        for (int i = 0; i < directionRotations; i++) {
            _ijkRotate60cw(&offset);
        }

        // Perform necessary translation
        _ijkAdd(&indexFijk.coord, &offset, &indexFijk.coord);
        _ijkNormalize(&indexFijk.coord);
    } else if (originOnPent && indexOnPent) {
        // If the origin and index are on pentagon, and we checked that the base
        // cells are the same or neighboring, then they must be the same base
        // cell.
        assert(baseCell == originBaseCell);

        int originLeadingDigit = _h3LeadingNonZeroDigit(origin);
        int indexLeadingDigit = _h3LeadingNonZeroDigit(h3);

        if (FAILED_DIRECTIONS_III[originLeadingDigit][indexLeadingDigit] ||
            FAILED_DIRECTIONS_II[originLeadingDigit][indexLeadingDigit]) {
            // TODO this part of the pentagon might not be unfolded
            // correctly.
            return 5;
        }

        int withinPentagonRotations =
            PENTAGON_ROTATIONS[originLeadingDigit][indexLeadingDigit];

        for (int i = 0; i < withinPentagonRotations; i++) {
            _ijkRotate60cw(&indexFijk.coord);
        }
    }

    *out = indexFijk.coord;
    return 0;
}

/**
 * Produces ij coordinates for an index anchored by an origin.
 *
 * The coordinate space used by this function may have deleted
 * regions or warping due to pentagonal distortion.
 *
 * Coordinates are only comparable if they come from the same
 * origin index.
 *
 * Failure may occur if the index is too far away from the origin
 * or if the index is on the other side of a pentagon.
 *
 * This function is experimental, and its output is not guaranteed
 * to be compatible across different versions of H3.
 *
 * @param origin An anchoring index for the ij coordinate system.
 * @param index Index to find the coordinates of
 * @param out ij coordinates of the index will be placed here on success
 * @return 0 on success, or another value on failure.
 */
int H3_EXPORT(experimentalH3ToLocalIj)(H3Index origin, H3Index h3,
                                       CoordIJ *out) {
    // This function is currently experimental. Once ready to be part of the
    // non-experimental API, this function (with the experimental prefix) will
    // be marked as deprecated and to be removed in the next major version. It
    // will be replaced with a non-prefixed function name.
    CoordIJK ijk;
    int failed = h3ToLocalIjk(origin, h3, &ijk);
    if (failed) {
        return failed;
    }

    ijkToIj(&ijk, out);

    return 0;
}

/**
 * Produces the grid distance between the two indexes.
 *
 * This function may fail to find the distance between two indexes, for
 * example if they are very far apart. It may also fail when finding
 * distances for indexes on opposite sides of a pentagon.
 *
 * @param origin Index to find the distance from.
 * @param index Index to find the distance to.
 * @return The distance, or a negative number if the library could not
 * compute the distance.
 */
int H3_EXPORT(h3Distance)(H3Index origin, H3Index h3) {
    CoordIJK originIjk, h3Ijk;
    if (h3ToLocalIjk(origin, origin, &originIjk)) {
        // Only possible if origin is invalid, for example if it's a
        // unidirectional edge.
        return -1;
    }
    if (h3ToLocalIjk(origin, h3, &h3Ijk)) {
        return -1;
    }

    return ijkDistance(&originIjk, &h3Ijk);
}
