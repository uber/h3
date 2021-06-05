/*
 * Copyright 2016-2018, 2020-2021 Uber Technologies, Inc.
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
/** @file coordijk.c
 * @brief   Hex IJK coordinate systems functions including conversions to/from
 * lat/lng.
 */

#include "coordijk.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "latLng.h"
#include "mathExtensions.h"

/**
 * Sets an IJK coordinate to the specified component values.
 *
 * @param ijk The IJK coordinate to set.
 * @param i The desired i component value.
 * @param j The desired j component value.
 * @param k The desired k component value.
 */
void _setIJK(CoordIJK *ijk, int i, int j, int k) {
    ijk->i = i;
    ijk->j = j;
    ijk->k = k;
}

/**
 * Determine the containing hex in ijk+ coordinates for a 2D cartesian
 * coordinate vector (from DGGRID).
 *
 * @param v The 2D cartesian coordinate vector.
 * @param h The ijk+ coordinates of the containing hex.
 */
void _hex2dToCoordIJK(const Vec2d *v, CoordIJK *h) {
    double a1, a2;
    double x1, x2;
    int m1, m2;
    double r1, r2;

    // quantize into the ij system and then normalize
    h->k = 0;

    a1 = fabsl(v->x);
    a2 = fabsl(v->y);

    // first do a reverse conversion
    x2 = a2 / M_SIN60;
    x1 = a1 + x2 / 2.0L;

    // check if we have the center of a hex
    m1 = x1;
    m2 = x2;

    // otherwise round correctly
    r1 = x1 - m1;
    r2 = x2 - m2;

    if (r1 < 0.5L) {
        if (r1 < 1.0L / 3.0L) {
            if (r2 < (1.0L + r1) / 2.0L) {
                h->i = m1;
                h->j = m2;
            } else {
                h->i = m1;
                h->j = m2 + 1;
            }
        } else {
            if (r2 < (1.0L - r1)) {
                h->j = m2;
            } else {
                h->j = m2 + 1;
            }

            if ((1.0L - r1) <= r2 && r2 < (2.0 * r1)) {
                h->i = m1 + 1;
            } else {
                h->i = m1;
            }
        }
    } else {
        if (r1 < 2.0L / 3.0L) {
            if (r2 < (1.0L - r1)) {
                h->j = m2;
            } else {
                h->j = m2 + 1;
            }

            if ((2.0L * r1 - 1.0L) < r2 && r2 < (1.0L - r1)) {
                h->i = m1;
            } else {
                h->i = m1 + 1;
            }
        } else {
            if (r2 < (r1 / 2.0L)) {
                h->i = m1 + 1;
                h->j = m2;
            } else {
                h->i = m1 + 1;
                h->j = m2 + 1;
            }
        }
    }

    // now fold across the axes if necessary

    if (v->x < 0.0L) {
        if ((h->j % 2) == 0)  // even
        {
            long long int axisi = h->j / 2;
            long long int diff = h->i - axisi;
            h->i = h->i - 2.0 * diff;
        } else {
            long long int axisi = (h->j + 1) / 2;
            long long int diff = h->i - axisi;
            h->i = h->i - (2.0 * diff + 1);
        }
    }

    if (v->y < 0.0L) {
        h->i = h->i - (2 * h->j + 1) / 2;
        h->j = -1 * h->j;
    }

    _ijkNormalize(h);
}

/**
 * Find the center point in 2D cartesian coordinates of a hex.
 *
 * @param h The ijk coordinates of the hex.
 * @param v The 2D cartesian coordinates of the hex center point.
 */
void _ijkToHex2d(const CoordIJK *h, Vec2d *v) {
    int i = h->i - h->k;
    int j = h->j - h->k;

    v->x = i - 0.5L * j;
    v->y = j * M_SQRT3_2;
}

/**
 * Returns whether or not two ijk coordinates contain exactly the same
 * component values.
 *
 * @param c1 The first set of ijk coordinates.
 * @param c2 The second set of ijk coordinates.
 * @return 1 if the two addresses match, 0 if they do not.
 */
int _ijkMatches(const CoordIJK *c1, const CoordIJK *c2) {
    return (c1->i == c2->i && c1->j == c2->j && c1->k == c2->k);
}

/**
 * Add two ijk coordinates.
 *
 * @param h1 The first set of ijk coordinates.
 * @param h2 The second set of ijk coordinates.
 * @param sum The sum of the two sets of ijk coordinates.
 */
