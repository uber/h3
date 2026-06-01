/*
 * Copyright 2016-2018, 2020-2022, 2026 Uber Technologies, Inc.
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
 * @brief   Header-only implementation of CoordIJK functions including
 *          conversion from lat/lng.
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

#include <math.h>
#include <stdlib.h>

#include "constants.h"
#include "h3Assert.h"
#include "h3api.h"
#include "mathExtensions.h"
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

#define INT32_MAX_3 (INT32_MAX / 3)

/**
 * Sets an IJK coordinate to the specified component values.
 *
 * @param ijk The IJK coordinate to set.
 * @param i The desired i component value.
 * @param j The desired j component value.
 * @param k The desired k component value.
 */
static inline void _setIJK(CoordIJK *ijk, int i, int j, int k) {
    ijk->i = i;
    ijk->j = j;
    ijk->k = k;
}

/**
 * Returns whether or not two ijk coordinates contain exactly the same
 * component values.
 *
 * @param c1 The first set of ijk coordinates.
 * @param c2 The second set of ijk coordinates.
 * @return 1 if the two addresses match, 0 if they do not.
 */
static inline int _ijkMatches(const CoordIJK *c1, const CoordIJK *c2) {
    return (c1->i == c2->i && c1->j == c2->j && c1->k == c2->k);
}

/**
 * Add two ijk coordinates.
 *
 * @param h1 The first set of ijk coordinates.
 * @param h2 The second set of ijk coordinates.
 * @param sum The sum of the two sets of ijk coordinates.
 */
static inline void _ijkAdd(const CoordIJK *h1, const CoordIJK *h2,
                           CoordIJK *sum) {
    sum->i = h1->i + h2->i;
    sum->j = h1->j + h2->j;
    sum->k = h1->k + h2->k;
}

/**
 * Subtract two ijk coordinates.
 *
 * @param h1 The first set of ijk coordinates.
 * @param h2 The second set of ijk coordinates.
 * @param diff The difference of the two sets of ijk coordinates (h1 - h2).
 */
static inline void _ijkSub(const CoordIJK *h1, const CoordIJK *h2,
                           CoordIJK *diff) {
    diff->i = h1->i - h2->i;
    diff->j = h1->j - h2->j;
    diff->k = h1->k - h2->k;
}

/**
 * Uniformly scale ijk coordinates by a scalar. Works in place.
 *
 * @param c The ijk coordinates to scale.
 * @param factor The scaling factor.
 */
static inline void _ijkScale(CoordIJK *c, int factor) {
    c->i *= factor;
    c->j *= factor;
    c->k *= factor;
}

/**
 * Returns true if _ijkNormalize with the given input could have a signed
 * integer overflow. Assumes k is set to 0.
 */
static inline bool _ijkNormalizeCouldOverflow(const CoordIJK *ijk) {
    // Check for the possibility of overflow
    int max, min;
    if (ijk->i > ijk->j) {
        max = ijk->i;
        min = ijk->j;
    } else {
        max = ijk->j;
        min = ijk->i;
    }
    if (min < 0) {
        // Only if the min is less than 0 will the resulting number be larger
        // than max. If min is positive, then max is also positive, and a
        // positive signed integer minus another positive signed integer will
        // not overflow.
        if (ADD_INT32S_OVERFLOWS(max, min)) {
            // max + min would overflow
            return true;
        }
        if (SUB_INT32S_OVERFLOWS(0, min)) {
            // 0 - INT32_MIN would overflow
            return true;
        }
        if (SUB_INT32S_OVERFLOWS(max, min)) {
            // max - min would overflow
            return true;
        }
    }
    return false;
}

/**
 * Normalizes ijk coordinates by setting the components to the smallest possible
 * values. Works in place.
 *
 * This function does not protect against signed integer overflow. The caller
 * must ensure that none of (i - j), (i - k), (j - i), (j - k), (k - i), (k -
 * j) will overflow. This function may be changed in the future to make that
 * check itself and return an error code.
 *
 * @param c The ijk coordinates to normalize.
 */
