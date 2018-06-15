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
/** @file algos.c
 * @brief   Hexagon grid algorithms
 */

#include "algos.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "baseCells.h"
#include "bbox.h"
#include "faceijk.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "h3api.h"
#include "linkedGeo.h"
#include "stackAlloc.h"
#include "vertexGraph.h"

/*
 * Return codes from hexRange and related functions.
 */

#define HEX_RANGE_SUCCESS 0
#define HEX_RANGE_PENTAGON 1
#define HEX_RANGE_K_SUBSEQUENCE 2

/**
 * Directions used for traversing a hexagonal ring counterclockwise around
 * {1, 0, 0}
 *
 * <pre>
 *      _
 *    _/ \\_
 *   / \\5/ \\
 *   \\0/ \\4/
 *   / \\_/ \\
 *   \\1/ \\3/
 *     \\2/
 * </pre>
 */
static const Direction DIRECTIONS[6] = {J_AXES_DIGIT, JK_AXES_DIGIT,
                                        K_AXES_DIGIT, IK_AXES_DIGIT,
                                        I_AXES_DIGIT, IJ_AXES_DIGIT};

/**
 * Direction used for traversing to the next outward hexagonal ring.
 */
static const Direction NEXT_RING_DIRECTION = I_AXES_DIGIT;

/**
 * New digit when traversing along class II grids.
 *
 * Current digit -> direction -> new digit.
 */
static const Direction NEW_DIGIT_II[7][7] = {
    {CENTER_DIGIT, K_AXES_DIGIT, J_AXES_DIGIT, JK_AXES_DIGIT, I_AXES_DIGIT,
     IK_AXES_DIGIT, IJ_AXES_DIGIT},
    {K_AXES_DIGIT, I_AXES_DIGIT, JK_AXES_DIGIT, IJ_AXES_DIGIT, IK_AXES_DIGIT,
     J_AXES_DIGIT, CENTER_DIGIT},
    {J_AXES_DIGIT, JK_AXES_DIGIT, K_AXES_DIGIT, I_AXES_DIGIT, IJ_AXES_DIGIT,
     CENTER_DIGIT, IK_AXES_DIGIT},
    {JK_AXES_DIGIT, IJ_AXES_DIGIT, I_AXES_DIGIT, IK_AXES_DIGIT, CENTER_DIGIT,
     K_AXES_DIGIT, J_AXES_DIGIT},
    {I_AXES_DIGIT, IK_AXES_DIGIT, IJ_AXES_DIGIT, CENTER_DIGIT, J_AXES_DIGIT,
     JK_AXES_DIGIT, K_AXES_DIGIT},
    {IK_AXES_DIGIT, J_AXES_DIGIT, CENTER_DIGIT, K_AXES_DIGIT, JK_AXES_DIGIT,
     IJ_AXES_DIGIT, I_AXES_DIGIT},
    {IJ_AXES_DIGIT, CENTER_DIGIT, IK_AXES_DIGIT, J_AXES_DIGIT, K_AXES_DIGIT,
     I_AXES_DIGIT, JK_AXES_DIGIT}};

/**
 * New traversal direction when traversing along class II grids.
 *
 * Current digit -> direction -> new ap7 move (at coarser level).
 */
static const Direction NEW_ADJUSTMENT_II[7][7] = {
    {CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT,
     CENTER_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, K_AXES_DIGIT, CENTER_DIGIT, K_AXES_DIGIT, CENTER_DIGIT,
     IK_AXES_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, CENTER_DIGIT, J_AXES_DIGIT, JK_AXES_DIGIT, CENTER_DIGIT,
     CENTER_DIGIT, J_AXES_DIGIT},
    {CENTER_DIGIT, K_AXES_DIGIT, JK_AXES_DIGIT, JK_AXES_DIGIT, CENTER_DIGIT,
     CENTER_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, I_AXES_DIGIT,
     I_AXES_DIGIT, IJ_AXES_DIGIT},
    {CENTER_DIGIT, IK_AXES_DIGIT, CENTER_DIGIT, CENTER_DIGIT, I_AXES_DIGIT,
     IK_AXES_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, CENTER_DIGIT, J_AXES_DIGIT, CENTER_DIGIT, IJ_AXES_DIGIT,
     CENTER_DIGIT, IJ_AXES_DIGIT}};

/**
 * New traversal direction when traversing along class III grids.
 *
 * Current digit -> direction -> new ap7 move (at coarser level).
 */