void _ijkAdd(const CoordIJK *h1, const CoordIJK *h2, CoordIJK *sum) {
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
void _ijkSub(const CoordIJK *h1, const CoordIJK *h2, CoordIJK *diff) {
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
void _ijkScale(CoordIJK *c, int factor) {
    c->i *= factor;
    c->j *= factor;
    c->k *= factor;
}

/**
 * Normalizes ijk coordinates by setting the components to the smallest possible
 * values. Works in place.
 *
 * @param c The ijk coordinates to normalize.
 */
void _ijkNormalize(CoordIJK *c) {
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
 * Determines the H3 digit corresponding to a unit vector in ijk coordinates.
 *
 * @param ijk The ijk coordinates; must be a unit vector.
 * @return The H3 digit (0-6) corresponding to the ijk unit vector, or
 * INVALID_DIGIT on failure.
 */
Direction _unitIjkToDigit(const CoordIJK *ijk) {
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
 * Find the normalized ijk coordinates of the indexing parent of a cell in a
 * counter-clockwise aperture 7 grid. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void _upAp7(CoordIJK *ijk) {
    // convert to CoordIJ
    int i = ijk->i - ijk->k;
    int j = ijk->j - ijk->k;

    ijk->i = (int)lroundl((3 * i - j) / 7.0L);
    ijk->j = (int)lroundl((i + 2 * j) / 7.0L);
    ijk->k = 0;
    _ijkNormalize(ijk);
}

/**
 * Find the normalized ijk coordinates of the indexing parent of a cell in a
 * clockwise aperture 7 grid. Works in place.
 *
 * @param ijk The ijk coordinates.
 */
void _upAp7r(CoordIJK *ijk) {
    // convert to CoordIJ
    int i = ijk->i - ijk->k;
    int j = ijk->j - ijk->k;

    ijk->i = (int)lroundl((2 * i + j) / 7.0L);
    ijk->j = (int)lroundl((3 * j - i) / 7.0L);
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
void _downAp7(CoordIJK *ijk) {
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
void _downAp7r(CoordIJK *ijk) {
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
 * Find the normalized ijk coordinates of the hex in the specified digit
 * direction from the specified ijk coordinates. Works in place.
 *
 * @param ijk The ijk coordinates.
 * @param digit The digit direction from the original ijk coordinates.
 */
void _neighbor(CoordIJK *ijk, Direction digit) {
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
void _ijkRotate60ccw(CoordIJK *ijk) {
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
void _ijkRotate60cw(CoordIJK *ijk) {
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
Direction _rotate60ccw(Direction digit) {
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
Direction _rotate60cw(Direction digit) {
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
 * Find the normalized ijk coordinates of the hex centered on the indicated
 * hex at the next finer aperture 3 counter-clockwise resolution. Works in
 * place.
 *
 * @param ijk The ijk coordinates.
 */
void _downAp3(CoordIJK *ijk) {
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
void _downAp3r(CoordIJK *ijk) {
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
 * Finds the distance between the two coordinates. Returns result.
 *
 * @param c1 The first set of ijk coordinates.
 * @param c2 The second set of ijk coordinates.
 */
int ijkDistance(const CoordIJK *c1, const CoordIJK *c2) {
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
void ijkToIj(const CoordIJK *ijk, CoordIJ *ij) {
    ij->i = ijk->i - ijk->k;
    ij->j = ijk->j - ijk->k;
}

/**
 * Transforms coordinates from the IJ coordinate system to the IJK+ coordinate
 * system.
 *
 * @param ij The input IJ coordinates
 * @param ijk The output IJK+ coordinates
 */
void ijToIjk(const CoordIJ *ij, CoordIJK *ijk) {
    ijk->i = ij->i;
    ijk->j = ij->j;
    ijk->k = 0;

    _ijkNormalize(ijk);
}

/**
 * Convert IJK coordinates to cube coordinates, in place
 * @param ijk Coordinate to convert
 */
void ijkToCube(CoordIJK *ijk) {
    ijk->i = -ijk->i + ijk->k;
    ijk->j = ijk->j - ijk->k;
    ijk->k = -ijk->i - ijk->j;
}

/**
 * Convert cube coordinates to IJK coordinates, in place
 * @param ijk Coordinate to convert
 */
void cubeToIjk(CoordIJK *ijk) {
    ijk->i = -ijk->i;
    ijk->k = 0;
    _ijkNormalize(ijk);
}
