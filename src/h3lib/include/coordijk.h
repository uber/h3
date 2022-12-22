/*
 * Copyright 2016-2018, 2020-2022 Uber Technologies, Inc.
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
/** @file coordijk.h
 * @brief   Header file for CoordIJK functions including conversion from lat/lng
 *
 * References two Vec2d cartesian coordinate systems:
 *
 *    1. gnomonic: face-centered polyhedral gnomonic projection space with
 *             traditional scaling and x-axes aligned with the face Class II
 *             i-axes.
 *
 *    2. hex2d: local face-centered coordinate system scaled a specific H3 grid
 *             resolution unit length and with x-axes aligned with the local
 *             i-axes
 */

#ifndef COORDIJK_H
#define COORDIJK_H

#include "h3api.h"
#include "latLng.h"
#include "vec2d.h"

/** @struct CoordIJK
 * @brief IJK hexagon coordinates
 *
 * Each axis is spaced 120 degrees apart.
 */
typedef struct {
    int i;  ///< i component
    int j;  ///< j component
    int k;  ///< k component
} CoordIJK;

/** @brief CoordIJK unit vectors corresponding to the 7 H3 digits.
 */
static const CoordIJK UNIT_VECS[] = {
    {0, 0, 0},  // direction 0
    {0, 0, 1},  // direction 1
    {0, 1, 0},  // direction 2
    {0, 1, 1},  // direction 3
    {1, 0, 0},  // direction 4
    {1, 0, 1},  // direction 5
    {1, 1, 0}   // direction 6
};

/** @brief H3 digit representing ijk+ axes direction.
 * Values will be within the lowest 3 bits of an integer.
 */
typedef enum {
    /** H3 digit in center */
    CENTER_DIGIT = 0,
    /** H3 digit in k-axes direction */
    K_AXES_DIGIT = 1,
    /** H3 digit in j-axes direction */
    J_AXES_DIGIT = 2,
    /** H3 digit in j == k direction */
    JK_AXES_DIGIT = J_AXES_DIGIT | K_AXES_DIGIT, /* 3 */
    /** H3 digit in i-axes direction */
    I_AXES_DIGIT = 4,
    /** H3 digit in i == k direction */
    IK_AXES_DIGIT = I_AXES_DIGIT | K_AXES_DIGIT, /* 5 */
    /** H3 digit in i == j direction */
    IJ_AXES_DIGIT = I_AXES_DIGIT | J_AXES_DIGIT, /* 6 */
    /** H3 digit in the invalid direction */
    INVALID_DIGIT = 7,
    /** Valid digits will be less than this value. Same value as INVALID_DIGIT.
     */
    NUM_DIGITS = INVALID_DIGIT,
    /** Child digit which is skipped for pentagons */
    PENTAGON_SKIPPED_DIGIT = K_AXES_DIGIT /* 1 */
} Direction;

// Internal functions

void _setIJK(CoordIJK *ijk, int i, int j, int k);
void _hex2dToCoordIJK(const Vec2d *v, CoordIJK *h);
void _ijkToHex2d(const CoordIJK *h, Vec2d *v);
int _ijkMatches(const CoordIJK *c1, const CoordIJK *c2);
void _ijkAdd(const CoordIJK *h1, const CoordIJK *h2, CoordIJK *sum);
void _ijkSub(const CoordIJK *h1, const CoordIJK *h2, CoordIJK *diff);
void _ijkScale(CoordIJK *c, int factor);
bool _ijkNormalizeCouldOverflow(const CoordIJK *ijk);
void _ijkNormalize(CoordIJK *c);
Direction _unitIjkToDigit(const CoordIJK *ijk);
H3Error _upAp7Checked(CoordIJK *ijk);
H3Error _upAp7rChecked(CoordIJK *ijk);
void _upAp7(CoordIJK *ijk);
void _upAp7r(CoordIJK *ijk);
void _downAp7(CoordIJK *ijk);
void _downAp7r(CoordIJK *ijk);
void _downAp3(CoordIJK *ijk);
void _downAp3r(CoordIJK *ijk);
void _neighbor(CoordIJK *ijk, Direction digit);
void _ijkRotate60ccw(CoordIJK *ijk);
void _ijkRotate60cw(CoordIJK *ijk);
Direction _rotate60ccw(Direction digit);
Direction _rotate60cw(Direction digit);
int ijkDistance(const CoordIJK *a, const CoordIJK *b);
void ijkToIj(const CoordIJK *ijk, CoordIJ *ij);
H3Error ijToIjk(const CoordIJ *ij, CoordIJK *ijk);
void ijkToCube(CoordIJK *ijk);
void cubeToIjk(CoordIJK *ijk);

#endif