static const Direction NEW_DIGIT_III[7][7] = {
    {CENTER_DIGIT, K_AXES_DIGIT, J_AXES_DIGIT, JK_AXES_DIGIT, I_AXES_DIGIT,
     IK_AXES_DIGIT, IJ_AXES_DIGIT},
    {K_AXES_DIGIT, J_AXES_DIGIT, JK_AXES_DIGIT, I_AXES_DIGIT, IK_AXES_DIGIT,
     IJ_AXES_DIGIT, CENTER_DIGIT},
    {J_AXES_DIGIT, JK_AXES_DIGIT, I_AXES_DIGIT, IK_AXES_DIGIT, IJ_AXES_DIGIT,
     CENTER_DIGIT, K_AXES_DIGIT},
    {JK_AXES_DIGIT, I_AXES_DIGIT, IK_AXES_DIGIT, IJ_AXES_DIGIT, CENTER_DIGIT,
     K_AXES_DIGIT, J_AXES_DIGIT},
    {I_AXES_DIGIT, IK_AXES_DIGIT, IJ_AXES_DIGIT, CENTER_DIGIT, K_AXES_DIGIT,
     J_AXES_DIGIT, JK_AXES_DIGIT},
    {IK_AXES_DIGIT, IJ_AXES_DIGIT, CENTER_DIGIT, K_AXES_DIGIT, J_AXES_DIGIT,
     JK_AXES_DIGIT, I_AXES_DIGIT},
    {IJ_AXES_DIGIT, CENTER_DIGIT, K_AXES_DIGIT, J_AXES_DIGIT, JK_AXES_DIGIT,
     I_AXES_DIGIT, IK_AXES_DIGIT}};

/**
 * New traversal direction when traversing along class III grids.
 *
 * Current digit -> direction -> new ap7 move (at coarser level).
 */
static const Direction NEW_ADJUSTMENT_III[7][7] = {
    {CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT,
     CENTER_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, K_AXES_DIGIT, CENTER_DIGIT, JK_AXES_DIGIT, CENTER_DIGIT,
     K_AXES_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, CENTER_DIGIT, J_AXES_DIGIT, J_AXES_DIGIT, CENTER_DIGIT,
     CENTER_DIGIT, IJ_AXES_DIGIT},
    {CENTER_DIGIT, JK_AXES_DIGIT, J_AXES_DIGIT, JK_AXES_DIGIT, CENTER_DIGIT,
     CENTER_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, CENTER_DIGIT, I_AXES_DIGIT,
     IK_AXES_DIGIT, I_AXES_DIGIT},
    {CENTER_DIGIT, K_AXES_DIGIT, CENTER_DIGIT, CENTER_DIGIT, IK_AXES_DIGIT,
     IK_AXES_DIGIT, CENTER_DIGIT},
    {CENTER_DIGIT, CENTER_DIGIT, IJ_AXES_DIGIT, CENTER_DIGIT, I_AXES_DIGIT,
     CENTER_DIGIT, IJ_AXES_DIGIT}};

/**
 * Maximum number of indices that result from the kRing algorithm with the given
 * k.
 *
 * @param k k value, k >= 0.
 */
int H3_EXPORT(maxKringSize)(int k) {
    int result = 1;
    for (int i = 0; i < k; i++) {
        result = result + 6 * (i + 1);
    }
    return result;
}

/**
 * k-rings produces indices within k distance of the origin index.
 *
 * k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
 * all neighboring indices, and so on.
 *
 * Output is placed in the provided array in no particular order. Elements of
 * the output array may be left zero, as can happen when crossing a pentagon.
 *
 * @param origin Origin location.
 * @param k k >= 0
 * @param out Zero-filled array which must be of size maxKringSize(k).
 */
void H3_EXPORT(kRing)(H3Index origin, int k, H3Index* out) {
    int maxIdx = H3_EXPORT(maxKringSize)(k);
    STACK_ARRAY_CALLOC(int, distances, maxIdx);
    H3_EXPORT(kRingDistances)(origin, k, out, distances);
}

/**
 * k-rings produces indices within k distance of the origin index.
 *
 * k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
 * all neighboring indices, and so on.
 *
 * Output is placed in the provided array in no particular order. Elements of
 * the output array may be left zero, as can happen when crossing a pentagon.
 *
 * @param origin Origin location.
 * @param k k >= 0
 * @param out Zero-filled array which must be of size maxKringSize(k).
 * @param distances Zero-filled array which must be of size maxKringSize(k).
 */