static inline void _ijkNormalize(CoordIJK *c) {
    // remove any negative values
    if (c->i < 0) {
        c->j -= c->i;
        c->k -= c->i;
        c->i = 0;
    }

    if (c->j < 0) {
        c->i -= c->j;
        c->k -= c->j;
        c->j = 0;
    }

    if (c->k < 0) {
        c->i -= c->k;
        c->j -= c->k;
        c->k = 0;
    }

    // remove the min value if needed
    int min = c->i;
    if (c->j < min) min = c->j;
    if (c->k < min) min = c->k;
    if (min > 0) {
        c->i -= min;
        c->j -= min;
        c->k -= min;
    }
}

/**
 * Find the center point in 2D cartesian coordinates of a hex.
 *
 * @param h The ijk coordinates of the hex.
 * @param v The 2D cartesian coordinates of the hex center point.
 */
static inline void _ijkToHex2d(const CoordIJK *h, Vec2d *v) {
    int i = h->i - h->k;
    int j = h->j - h->k;

    v->x = i - 0.5 * j;
    v->y = j * M_SQRT3_2;
}

/**
 * Determine the containing hex in ijk+ coordinates for a 2D cartesian
 * coordinate vector (from DGGRID).
 *
 * @param v The 2D cartesian coordinate vector.
 * @param h The ijk+ coordinates of the containing hex.
 */
static inline void _hex2dToCoordIJK(const Vec2d *v, CoordIJK *h) {
    double a1, a2;
    double x1, x2;
    int m1, m2;
    double r1, r2;

    // quantize into the ij system and then normalize
    h->k = 0;

    a1 = fabsl(v->x);
    a2 = fabsl(v->y);

    // first do a reverse conversion
    x2 = a2 * M_RSIN60;
    x1 = a1 + x2 / 2.0;

    // check if we have the center of a hex
    m1 = (int)x1;
    m2 = (int)x2;

    // otherwise round correctly
    r1 = x1 - m1;
    r2 = x2 - m2;

    if (r1 < 0.5) {
        if (r1 < 1.0 / 3.0) {
            if (r2 < (1.0 + r1) / 2.0) {
                h->i = m1;
                h->j = m2;
            } else {
                h->i = m1;
                h->j = m2 + 1;
            }
        } else {
            if (r2 < (1.0 - r1)) {
                h->j = m2;
            } else {
                h->j = m2 + 1;
            }

            if ((1.0 - r1) <= r2 && r2 < (2.0 * r1)) {
                h->i = m1 + 1;
            } else {
                h->i = m1;
            }
        }
    } else {
        if (r1 < 2.0 / 3.0) {
            if (r2 < (1.0 - r1)) {
                h->j = m2;
            } else {
                h->j = m2 + 1;
            }

            if ((2.0 * r1 - 1.0) < r2 && r2 < (1.0 - r1)) {
                h->i = m1;
            } else {
                h->i = m1 + 1;
            }
        } else {
            if (r2 < (r1 / 2.0)) {
                h->i = m1 + 1;
                h->j = m2;
            } else {
                h->i = m1 + 1;
                h->j = m2 + 1;
            }
        }
    }

    // now fold across the axes if necessary

    if (v->x < 0.0) {
        if ((h->j % 2) == 0)  // even
        {
            long long int axisi = h->j / 2;
            long long int diff = h->i - axisi;
            h->i = (int)(h->i - 2.0 * diff);
        } else {
            long long int axisi = (h->j + 1) / 2;
            long long int diff = h->i - axisi;
            h->i = (int)(h->i - (2.0 * diff + 1));
        }
    }

    if (v->y < 0.0) {
        h->i = h->i - (2 * h->j + 1) / 2;
        h->j = -1 * h->j;
    }

    _ijkNormalize(h);
}

/**
 * Determines the H3 digit corresponding to a unit vector or the zero vector
 * in ijk coordinates.
 *
 * @param ijk The ijk coordinates; must be a unit vector or zero vector.
 * @return The H3 digit (0-6) corresponding to the ijk unit vector, zero
 * vector, or INVALID_DIGIT (7) on failure.
 */
static inline Direction _unitIjkToDigit(const CoordIJK *ijk) {
    CoordIJK c = *ijk;
    _ijkNormalize(&c);

    Direction digit = INVALID_DIGIT;
    for (Direction i = CENTER_DIGIT; i < NUM_DIGITS; i++) {
        if (_ijkMatches(&c, &UNIT_VECS[i])) {
            digit = i;
            break;
        }
    }

    return digit;
}

