/*
 * Copyright 2016-2019 Uber Technologies, Inc.
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

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "baseCells.h"
#include "bbox.h"
#include "faceijk.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "h3api.h"
#include "linkedGeo.h"
#include "polygon.h"
#include "vertexGraph.h"

/*
 * Return codes from hexRange and related functions.
 */

#define HEX_RANGE_SUCCESS 0
#define HEX_RANGE_PENTAGON 1
#define HEX_RANGE_K_SUBSEQUENCE 2
#define MAX_ONE_RING_SIZE 7
#define HEX_HASH_OVERFLOW -1
#define POLYFILL_BUFFER 12

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
 * Maximum number of cells that result from the kRing algorithm with the given
 * k. Formula source and proof: https://oeis.org/A003215
 *
 * @param  k   k value, k >= 0.
 */
int H3_EXPORT(maxKringSize)(int k) { return 3 * k * (k + 1) + 1; }

/**
 * Produce cells within grid distance k of the origin cell.
 *
 * k-ring 0 is defined as the origin cell, k-ring 1 is defined as k-ring 0 and
 * all neighboring cells, and so on.
 *
 * Output is placed in the provided array in no particular order. Elements of
 * the output array may be left zero, as can happen when crossing a pentagon.
 *
 * @param  origin   origin cell
 * @param  k        k >= 0
 * @param  out      zero-filled array which must be of size maxKringSize(k)
 */
void H3_EXPORT(kRing)(H3Index origin, int k, H3Index* out) {
    H3_EXPORT(kRingDistances)(origin, k, out, NULL);
}

/**
 * Produce cells and their distances from the given origin cell, up to
 * distance k.
 *
 * k-ring 0 is defined as the origin cell, k-ring 1 is defined as k-ring 0 and
 * all neighboring cells, and so on.
 *
 * Output is placed in the provided array in no particular order. Elements of
 * the output array may be left zero, as can happen when crossing a pentagon.
 *
 * @param  origin      origin cell
 * @param  k           k >= 0
 * @param  out         zero-filled array which must be of size maxKringSize(k)
 * @param  distances   NULL or a zero-filled array which must be of size
 *                     maxKringSize(k)
 */
void H3_EXPORT(kRingDistances)(H3Index origin, int k, H3Index* out,
                               int* distances) {
    // Optimistically try the faster hexRange algorithm first
    const bool failed = H3_EXPORT(hexRangeDistances)(origin, k, out, distances);
    if (failed) {
        const int maxIdx = H3_EXPORT(maxKringSize)(k);
        // Fast algo failed, fall back to slower, correct algo
        // and also wipe out array because contents untrustworthy
        memset(out, 0, maxIdx * sizeof(H3Index));

        if (distances == NULL) {
            distances = H3_MEMORY(calloc)(maxIdx, sizeof(int));
            if (!distances) {
                // TODO: Return an error code when this is not void
                return;
            }
            _kRingInternal(origin, k, out, distances, maxIdx, 0);
            H3_MEMORY(free)(distances);
        } else {
            memset(distances, 0, maxIdx * sizeof(int));
            _kRingInternal(origin, k, out, distances, maxIdx, 0);
        }
    }
}

/**
 * Internal helper function called recursively for kRingDistances.
 *
 * Adds the origin cell to the output set (treating it as a hash set)
 * and recurses to its neighbors, if needed.
 *
 * @param  origin      Origin cell
 * @param  k           Maximum distance to move from the origin
 * @param  out         Array treated as a hash set, elements being either
 *                     H3Index or 0.
 * @param  distances   Scratch area, with elements paralleling the out array.
 *                     Elements indicate ijk distance from the origin cell to
 *                     the output cell
 * @param  maxIdx      Size of out and scratch arrays (must be maxKringSize(k))
 * @param  curK        Current distance from the origin
 */