void H3_EXPORT(kRingDistances)(H3Index origin, int k, H3Index* out,
                               int* distances) {
    int maxIdx = H3_EXPORT(maxKringSize)(k);
    // Optimistically try the faster hexRange algorithm first
    int failed = H3_EXPORT(hexRangeDistances)(origin, k, out, distances);
    if (failed) {
        // Fast algo failed, fall back to slower, correct algo
        // and also wipe out array because contents untrustworthy
        for (int i = 0; i < maxIdx; i++) {
            out[i] = H3_INVALID_INDEX;
            distances[i] = 0;
        }
        _kRingInternal(origin, k, out, distances, maxIdx, 0);
    }
}

/**
 * Internal helper function called recursively for kRingDistances.
 *
 * Adds the origin index to the output set (treating it as a hash set)
 * and recurses to its neighbors, if needed.
 *
 * @param origin
 * @param k Maximum distance to move from the origin.
 * @param out Array treated as a hash set, elements being either H3Index or 0.
 * @param distances Scratch area, with elements paralleling the out array.
 * Elements indicate ijk distance from the origin index to the output index.
 * @param maxIdx Size of out and scratch arrays (must be maxKringSize(k))
 * @param curK Current distance from the origin.
 */
void _kRingInternal(H3Index origin, int k, H3Index* out, int* distances,
                    int maxIdx, int curK) {
    if (origin == 0) return;

    // Put origin in the output array. out is used as a hash set.
    int off = origin % maxIdx;
    while (out[off] != 0 && out[off] != origin) {
        off++;
        if (off >= maxIdx) {
            off = 0;
        }
    }

    // We either got a free slot in the hash set or hit a duplicate
    // We might need to process the duplicate anyways because we got
    // here on a longer path before.
    if (out[off] == origin && distances[off] <= curK) return;

    out[off] = origin;
    distances[off] = curK;

    // Base case: reached an index k away from the origin.
    if (curK >= k) return;

    // Recurse to all neighbors in no particular order.
    for (int i = 0; i < 6; i++) {
        int rotations = 0;
        _kRingInternal(h3NeighborRotations(origin, DIRECTIONS[i], &rotations),
                       k, out, distances, maxIdx, curK + 1);
    }
}

/**
 * Returns the hexagon index neighboring the origin, in the direction dir.
 *
 * Implementation note: The only reachable case where this returns 0 is if the
 * origin is a pentagon and the translation is in the k direction. Thus,
 * 0 can only be returned if origin is a pentagon.
 *
 * @param origin Origin index
 * @param dir Direction to move in
 * @param rotations Number of ccw rotations to perform to reorient the
 *                  translation vector. Will be modified to the new number of
 *                  rotations to perform (such as when crossing a face edge.)
 * @return H3Index of the specified neighbor or 0 if deleted k-subsequence
 *         distortion is encountered.
 */