/**
 * Returns non-zero if _upAp7 with the given input could have a signed integer
 * overflow.
 *
 * Assumes ijk is IJK+ coordinates (no negative numbers).
 */
static inline H3Error _upAp7Checked(CoordIJK *ijk) {
    // Doesn't need to be checked because i, j, and k must all be non-negative
    int i = ijk->i - ijk->k;
    int j = ijk->j - ijk->k;

    // <0 is checked because the input must all be non-negative, but some
    // negative inputs are used in unit tests to exercise the below.
    if (i >= INT32_MAX_3 || j >= INT32_MAX_3 || i < 0 || j < 0) {
        if (ADD_INT32S_OVERFLOWS(i, i)) {
            return E_FAILED;
        }
        int i2 = i + i;
        if (ADD_INT32S_OVERFLOWS(i2, i)) {
            return E_FAILED;
        }
        int i3 = i2 + i;
        if (ADD_INT32S_OVERFLOWS(j, j)) {
            return E_FAILED;
        }
        int j2 = j + j;

        if (SUB_INT32S_OVERFLOWS(i3, j)) {
            return E_FAILED;
        }
        if (ADD_INT32S_OVERFLOWS(i, j2)) {
            return E_FAILED;
        }
    }

    ijk->i = (int)lround(((i * 3) - j) * M_ONESEVENTH);
    ijk->j = (int)lround((i + (j * 2)) * M_ONESEVENTH);
    ijk->k = 0;

    // Expected not to be reachable, because max + min or max - min would need
    // to overflow.
    if (NEVER(_ijkNormalizeCouldOverflow(ijk))) {
        return E_FAILED;
    }
    _ijkNormalize(ijk);
    return E_SUCCESS;
}

/**
 * Returns non-zero if _upAp7r with the given input could have a signed integer
 * overflow.
 *
 * Assumes ijk is IJK+ coordinates (no negative numbers).
 */
static inline H3Error _upAp7rChecked(CoordIJK *ijk) {
    // Doesn't need to be checked because i, j, and k must all be non-negative
    int i = ijk->i - ijk->k;
    int j = ijk->j - ijk->k;

    // <0 is checked because the input must all be non-negative, but some
    // negative inputs are used in unit tests to exercise the below.
    if (i >= INT32_MAX_3 || j >= INT32_MAX_3 || i < 0 || j < 0) {
        if (ADD_INT32S_OVERFLOWS(i, i)) {
            return E_FAILED;
        }
        int i2 = i + i;
        if (ADD_INT32S_OVERFLOWS(j, j)) {
            return E_FAILED;
        }
        int j2 = j + j;
        if (ADD_INT32S_OVERFLOWS(j2, j)) {
            return E_FAILED;
        }
        int j3 = j2 + j;

        if (ADD_INT32S_OVERFLOWS(i2, j)) {
            return E_FAILED;
        }
        if (SUB_INT32S_OVERFLOWS(j3, i)) {
            return E_FAILED;
        }
    }

    ijk->i = (int)lround(((i * 2) + j) * M_ONESEVENTH);
    ijk->j = (int)lround(((j * 3) - i) * M_ONESEVENTH);
    ijk->k = 0;

    // Expected not to be reachable, because max + min or max - min would need
    // to overflow.
    if (NEVER(_ijkNormalizeCouldOverflow(ijk))) {
        return E_FAILED;
    }
    _ijkNormalize(ijk);
    return E_SUCCESS;
}