void _kRingInternal(H3Index origin, int k, H3Index* out, int* distances,
                    int maxIdx, int curK) {
    if (origin == 0) return;

    // Put origin in the output array. out is used as a hash set.
    int off = origin % maxIdx;
    while (out[off] != 0 && out[off] != origin) {
        off = (off + 1) % maxIdx;
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
 * @return H3Index of the specified neighbor or H3_NULL if deleted k-subsequence
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
                    return H3_NULL;
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
                    return H3_NULL;  // LCOV_EXCL_LINE
                }
            }
        }

        for (int i = 0; i < newRotations; i++) out = _h3RotatePent60ccw(out);

        // Account for differing orientation of the base cells (this edge
        // might not follow properties of some other edges.)
        if (oldBaseCell != newBaseCell) {
            if (_isBaseCellPolarPentagon(newBaseCell)) {
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
    return H3_EXPORT(hexRangeDistances)(origin, k, out, NULL);
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
            if (origin == 0) {  // LCOV_EXCL_BR_LINE
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
        if (origin == 0) {  // LCOV_EXCL_BR_LINE
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
 * Returns the "hollow" ring of hexagons at exactly grid distance k from
 * the origin hexagon. In particular, k=0 returns just the origin hexagon.
 *
 * A nonzero failure code may be returned in some cases, for example,
 * if a pentagon is encountered.
 * Failure cases may be fixed in future versions.
 *
 * @param origin Origin location.
 * @param k k >= 0
 * @param out Array which must be of size 6 * k (or 1 if k == 0)
 * @return 0 if successful; nonzero otherwise.
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
        if (origin == 0) {  // LCOV_EXCL_BR_LINE
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
            if (origin == 0) {  // LCOV_EXCL_BR_LINE
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
 * The size is the maximum of either the number of points in the geofence or the
 * number of hexagons in the bounding box of the geofence.
 *
 * @param geoPolygon A GeoJSON-like data structure indicating the poly to fill
 * @param res Hexagon resolution (0-15)
 * @return number of hexagons to allocate for
 */
int H3_EXPORT(maxPolyfillSize)(const GeoPolygon* geoPolygon, int res) {
    // Get the bounding box for the GeoJSON-like struct
    BBox bbox;
    const Geofence geofence = geoPolygon->geofence;
    bboxFromGeofence(&geofence, &bbox);
    int numHexagons = bboxHexEstimate(&bbox, res);
    // This algorithm assumes that the number of vertices is usually less than
    // the number of hexagons, but when it's wrong, this will keep it from
    // failing
    int totalVerts = geofence.numVerts;
    for (int i = 0; i < geoPolygon->numHoles; i++) {
        totalVerts += geoPolygon->holes[i].numVerts;
    }
    if (numHexagons < totalVerts) numHexagons = totalVerts;
    // When the polygon is very small, near an icosahedron edge and is an odd
    // resolution, the line tracing needs an extra buffer than the estimator
    // function provides (but beefing that up to cover causes most situations to
    // overallocate memory)
    numHexagons += POLYFILL_BUFFER;
    return numHexagons;
}

/**
 * polyfill takes a given GeoJSON-like data structure and preallocated,
 * zeroed memory, and fills it with the hexagons that are contained by
 * the GeoJSON-like data structure.
 *
 * This implementation traces the GeoJSON geofence(s) in cartesian space with
 * hexagons, tests them and their neighbors to be contained by the geofence(s),
 * and then any newly found hexagons are used to test again until no new
 * hexagons are found.
 *
 * @param geoPolygon The geofence and holes defining the relevant area
 * @param res The Hexagon resolution (0-15)
 * @param out The slab of zeroed memory to write to. Assumed to be big enough.
 */
void H3_EXPORT(polyfill)(const GeoPolygon* geoPolygon, int res, H3Index* out) {
    // TODO: Eliminate this wrapper with the H3 4.0.0 release
    int failure = _polyfillInternal(geoPolygon, res, out);
    // The polyfill algorithm can theoretically fail if the allocated memory is
    // not large enough for the polygon, but this should be impossible given the
    // conservative overestimation of the number of hexagons possible.
    // LCOV_EXCL_START
    if (failure) {
        int numHexagons = H3_EXPORT(maxPolyfillSize)(geoPolygon, res);
        for (int i = 0; i < numHexagons; i++) out[i] = H3_NULL;
    }
    // LCOV_EXCL_STOP
}

/**
 * _getEdgeHexagons takes a given geofence ring (either the main geofence or
 * one of the holes) and traces it with hexagons and updates the search and
 * found memory blocks. This is used for determining the initial hexagon set
 * for the polyfill algorithm to execute on.
 *
 * @param geofence The geofence (or hole) to be traced
 * @param numHexagons The maximum number of hexagons possible for the geofence
 *                    (also the bounds of the search and found arrays)
 * @param res The hexagon resolution (0-15)
 * @param numSearchHexes The number of hexagons found so far to be searched
 * @param search The block of memory containing the hexagons to search from
 * @param found The block of memory containing the hexagons found from the
 * search
 *
 * @return An error code if the hash function cannot insert a found hexagon
 *         into the found array.
 */
int _getEdgeHexagons(const Geofence* geofence, int numHexagons, int res,
                     int* numSearchHexes, H3Index* search, H3Index* found) {
    for (int i = 0; i < geofence->numVerts; i++) {
        GeoCoord origin = geofence->verts[i];
        GeoCoord destination = i == geofence->numVerts - 1
                                   ? geofence->verts[0]
                                   : geofence->verts[i + 1];
        const int numHexesEstimate =
            lineHexEstimate(&origin, &destination, res);
        for (int j = 0; j < numHexesEstimate; j++) {
            GeoCoord interpolate;
            interpolate.lat =
                (origin.lat * (numHexesEstimate - j) / numHexesEstimate) +
                (destination.lat * j / numHexesEstimate);
            interpolate.lon =
                (origin.lon * (numHexesEstimate - j) / numHexesEstimate) +
                (destination.lon * j / numHexesEstimate);
            H3Index pointHex = H3_EXPORT(geoToH3)(&interpolate, res);
            // A simple hash to store the hexagon, or move to another place if
            // needed
            int loc = (int)(pointHex % numHexagons);
            int loopCount = 0;
            while (found[loc] != 0) {
                // If this conditional is reached, the `found` memory block is
                // too small for the given polygon. This should not happen.
                if (loopCount > numHexagons)
                    return HEX_HASH_OVERFLOW;  // LCOV_EXCL_LINE
                if (found[loc] == pointHex)
                    break;  // At least two points of the geofence index to the
                            // same cell
                loc = (loc + 1) % numHexagons;
                loopCount++;
            }
            if (found[loc] == pointHex)
                continue;  // Skip this hex, already exists in the found hash
            // Otherwise, set it in the found hash for now
            found[loc] = pointHex;

            search[*numSearchHexes] = pointHex;
            (*numSearchHexes)++;
        }
    }
    return 0;
}

/**
 * _polyfillInternal traces the provided geoPolygon data structure with hexagons
 * and then iteratively searches through these hexagons and their immediate
 * neighbors to see if they are contained within the polygon or not. Those that
 * are found are added to the out array as well as the found array. Once all
 * hexagons to search are checked, the found hexagons become the new search
 * array and the found array is wiped and the process repeats until no new
 * hexagons can be found.
 *
 * @param geoPolygon The geofence and holes defining the relevant area
 * @param res The Hexagon resolution (0-15)
 * @param out The slab of zeroed memory to write to. Assumed to be big enough.
 *
 * @return An error code if any of the hash operations fails to insert a hexagon
 *         into an array of memory.
 */
int _polyfillInternal(const GeoPolygon* geoPolygon, int res, H3Index* out) {
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
    BBox* bboxes = H3_MEMORY(malloc)((geoPolygon->numHoles + 1) * sizeof(BBox));
    assert(bboxes != NULL);
    bboxesFromGeoPolygon(geoPolygon, bboxes);

    // Get the estimated number of hexagons and allocate some temporary memory
    // for the hexagons
    int numHexagons = H3_EXPORT(maxPolyfillSize)(geoPolygon, res);
    H3Index* search = H3_MEMORY(calloc)(numHexagons, sizeof(H3Index));
    H3Index* found = H3_MEMORY(calloc)(numHexagons, sizeof(H3Index));

    // Some metadata for tracking the state of the search and found memory
    // blocks
    int numSearchHexes = 0;
    int numFoundHexes = 0;

    // 1. Trace the hexagons along the polygon defining the outer geofence and
    // add them to the search hash. The hexagon containing the geofence point
    // may or may not be contained by the geofence (as the hexagon's center
    // point may be outside of the boundary.)
    const Geofence geofence = geoPolygon->geofence;
    int failure = _getEdgeHexagons(&geofence, numHexagons, res, &numSearchHexes,
                                   search, found);
    // If this branch is reached, we have exceeded the maximum number of
    // hexagons possible and need to clean up the allocated memory.
    // LCOV_EXCL_START
    if (failure) {
        H3_MEMORY(free)(search);
        H3_MEMORY(free)(found);
        H3_MEMORY(free)(bboxes);
        return failure;
    }
    // LCOV_EXCL_STOP

    // 2. Iterate over all holes, trace the polygons defining the holes with
    // hexagons and add to only the search hash. We're going to temporarily use
    // the `found` hash to use for dedupe purposes and then re-zero it once
    // we're done here, otherwise we'd have to scan the whole set on each insert
    // to make sure there's no duplicates, which is very inefficient.
    for (int i = 0; i < geoPolygon->numHoles; i++) {
        Geofence* hole = &(geoPolygon->holes[i]);
        failure = _getEdgeHexagons(hole, numHexagons, res, &numSearchHexes,
                                   search, found);
        // If this branch is reached, we have exceeded the maximum number of
        // hexagons possible and need to clean up the allocated memory.
        // LCOV_EXCL_START
        if (failure) {
            H3_MEMORY(free)(search);
            H3_MEMORY(free)(found);
            H3_MEMORY(free)(bboxes);
            return failure;
        }
        // LCOV_EXCL_STOP
    }

    // 3. Re-zero the found hash so it can be used in the main loop below
    for (int i = 0; i < numHexagons; i++) found[i] = 0;

    // 4. Begin main loop. While the search hash is not empty do the following
    while (numSearchHexes > 0) {
        // Iterate through all hexagons in the current search hash, then loop
        // through all neighbors and test Point-in-Poly, if point-in-poly
        // succeeds, add to out and found hashes if not already there.
        int currentSearchNum = 0;
        int i = 0;
        while (currentSearchNum < numSearchHexes) {
            H3Index ring[MAX_ONE_RING_SIZE] = {0};
            H3Index searchHex = search[i];
            H3_EXPORT(kRing)(searchHex, 1, ring);
            for (int j = 0; j < MAX_ONE_RING_SIZE; j++) {
                if (ring[j] == H3_NULL) {
                    continue;  // Skip if this was a pentagon and only had 5
                               // neighbors
                }

                H3Index hex = ring[j];

                // A simple hash to store the hexagon, or move to another place
                // if needed. This MUST be done before the point-in-poly check
                // since that's far more expensive
                int loc = (int)(hex % numHexagons);
                int loopCount = 0;
                while (out[loc] != 0) {
                    // If this branch is reached, we have exceeded the maximum
                    // number of hexagons possible and need to clean up the
                    // allocated memory.
                    // LCOV_EXCL_START
                    if (loopCount > numHexagons) {
                        H3_MEMORY(free)(search);
                        H3_MEMORY(free)(found);
                        H3_MEMORY(free)(bboxes);
                        return -1;
                    }
                    // LCOV_EXCL_STOP
                    if (out[loc] == hex) break;  // Skip duplicates found
                    loc = (loc + 1) % numHexagons;
                    loopCount++;
                }
                if (out[loc] == hex) {
                    continue;  // Skip this hex, already exists in the out hash
                }

                // Check if the hexagon is in the polygon or not
                GeoCoord hexCenter;
                H3_EXPORT(h3ToGeo)(hex, &hexCenter);

                // If not, skip
                if (!pointInsidePolygon(geoPolygon, bboxes, &hexCenter)) {
                    continue;
                }

                // Otherwise set it in the output array
                out[loc] = hex;

                // Set the hexagon in the found hash
                found[numFoundHexes] = hex;
                numFoundHexes++;
            }
            currentSearchNum++;
            i++;
        }

        // Swap the search and found pointers, copy the found hex count to the
        // search hex count, and zero everything related to the found memory.
        H3Index* temp = search;
        search = found;
        found = temp;
        for (int j = 0; j < numSearchHexes; j++) found[j] = 0;
        numSearchHexes = numFoundHexes;
        numFoundHexes = 0;
        // Repeat until no new hexagons are found
    }
    // The out memory structure should be complete, end it here
    H3_MEMORY(free)(bboxes);
    H3_MEMORY(free)(search);
    H3_MEMORY(free)(found);
    return 0;
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
    *out = (LinkedGeoPolygon){0};
    LinkedGeoLoop* loop;
    VertexNode* edge;
    GeoCoord nextVtx;
    // Find the next unused entry point
    while ((edge = firstVertexNode(graph)) != NULL) {
        loop = addNewLinkedLoop(out);
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
 * Polygon outlines will follow GeoJSON MultiPolygon order: Each polygon will
 * have one outer loop, which is first in the list, followed by any holes.
 *
 * It is the responsibility of the caller to call destroyLinkedPolygon on the
 * populated linked geo structure, or the memory for that structure will
 * not be freed.
 *
 * It is expected that all hexagons in the set have the same resolution and
 * that the set contains no duplicates. Behavior is undefined if duplicates
 * or multiple resolutions are present, and the algorithm may produce
 * unexpected or invalid output.
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
    // TODO: The return value, possibly indicating an error, is discarded here -
    // we should use this when we update the API to return a value
    normalizeMultiPolygon(out);
    destroyVertexGraph(&graph);
}