H3Index h3NeighborRotations(H3Index origin, Direction dir, int* rotations) {
    H3Index out = origin;

    for (int i = 0; i < *rotations; i++) {
        dir = _rotate60ccw(dir);
    }

    int newRotations = 0;
    int oldBaseCell = H3_GET_BASE_CELL(out);
    Direction oldLeadingDigit = _h3LeadingNonZeroDigit(out);

    // Adjust the indexing digits and, if needed, the base cell.
    int r = H3_GET_RESOLUTION(out) - 1;
    while (true) {
        if (r == -1) {
            H3_SET_BASE_CELL(out, baseCellNeighbors[oldBaseCell][dir]);
            newRotations = baseCellNeighbor60CCWRots[oldBaseCell][dir];

            if (H3_GET_BASE_CELL(out) == INVALID_BASE_CELL) {
                // Adjust for the deleted k vertex at the base cell level.
                // This edge actually borders a different neighbor.
                H3_SET_BASE_CELL(out,
                                 baseCellNeighbors[oldBaseCell][IK_AXES_DIGIT]);
                newRotations =
                    baseCellNeighbor60CCWRots[oldBaseCell][IK_AXES_DIGIT];

                // perform the adjustment for the k-subsequence we're skipping
                // over.
                out = _h3Rotate60ccw(out);
                *rotations = *rotations + 1;
            }

            break;
        } else {
            Direction oldDigit = H3_GET_INDEX_DIGIT(out, r + 1);
            Direction nextDir;
            if (isResClassIII(r + 1)) {
                H3_SET_INDEX_DIGIT(out, r + 1, NEW_DIGIT_II[oldDigit][dir]);
                nextDir = NEW_ADJUSTMENT_II[oldDigit][dir];
            } else {
                H3_SET_INDEX_DIGIT(out, r + 1, NEW_DIGIT_III[oldDigit][dir]);
                nextDir = NEW_ADJUSTMENT_III[oldDigit][dir];
            }

            if (nextDir != CENTER_DIGIT) {
                dir = nextDir;
                r--;
            } else {
                // No more adjustment to perform
                break;
            }
        }
    }

    int newBaseCell = H3_GET_BASE_CELL(out);
    if (_isBaseCellPentagon(newBaseCell)) {
        int alreadyAdjustedKSubsequence = 0;

        // force rotation out of missing k-axes sub-sequence
        if (_h3LeadingNonZeroDigit(out) == K_AXES_DIGIT) {
            if (oldBaseCell != newBaseCell) {
                // in this case, we traversed into the deleted
                // k subsequence of a pentagon base cell.
                // We need to rotate out of that case depending
                // on how we got here.
                // check for a cw/ccw offset face; default is ccw

                if (_baseCellIsCwOffset(
                        newBaseCell, baseCellData[oldBaseCell].homeFijk.face)) {
                    out = _h3Rotate60cw(out);
                } else {
                    // See cwOffsetPent in testKRing.c for why this is
                    // unreachable.
                    out = _h3Rotate60ccw(out);  // LCOV_EXCL_LINE
                }
                alreadyAdjustedKSubsequence = 1;
            } else {
                // In this case, we traversed into the deleted
                // k subsequence from within the same pentagon
                // base cell.
                if (oldLeadingDigit == CENTER_DIGIT) {
                    // Undefined: the k direction is deleted from here
                    return H3_INVALID_INDEX;
                } else if (oldLeadingDigit == JK_AXES_DIGIT) {
                    // Rotate out of the deleted k subsequence
                    // We also need an additional change to the direction we're
                    // moving in
                    out = _h3Rotate60ccw(out);
                    *rotations = *rotations + 1;
                } else if (oldLeadingDigit == IK_AXES_DIGIT) {
                    // Rotate out of the deleted k subsequence
                    // We also need an additional change to the direction we're
                    // moving in
                    out = _h3Rotate60cw(out);
                    *rotations = *rotations + 5;
                } else {
                    // Should never occur
                    return H3_INVALID_INDEX;  // LCOV_EXCL_LINE
                }
            }
        }

        for (int i = 0; i < newRotations; i++) out = _h3RotatePent60ccw(out);

        // Account for differing orientation of the base cells (this edge
        // might not follow properties of some other edges.)
        if (oldBaseCell != newBaseCell) {
            if (newBaseCell == 4 || newBaseCell == 117) {
                // 'polar' base cells behave differently because they have all
                // i neighbors.
                if (oldBaseCell != 118 && oldBaseCell != 8 &&
                    _h3LeadingNonZeroDigit(out) != JK_AXES_DIGIT) {
                    *rotations = *rotations + 1;
                }
            } else if (_h3LeadingNonZeroDigit(out) == IK_AXES_DIGIT &&
                       !alreadyAdjustedKSubsequence) {
                // account for distortion introduced to the 5 neighbor by the
                // deleted k subsequence.
                *rotations = *rotations + 1;
            }
        }
    } else {
        for (int i = 0; i < newRotations; i++) out = _h3Rotate60ccw(out);
    }

    *rotations = (*rotations + newRotations) % 6;

    return out;
}

/**
 * hexRange produces indexes within k distance of the origin index.
 * Output behavior is undefined when one of the indexes returned by this
 * function is a pentagon or is in the pentagon distortion area.
 *
 * k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
 * all neighboring indexes, and so on.
 *
 * Output is placed in the provided array in order of increasing distance from
 * the origin.
 *
 * @param origin Origin location.
 * @param k k >= 0
 * @param out Array which must be of size maxKringSize(k).
 * @return 0 if no pentagon or pentagonal distortion area was encountered.
 */
int H3_EXPORT(hexRange)(H3Index origin, int k, H3Index* out) {
    return H3_EXPORT(hexRangeDistances)(origin, k, out, 0);
}

/**
 * hexRange produces indexes within k distance of the origin index.
 * Output behavior is undefined when one of the indexes returned by this
 * function is a pentagon or is in the pentagon distortion area.
 *
 * k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
 * all neighboring indexes, and so on.
 *
 * Output is placed in the provided array in order of increasing distance from
 * the origin. The distances in hexagons is placed in the distances array at
 * the same offset.
 *
 * @param origin Origin location.
 * @param k k >= 0
 * @param out Array which must be of size maxKringSize(k).
 * @param distances Null or array which must be of size maxKringSize(k).
 * @return 0 if no pentagon or pentagonal distortion area was encountered.
 */