/**
 * Find the normalized ijk coordinates of the indexing parent of a cell in a
 * counter-clockwise aperture 7 grid. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _upAp7(CoordIJK *ijk) {
    // convert to CoordIJ
    int i = ijk->i - ijk->k;
    int j = ijk->j - ijk->k;

    ijk->i = (int)lround((3 * i - j) * M_ONESEVENTH);
    ijk->j = (int)lround((i + 2 * j) * M_ONESEVENTH);
    ijk->k = 0;
    _ijkNormalize(ijk);
}

/**
 * Find the normalized ijk coordinates of the indexing parent of a cell in a
 * clockwise aperture 7 grid. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _upAp7r(CoordIJK *ijk) {
    // convert to CoordIJ
    int i = ijk->i - ijk->k;
    int j = ijk->j - ijk->k;

    ijk->i = (int)lround((2 * i + j) * M_ONESEVENTH);
    ijk->j = (int)lround((3 * j - i) * M_ONESEVENTH);
    ijk->k = 0;
    _ijkNormalize(ijk);
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 7 counter-clockwise resolution. Works in
 * place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _downAp7(CoordIJK *ijk) {
    // res r unit vectors in res r+1
    CoordIJK iVec = {3, 0, 1};
    CoordIJK jVec = {1, 3, 0};
    CoordIJK kVec = {0, 1, 3};

    _ijkScale(&iVec, ijk->i);
    _ijkScale(&jVec, ijk->j);
    _ijkScale(&kVec, ijk->k);

    _ijkAdd(&iVec, &jVec, ijk);
    _ijkAdd(ijk, &kVec, ijk);

    _ijkNormalize(ijk);
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 7 clockwise resolution. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _downAp7r(CoordIJK *ijk) {
    // res r unit vectors in res r+1
    CoordIJK iVec = {3, 1, 0};
    CoordIJK jVec = {0, 3, 1};
    CoordIJK kVec = {1, 0, 3};

    _ijkScale(&iVec, ijk->i);
    _ijkScale(&jVec, ijk->j);
    _ijkScale(&kVec, ijk->k);

    _ijkAdd(&iVec, &jVec, ijk);
    _ijkAdd(ijk, &kVec, ijk);

    _ijkNormalize(ijk);
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 3 counter-clockwise resolution. Works in
 * place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _downAp3(CoordIJK *ijk) {
    // res r unit vectors in res r+1
    CoordIJK iVec = {2, 0, 1};
    CoordIJK jVec = {1, 2, 0};
    CoordIJK kVec = {0, 1, 2};

    _ijkScale(&iVec, ijk->i);
    _ijkScale(&jVec, ijk->j);
    _ijkScale(&kVec, ijk->k);

    _ijkAdd(&iVec, &jVec, ijk);
    _ijkAdd(ijk, &kVec, ijk);

    _ijkNormalize(ijk);
}

/**
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 3 clockwise resolution. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _downAp3r(CoordIJK *ijk) {
    // res r unit vectors in res r+1
    CoordIJK iVec = {2, 1, 0};
    CoordIJK jVec = {0, 2, 1};
    CoordIJK kVec = {1, 0, 2};

    _ijkScale(&iVec, ijk->i);
    _ijkScale(&jVec, ijk->j);
    _ijkScale(&kVec, ijk->k);

    _ijkAdd(&iVec, &jVec, ijk);
    _ijkAdd(ijk, &kVec, ijk);

    _ijkNormalize(ijk);
}

/**
 * Find the normalized ijk coordinates of the hex in the specified digit
 * direction from the specified ijk coordinates. Works in place.
 *
 * @param ijk The ijk coordinates.
 * @param digit The digit direction from the original ijk coordinates.
 */
static inline void _neighbor(CoordIJK *ijk, Direction digit) {
    if (digit > CENTER_DIGIT && digit < NUM_DIGITS) {
        _ijkAdd(ijk, &UNIT_VECS[digit], ijk);
        _ijkNormalize(ijk);
    }
}

/**
 * Rotates ijk coordinates 60 degrees counter-clockwise. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _ijkRotate60ccw(CoordIJK *ijk) {
    // unit vector rotations
    CoordIJK iVec = {1, 1, 0};
    CoordIJK jVec = {0, 1, 1};
    CoordIJK kVec = {1, 0, 1};

    _ijkScale(&iVec, ijk->i);
    _ijkScale(&jVec, ijk->j);
    _ijkScale(&kVec, ijk->k);

    _ijkAdd(&iVec, &jVec, ijk);
    _ijkAdd(ijk, &kVec, ijk);

    _ijkNormalize(ijk);
}

/**
 * Rotates ijk coordinates 60 degrees clockwise. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
static inline void _ijkRotate60cw(CoordIJK *ijk) {
    // unit vector rotations
    CoordIJK iVec = {1, 0, 1};
    CoordIJK jVec = {1, 1, 0};
    CoordIJK kVec = {0, 1, 1};

    _ijkScale(&iVec, ijk->i);
    _ijkScale(&jVec, ijk->j);
    _ijkScale(&kVec, ijk->k);

    _ijkAdd(&iVec, &jVec, ijk);
    _ijkAdd(ijk, &kVec, ijk);

    _ijkNormalize(ijk);
}

/**
 * Rotates indexing digit 60 degrees counter-clockwise. Returns result.
 *
 * @param digit Indexing digit (between 1 and 6 inclusive)
 */