int H3_EXPORT(hexRangeDistances)(H3Index origin, int k, H3Index* out,
                                 int* distances) {
    // Return codes:
    // 1 Pentagon was encountered
    // 2 Pentagon distortion (deleted k subsequence) was encountered
    // Pentagon being encountered is not itself a problem; really the deleted
    // k-subsequence is the problem, but for compatibility reasons we fail on
    // the pentagon.

    // k must be >= 0, so origin is always needed
    int idx = 0;
    out[idx] = origin;
    if (distances) {
        distances[idx] = 0;
    }
    idx++;

    if (H3_EXPORT(h3IsPentagon)(origin)) {
        // Pentagon was encountered; bail out as user doesn't want this.
        return HEX_RANGE_PENTAGON;
    }

    // 0 < ring <= k, current ring
    int ring = 1;
    // 0 <= direction < 6, current side of the ring
    int direction = 0;
    // 0 <= i < ring, current position on the side of the ring
    int i = 0;
    // Number of 60 degree ccw rotations to perform on the direction (based on
    // which faces have been crossed.)
    int rotations = 0;

    while (ring <= k) {
        if (direction == 0 && i == 0) {
            // Not putting in the output set as it will be done later, at
            // the end of this ring.
            origin =
                h3NeighborRotations(origin, NEXT_RING_DIRECTION, &rotations);
            if (origin == 0) {
                // Should not be possible because `origin` would have to be a
                // pentagon
                return HEX_RANGE_K_SUBSEQUENCE;  // LCOV_EXCL_LINE
            }

            if (H3_EXPORT(h3IsPentagon)(origin)) {
                // Pentagon was encountered; bail out as user doesn't want this.
                return HEX_RANGE_PENTAGON;
            }
        }

        origin = h3NeighborRotations(origin, DIRECTIONS[direction], &rotations);
        if (origin == 0) {
            // Should not be possible because `origin` would have to be a
            // pentagon
            return HEX_RANGE_K_SUBSEQUENCE;  // LCOV_EXCL_LINE
        }
        out[idx] = origin;
        if (distances) {
            distances[idx] = ring;
        }
        idx++;

        i++;
        // Check if end of this side of the k-ring
        if (i == ring) {
            i = 0;
            direction++;
            // Check if end of this ring.
            if (direction == 6) {
                direction = 0;
                ring++;
            }
        }

        if (H3_EXPORT(h3IsPentagon)(origin)) {
            // Pentagon was encountered; bail out as user doesn't want this.
            return HEX_RANGE_PENTAGON;
        }
    }
    return HEX_RANGE_SUCCESS;
}

/**
 * hexRanges takes an array of input hex IDs and a max k-ring and returns an
 * array of hexagon IDs sorted first by the original hex IDs and then by the
 * k-ring (0 to max), with no guaranteed sorting within each k-ring group.
 *
 * @param h3Set A pointer to an array of H3Indexes
 * @param length The total number of H3Indexes in h3Set
 * @param k The number of rings to generate
 * @param out A pointer to the output memory to dump the new set of H3Indexes to
 *            The memory block should be equal to maxKringSize(k) * length
 * @return 0 if no pentagon is encountered. Cannot trust output otherwise
 */
int H3_EXPORT(hexRanges)(H3Index* h3Set, int length, int k, H3Index* out) {
    int success = 0;
    H3Index* segment;
    int segmentSize = H3_EXPORT(maxKringSize)(k);
    for (int i = 0; i < length; i++) {
        // Determine the appropriate segment of the output array to operate on
        segment = out + i * segmentSize;
        success = H3_EXPORT(hexRange)(h3Set[i], k, segment);
        if (success != 0) return success;
    }
    return 0;
}

/**
 * Returns the hollow hexagonal ring centered at origin with sides of length k.
 *
 * @param origin Origin location.
 * @param k k >= 0
 * @param out Array which must be of size 6 * k (or 1 if k == 0)
 * @return 0 if no pentagonal distortion was encountered.
 */
int H3_EXPORT(hexRing)(H3Index origin, int k, H3Index* out) {
    // Short-circuit on 'identity' ring
    if (k == 0) {
        out[0] = origin;
        return 0;
    }
    int idx = 0;
    // Number of 60 degree ccw rotations to perform on the direction (based on
    // which faces have been crossed.)
    int rotations = 0;
    // Scratch structure for checking for pentagons
    if (H3_EXPORT(h3IsPentagon)(origin)) {
        // Pentagon was encountered; bail out as user doesn't want this.
        return HEX_RANGE_PENTAGON;
    }

    for (int ring = 0; ring < k; ring++) {
        origin = h3NeighborRotations(origin, NEXT_RING_DIRECTION, &rotations);
        if (origin == 0) {
            // Should not be possible because `origin` would have to be a
            // pentagon
            return HEX_RANGE_K_SUBSEQUENCE;  // LCOV_EXCL_LINE
        }

        if (H3_EXPORT(h3IsPentagon)(origin)) {
            return HEX_RANGE_PENTAGON;
        }
    }

    H3Index lastIndex = origin;

    out[idx] = origin;
    idx++;

    for (int direction = 0; direction < 6; direction++) {
        for (int pos = 0; pos < k; pos++) {
            origin =
                h3NeighborRotations(origin, DIRECTIONS[direction], &rotations);
            if (origin == 0) {
                // Should not be possible because `origin` would have to be a
                // pentagon
                return HEX_RANGE_K_SUBSEQUENCE;  // LCOV_EXCL_LINE
            }

            // Skip the very last index, it was already added. We do
            // however need to traverse to it because of the pentagonal
            // distortion check, below.
            if (pos != k - 1 || direction != 5) {
                out[idx] = origin;
                idx++;

                if (H3_EXPORT(h3IsPentagon)(origin)) {
                    return HEX_RANGE_PENTAGON;
                }
            }
        }
    }

    // Check that this matches the expected lastIndex, if it doesn't,
    // it indicates pentagonal distortion occurred and we should report
    // failure.
    if (lastIndex != origin) {
        return HEX_RANGE_PENTAGON;
    } else {
        return HEX_RANGE_SUCCESS;
    }
}

/**
 * maxPolyfillSize returns the number of hexagons to allocate space for when
 * performing a polyfill on the given GeoJSON-like data structure.
 *
 * Currently a laughably padded response, being a k-ring that wholly contains
 * a bounding box of the GeoJSON, but still less wasted memory than initializing
 * a Python application? ;)
 *
 * @param geoPolygon A GeoJSON-like data structure indicating the poly to fill
 * @param res Hexagon resolution (0-15)
 * @return number of hexagons to allocate for
 */
int H3_EXPORT(maxPolyfillSize)(const GeoPolygon* geoPolygon, int res) {
    // Get the bounding box for the GeoJSON-like struct
    BBox bbox;
    bboxFromGeofence(&geoPolygon->geofence, &bbox);
    int minK = bboxHexRadius(&bbox, res);

    // The total number of hexagons to allocate can now be determined by
    // the k-ring hex allocation helper function.
    return H3_EXPORT(maxKringSize)(minK);
}

/**
 * Normalize a longitude value, converting it into a comparable value in
 * transmeridian cases.
 * @param  lng Longitude to normalize
 * @param  isTransmeridian Whether this longitude is part of a transmeridian
 *                         polygon
 * @return Normalized longitude
 */
double _normalizeLng(double lng, bool isTransmeridian) {
    return isTransmeridian && lng < 0 ? lng + M_2PI : lng;
}

/**
 * _pointInPolyContainsLoop is the core loop of the pointInPolyContains
 * algorithm, working on a Geofence struct
 *
 * @param geofence The geofence to check
 * @param bbox The bbox for the loop being tested
 * @param coord The coordinate to check if contained by the geofence
 * @return true or false
 */
bool _pointInPolyContainsLoop(const Geofence* geofence, const BBox* bbox,
                              const GeoCoord* coord) {
    // fail fast if we're outside the bounding box
    if (!bboxContains(bbox, coord)) {
        return false;
    }
    bool isTransmeridian = bboxIsTransmeridian(bbox);
    bool contains = false;

    double lat = coord->lat;
    double lng = _normalizeLng(coord->lon, isTransmeridian);

    for (int i = 0; i < geofence->numVerts; i++) {
        GeoCoord a = geofence->verts[i];
        GeoCoord b;
        if (i + 1 == geofence->numVerts) {
            b = geofence->verts[0];
        } else {
            b = geofence->verts[i + 1];
        }

        // Ray casting algo requires the second point to always be higher
        // than the first, so swap if needed
        if (a.lat > b.lat) {
            a = b;
            b = geofence->verts[i];
        }

        // If we're totally above or below the latitude ranges, the test
        // ray cannot intersect the line segment, so let's move on
        if (lat < a.lat || lat > b.lat) {
            continue;
        }

        double aLng = _normalizeLng(a.lon, isTransmeridian);
        double bLng = _normalizeLng(b.lon, isTransmeridian);

        // Rays are cast in the longitudinal direction, in case a point
        // exactly matches, to decide tiebreakers, bias westerly
        if (aLng == lng || bLng == lng) {
            lng -= DBL_EPSILON;
        }

        // For the latitude of the point, compute the longitude of the
        // point that lies on the line segment defined by a and b
        // This is done by computing the percent above a the lat is,
        // and traversing the same percent in the longitudinal direction
        // of a to b
        double ratio = (lat - a.lat) / (b.lat - a.lat);
        double testLng =
            _normalizeLng(aLng + (bLng - aLng) * ratio, isTransmeridian);

        // Intersection of the ray
        if (testLng > lng) {
            contains = !contains;
        };
    }

    return contains;
}