static inline Direction _rotate60ccw(Direction digit) {
    switch (digit) {
        case K_AXES_DIGIT:
            return IK_AXES_DIGIT;
        case IK_AXES_DIGIT:
            return I_AXES_DIGIT;
        case I_AXES_DIGIT:
            return IJ_AXES_DIGIT;
        case IJ_AXES_DIGIT:
            return J_AXES_DIGIT;
        case J_AXES_DIGIT:
            return JK_AXES_DIGIT;
        case JK_AXES_DIGIT:
            return K_AXES_DIGIT;
        default:
            return digit;
    }
}

/**
 * Rotates indexing digit 60 degrees clockwise. Returns result.
 *
 * @param digit Indexing digit (between 1 and 6 inclusive)
 */
static inline Direction _rotate60cw(Direction digit) {
    switch (digit) {
        case K_AXES_DIGIT:
            return JK_AXES_DIGIT;
        case JK_AXES_DIGIT:
            return J_AXES_DIGIT;
        case J_AXES_DIGIT:
            return IJ_AXES_DIGIT;
        case IJ_AXES_DIGIT:
            return I_AXES_DIGIT;
        case I_AXES_DIGIT:
            return IK_AXES_DIGIT;
        case IK_AXES_DIGIT:
            return K_AXES_DIGIT;
        default:
            return digit;
    }
}

/**
 * Finds the distance between the two coordinates. Returns result.
 *
 * @param c1 The first set of ijk coordinates.
 * @param c2 The second set of ijk coordinates.
 */
static inline int ijkDistance(const CoordIJK *c1, const CoordIJK *c2) {
    CoordIJK diff;
    _ijkSub(c1, c2, &diff);
    _ijkNormalize(&diff);
    CoordIJK absDiff = {abs(diff.i), abs(diff.j), abs(diff.k)};
    return MAX(absDiff.i, MAX(absDiff.j, absDiff.k));
}

/**
 * Transforms coordinates from the IJK+ coordinate system to the IJ coordinate
 * system.
 *
 * @param ijk The input IJK+ coordinates
 * @param ij The output IJ coordinates
 */
static inline void ijkToIj(const CoordIJK *ijk, CoordIJ *ij) {
    ij->i = ijk->i - ijk->k;
    ij->j = ijk->j - ijk->k;
}

/**
 * Transforms coordinates from the IJ coordinate system to the IJK+ coordinate
 * system.
 *
 * @param ij The input IJ coordinates
 * @param ijk The output IJK+ coordinates
 * @returns E_SUCCESS on success, E_FAILED if signed integer overflow would have
 * occurred.
 */
static inline H3Error ijToIjk(const CoordIJ *ij, CoordIJK *ijk) {
    ijk->i = ij->i;
    ijk->j = ij->j;
    ijk->k = 0;

    if (_ijkNormalizeCouldOverflow(ijk)) {
        return E_FAILED;
    }

    _ijkNormalize(ijk);
    return E_SUCCESS;
}

/**
 * Convert IJK coordinates to cube coordinates, in place
 * @param ijk Coordinate to convert
 */
static inline void ijkToCube(CoordIJK *ijk) {
    ijk->i = -ijk->i + ijk->k;
    ijk->j = ijk->j - ijk->k;
    ijk->k = -ijk->i - ijk->j;
}

/**
 * Convert cube coordinates to IJK coordinates, in place
 * @param ijk Coordinate to convert
 */
static inline void cubeToIjk(CoordIJK *ijk) {
    ijk->i = -ijk->i;
    ijk->k = 0;
    _ijkNormalize(ijk);
}

#undef INT32_MAX_3

#endif