/**
 * pointInPolyContains takes a given GeoJSON-like data structure
 * and a point, and checks if said point is contained in the GeoJSON-like
 * struct.
 *
 * @param geoPolygon The geofence and holes defining the relevant area
 * @param bboxes The bboxes for the main geofence and each of its holes
 * @param coord The coordinate to check if contained by the geoJson-like
 * struct
 * @return true or false
 */
bool _pointInPolyContains(const GeoPolygon* geoPolygon, const BBox* bboxes,
                          const GeoCoord* coord) {
    // Start with contains state of primary geofence
    bool contains =
        _pointInPolyContainsLoop(&(geoPolygon->geofence), &bboxes[0], coord);

    // If the point is contained in the primary geofence, but there are holes in
    // the geofence iterate through all holes and return false if the point is
    // contained in any hole
    if (contains && geoPolygon->numHoles > 0) {
        for (int i = 0; i < geoPolygon->numHoles; i++) {
            if (_pointInPolyContainsLoop(&(geoPolygon->holes[i]),
                                         &bboxes[i + 1], coord)) {
                return false;
            }
        }
    }

    return contains;
}

/**
 * polyfill takes a given GeoJSON-like data structure and preallocated,
 * zeroed memory, and fills it with the hexagons that are contained by
 * the GeoJSON-like data structure.
 *
 * The current implementation is very primitive and slow, but correct,
 * performing a point-in-poly operation on every hexagon in a k-ring defined
 * around the given geofence.
 *
 * @param geoPolygon The geofence and holes defining the relevant area
 * @param res The Hexagon resolution (0-15)
 * @param out The slab of zeroed memory to write to. Assumed to be big enough.
 */
void H3_EXPORT(polyfill)(const GeoPolygon* geoPolygon, int res, H3Index* out) {
    // One of the goals of the polyfill algorithm is that two adjacent polygons
    // with zero overlap have zero overlapping hexagons. That the hexagons are
    // uniquely assigned. There are a few approaches to take here, such as
    // deciding based on which polygon has the greatest overlapping area of the
    // hexagon, or the most number of contained points on the hexagon (using the
    // center point as a tiebreaker).
    //
    // But if the polygons are convex, both of these more complex algorithms can
    // be reduced down to checking whether or not the center of the hexagon is
    // contained in the polygon, and so this is the approach that this polyfill
    // algorithm will follow, as it's simpler, faster, and the error for concave
    // polygons is still minimal (only affecting concave shapes on the order of
    // magnitude of the hexagon size or smaller, not impacting larger concave
    // shapes)
    //
    // This first part is identical to the maxPolyfillSize above.

    // Get the bounding boxes for the polygon and any holes
    STACK_ARRAY_CALLOC(BBox, bboxes, geoPolygon->numHoles + 1);
    bboxesFromGeoPolygon(geoPolygon, bboxes);
    int minK = bboxHexRadius(&bboxes[0], res);
    int numHexagons = H3_EXPORT(maxKringSize)(minK);

    // Get the center hex
    GeoCoord center;
    bboxCenter(&bboxes[0], &center);
    H3Index centerH3 = H3_EXPORT(geoToH3)(&center, res);

    // From here on it works differently, first we get all potential
    // hexagons inserted into the available memory
    H3_EXPORT(kRing)(centerH3, minK, out);

    // Next we iterate through each hexagon, and test its center point to see if
    // it's contained in the GeoJSON-like struct
    for (int i = 0; i < numHexagons; i++) {
        // Skip records that are already zeroed
        if (out[i] == 0) {
            continue;
        }
        // Check if hexagon is inside of polygon
        GeoCoord hexCenter;
        H3_EXPORT(h3ToGeo)(out[i], &hexCenter);
        hexCenter.lat = constrainLat(hexCenter.lat);
        hexCenter.lon = constrainLng(hexCenter.lon);
        // And remove from list if not
        if (!_pointInPolyContains(geoPolygon, bboxes, &hexCenter)) {
            out[i] = H3_INVALID_INDEX;
        }
    }
}

/**
 * Internal: Create a vertex graph from a set of hexagons. It is the
 * responsibility of the caller to call destroyVertexGraph on the populated
 * graph, otherwise the memory in the graph nodes will not be freed.
 * @private
 * @param h3Set    Set of hexagons
 * @param numHexes Number of hexagons in the set
 * @param graph    Output graph
 */
void h3SetToVertexGraph(const H3Index* h3Set, const int numHexes,
                        VertexGraph* graph) {
    GeoBoundary vertices;
    GeoCoord* fromVtx;
    GeoCoord* toVtx;
    VertexNode* edge;
    if (numHexes < 1) {
        // We still need to init the graph, or calls to destroyVertexGraph will
        // fail
        initVertexGraph(graph, 0, 0);
        return;
    }
    int res = H3_GET_RESOLUTION(h3Set[0]);
    const int minBuckets = 6;
    // TODO: Better way to calculate/guess?
    int numBuckets = numHexes > minBuckets ? numHexes : minBuckets;
    initVertexGraph(graph, numBuckets, res);
    // Iterate through every hexagon
    for (int i = 0; i < numHexes; i++) {
        H3_EXPORT(h3ToGeoBoundary)(h3Set[i], &vertices);
        // iterate through every edge
        for (int j = 0; j < vertices.numVerts; j++) {
            fromVtx = &vertices.verts[j];
            toVtx = &vertices.verts[(j + 1) % vertices.numVerts];
            // If we've seen this edge already, it will be reversed
            edge = findNodeForEdge(graph, toVtx, fromVtx);
            if (edge != NULL) {
                // If we've seen it, drop it. No edge is shared by more than 2
                // hexagons, so we'll never see it again.
                removeVertexNode(graph, edge);
            } else {
                // Add a new node for this edge
                addVertexNode(graph, fromVtx, toVtx);
            }
        }
    }
}

/**
 * Internal: Create a LinkedGeoPolygon from a vertex graph. It is the
 * responsibility of the caller to call destroyLinkedPolygon on the populated
 * linked geo structure, or the memory for that structure will not be freed.
 * @private
 * @param graph Input graph
 * @param out   Output polygon
 */
void _vertexGraphToLinkedGeo(VertexGraph* graph, LinkedGeoPolygon* out) {
    initLinkedPolygon(out);
    LinkedGeoLoop* loop;
    VertexNode* edge;
    GeoCoord nextVtx;
    // Find the next unused entry point
    while ((edge = firstVertexNode(graph)) != NULL) {
        loop = addLinkedLoop(out);
        // Walk the graph to get the outline
        do {
            addLinkedCoord(loop, &edge->from);
            nextVtx = edge->to;
            // Remove frees the node, so we can't use edge after this
            removeVertexNode(graph, edge);
            edge = findNodeForVertex(graph, &nextVtx);
        } while (edge);
    }
}

/**
 * Create a LinkedGeoPolygon describing the outline(s) of a set of  hexagons.
 * It is the responsibility of the caller to call destroyLinkedPolygon on the
 * populated linked geo structure, or the memory for that structure will
 * not be freed.
 *
 * It is expected that all hexagons in the set will have the same resolution.
 * If you pass in hexagons at different resolutions, the algorithm should work
 * fine, but we won't be able to merge the outlines of different-resolution
 * hexagons, so you might get overlap. I'd suggest not doing this.
 *
 * TODO: At present, if the set of hexagons is not contiguous, this function
 * will return a single polygon with multiple outer loops. The correct GeoJSON
 * output should only have one outer loop per polygon. It appears that most
 * GeoJSON consumers are fine with the first input format, but it's less correct
 * than the second format, and we should update the function to produce
 * multiple polygons in that case.
 *
 * @param h3Set    Set of hexagons
 * @param numHexes Number of hexagons in set
 * @param out      Output polygon
 */
void H3_EXPORT(h3SetToLinkedGeo)(const H3Index* h3Set, const int numHexes,
                                 LinkedGeoPolygon* out) {
    VertexGraph graph;
    h3SetToVertexGraph(h3Set, numHexes, &graph);
    _vertexGraphToLinkedGeo(&graph, out);
    destroyVertexGraph(&graph);
}
