/*
 * Copyright 2016-2021 Uber Technologies, Inc.
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
#include "h3Assert.h"
#include "h3Index.h"
#include "h3api.h"
#include "latLng.h"
#include "linkedGeo.h"
#include "polygon.h"
#include "vertexGraph.h"

/*
 * Return codes from gridDiskUnsafe and related functions.
 */

#define MAX_ONE_RING_SIZE 7
#define POLYGON_TO_CELLS_BUFFER 12

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
 * k value which will encompass all cells at resolution 15.
 * This is the largest possible k in the H3 grid system.
 */
static const int K_ALL_CELLS_AT_RES_15 = 13780510;

/**
 * Maximum number of cells that result from the gridDisk algorithm with the
 * given k. Formula source and proof: https://oeis.org/A003215
 *
 * @param   k   k value, k >= 0.
 * @param out   size in indexes
 */
H3Error H3_EXPORT(maxGridDiskSize)(int k, int64_t *out) {
    if (k < 0) {
        return E_DOMAIN;
    }
    if (k >= K_ALL_CELLS_AT_RES_15) {
        // If a k value of this value or above is provided, this function will
        // estimate more cells than exist in the H3 grid at the finest
        // resolution. This is a problem since the function does signed integer
        // arithmetic on `k`, which could overflow. To prevent that, instead
        // substitute the maximum number of cells in the grid, as it should not
        // be possible for the gridDisk functions to exceed that. Note this is
        // not resolution specific. So, when resolution < 15, this function may
        // still estimate a size larger than the number of cells in the grid.
        return H3_EXPORT(getNumCells)(MAX_H3_RES, out);
    }
    *out = 3 * (int64_t)k * ((int64_t)k + 1) + 1;
    return E_SUCCESS;
}

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
 * @param  out      zero-filled array which must be of size maxGridDiskSize(k)
 */
H3Error H3_EXPORT(gridDisk)(H3Index origin, int k, H3Index *out) {
    return H3_EXPORT(gridDiskDistances)(origin, k, out, NULL);
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
 * @param  out         zero-filled array which must be of size
 * maxGridDiskSize(k)
 * @param  distances   NULL or a zero-filled array which must be of size
 *                     maxGridDiskSize(k)
 */
H3Error H3_EXPORT(gridDiskDistances)(H3Index origin, int k, H3Index *out,
                                     int *distances) {
    // Optimistically try the faster gridDiskUnsafe algorithm first
    const H3Error failed =
        H3_EXPORT(gridDiskDistancesUnsafe)(origin, k, out, distances);
    if (failed) {
        int64_t maxIdx;
        H3Error err = H3_EXPORT(maxGridDiskSize)(k, &maxIdx);
        if (err) {
            return err;
        }
        // Fast algo failed, fall back to slower, correct algo
        // and also wipe out array because contents untrustworthy
        memset(out, 0, maxIdx * sizeof(H3Index));

        if (distances == NULL) {
            distances = H3_MEMORY(calloc)(maxIdx, sizeof(int));
            if (!distances) {
                return E_MEMORY_ALLOC;
            }
            H3Error result = _gridDiskDistancesInternal(origin, k, out,
                                                        distances, maxIdx, 0);
            H3_MEMORY(free)(distances);
            return result;
        } else {
            memset(distances, 0, maxIdx * sizeof(int));
            return _gridDiskDistancesInternal(origin, k, out, distances, maxIdx,
                                              0);
        }
    } else {
        return E_SUCCESS;
    }
}

/**
 * Internal algorithm for the safe but slow version of gridDiskDistances
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
 * @param  maxIdx      Size of out and scratch arrays (must be
 * maxGridDiskSize(k))
 * @param  curK        Current distance from the origin
 */
H3Error _gridDiskDistancesInternal(H3Index origin, int k, H3Index *out,
                                   int *distances, int64_t maxIdx, int curK) {
    // Put origin in the output array. out is used as a hash set.
    int64_t off = origin % maxIdx;
    while (out[off] != 0 && out[off] != origin) {
        off = (off + 1) % maxIdx;
    }

    // We either got a free slot in the hash set or hit a duplicate
    // We might need to process the duplicate anyways because we got
    // here on a longer path before.
    if (out[off] == origin && distances[off] <= curK) return E_SUCCESS;

    out[off] = origin;
    distances[off] = curK;

    // Base case: reached an index k away from the origin.
    if (curK >= k) return E_SUCCESS;

    // Recurse to all neighbors in no particular order.
    for (int i = 0; i < 6; i++) {
        int rotations = 0;
        H3Index nextNeighbor;
        H3Error neighborResult = h3NeighborRotations(origin, DIRECTIONS[i],
                                                     &rotations, &nextNeighbor);
        if (neighborResult != E_PENTAGON) {
            // E_PENTAGON is an expected case when trying to traverse off of
            // pentagons.
            if (neighborResult != E_SUCCESS) {
                return neighborResult;
            }
            neighborResult = _gridDiskDistancesInternal(
                nextNeighbor, k, out, distances, maxIdx, curK + 1);
            if (neighborResult) {
                return neighborResult;
            }
        }
    }
    return E_SUCCESS;
}

/**
 * Safe but slow version of gridDiskDistances (also called by it when needed).
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
 */
H3Error H3_EXPORT(gridDiskDistancesSafe)(H3Index origin, int k, H3Index *out,
                                         int *distances) {
    int64_t maxIdx;
    H3Error err = H3_EXPORT(maxGridDiskSize)(k, &maxIdx);
    if (err) {
        return err;
    }
    return _gridDiskDistancesInternal(origin, k, out, distances, maxIdx, 0);
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
 * @param out H3Index of the specified neighbor if succesful
 * @return E_SUCCESS on success
 */
H3Error h3NeighborRotations(H3Index origin, Direction dir, int *rotations,
                            H3Index *out) {
    H3Index current = origin;

    if (dir < CENTER_DIGIT || dir >= INVALID_DIGIT) {
        return E_FAILED;
    }
    // Ensure that rotations is modulo'd by 6 before any possible addition,
    // to protect against signed integer overflow.
    *rotations = *rotations % 6;
    for (int i = 0; i < *rotations; i++) {
        dir = _rotate60ccw(dir);
    }

    int newRotations = 0;
    int oldBaseCell = H3_GET_BASE_CELL(current);
    if (NEVER(oldBaseCell < 0) || oldBaseCell >= NUM_BASE_CELLS) {
        // Base cells less than zero can not be represented in an index
        return E_CELL_INVALID;
    }
    Direction oldLeadingDigit = _h3LeadingNonZeroDigit(current);

    // Adjust the indexing digits and, if needed, the base cell.
    int r = H3_GET_RESOLUTION(current) - 1;
    while (true) {
        if (r == -1) {
            H3_SET_BASE_CELL(current, baseCellNeighbors[oldBaseCell][dir]);
            newRotations = baseCellNeighbor60CCWRots[oldBaseCell][dir];

            if (H3_GET_BASE_CELL(current) == INVALID_BASE_CELL) {
                // Adjust for the deleted k vertex at the base cell level.
                // This edge actually borders a different neighbor.
                H3_SET_BASE_CELL(current,
                                 baseCellNeighbors[oldBaseCell][IK_AXES_DIGIT]);
                newRotations =
                    baseCellNeighbor60CCWRots[oldBaseCell][IK_AXES_DIGIT];

                // perform the adjustment for the k-subsequence we're skipping
                // over.
                current = _h3Rotate60ccw(current);
                *rotations = *rotations + 1;
            }

            break;
        } else {
            Direction oldDigit = H3_GET_INDEX_DIGIT(current, r + 1);
            Direction nextDir;
            if (oldDigit == INVALID_DIGIT) {
                // Only possible on invalid input
                return E_CELL_INVALID;
            } else if (isResolutionClassIII(r + 1)) {
                H3_SET_INDEX_DIGIT(current, r + 1, NEW_DIGIT_II[oldDigit][dir]);
                nextDir = NEW_ADJUSTMENT_II[oldDigit][dir];
            } else {
                H3_SET_INDEX_DIGIT(current, r + 1,
                                   NEW_DIGIT_III[oldDigit][dir]);
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

    int newBaseCell = H3_GET_BASE_CELL(current);
    if (_isBaseCellPentagon(newBaseCell)) {
        int alreadyAdjustedKSubsequence = 0;

        // force rotation out of missing k-axes sub-sequence
        if (_h3LeadingNonZeroDigit(current) == K_AXES_DIGIT) {
            if (oldBaseCell != newBaseCell) {
                // in this case, we traversed into the deleted
                // k subsequence of a pentagon base cell.
                // We need to rotate out of that case depending
                // on how we got here.
                // check for a cw/ccw offset face; default is ccw

                if (ALWAYS(_baseCellIsCwOffset(
                        newBaseCell,
                        baseCellData[oldBaseCell].homeFijk.face))) {
                    current = _h3Rotate60cw(current);
                } else {
                    // See cwOffsetPent in testGridDisk.c for why this is
                    // unreachable.
                    current = _h3Rotate60ccw(current);
                }
                alreadyAdjustedKSubsequence = 1;
            } else {
                // In this case, we traversed into the deleted
                // k subsequence from within the same pentagon
                // base cell.
                if (oldLeadingDigit == CENTER_DIGIT) {
                    // Undefined: the k direction is deleted from here
                    return E_PENTAGON;
                } else if (oldLeadingDigit == JK_AXES_DIGIT) {
                    // Rotate out of the deleted k subsequence
                    // We also need an additional change to the direction we're
                    // moving in
                    current = _h3Rotate60ccw(current);
                    *rotations = *rotations + 1;
                } else if (oldLeadingDigit == IK_AXES_DIGIT) {
                    // Rotate out of the deleted k subsequence
                    // We also need an additional change to the direction we're
                    // moving in
                    current = _h3Rotate60cw(current);
                    *rotations = *rotations + 5;
                } else {
                    // TODO: Should never occur, but is reachable by fuzzer
                    return E_FAILED;
                }
            }
        }

        for (int i = 0; i < newRotations; i++)
            current = _h3RotatePent60ccw(current);

        // Account for differing orientation of the base cells (this edge
        // might not follow properties of some other edges.)
        if (oldBaseCell != newBaseCell) {
            if (_isBaseCellPolarPentagon(newBaseCell)) {
                // 'polar' base cells behave differently because they have all
                // i neighbors.
                if (oldBaseCell != 118 && oldBaseCell != 8 &&
                    _h3LeadingNonZeroDigit(current) != JK_AXES_DIGIT) {
                    *rotations = *rotations + 1;
                }
            } else if (_h3LeadingNonZeroDigit(current) == IK_AXES_DIGIT &&
                       !alreadyAdjustedKSubsequence) {
                // account for distortion introduced to the 5 neighbor by the
                // deleted k subsequence.
                *rotations = *rotations + 1;
            }
        }
    } else {
        for (int i = 0; i < newRotations; i++)
            current = _h3Rotate60ccw(current);
    }

    *rotations = (*rotations + newRotations) % 6;
    *out = current;

    return E_SUCCESS;
}

/**
 * Get the direction from the origin to a given neighbor. This is effectively
 * the reverse operation for h3NeighborRotations. Returns INVALID_DIGIT if the
 * cells are not neighbors.
 *
 * TODO: This is currently a brute-force algorithm, but as it's O(6) that's
 * probably acceptable.
 */
Direction directionForNeighbor(H3Index origin, H3Index destination) {
    bool isPent = H3_EXPORT(isPentagon)(origin);
    // Checks each neighbor, in order, to determine which direction the
    // destination neighbor is located. Skips CENTER_DIGIT since that
    // would be the origin; skips deleted K direction for pentagons.
    for (Direction direction = isPent ? J_AXES_DIGIT : K_AXES_DIGIT;
         direction < NUM_DIGITS; direction++) {
        H3Index neighbor;
        int rotations = 0;
        H3Error neighborError =
            h3NeighborRotations(origin, direction, &rotations, &neighbor);
        if (!neighborError && neighbor == destination) {
            return direction;
        }
    }
    return INVALID_DIGIT;
}

/**
 * gridDiskUnsafe produces indexes within k distance of the origin index.
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
 * @param out Array which must be of size maxGridDiskSize(k).
 * @return 0 if no pentagon or pentagonal distortion area was encountered.
 */
H3Error H3_EXPORT(gridDiskUnsafe)(H3Index origin, int k, H3Index *out) {
    return H3_EXPORT(gridDiskDistancesUnsafe)(origin, k, out, NULL);
}

/**
 * gridDiskDistancesUnsafe produces indexes within k distance of the origin
 * index. Output behavior is undefined when one of the indexes returned by this
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
 * @param out Array which must be of size maxGridDiskSize(k).
 * @param distances Null or array which must be of size maxGridDiskSize(k).
 * @return 0 if no pentagon or pentagonal distortion area was encountered.
 */
H3Error H3_EXPORT(gridDiskDistancesUnsafe)(H3Index origin, int k, H3Index *out,
                                           int *distances) {
    // Return codes:
    // 1 Pentagon was encountered
    // 2 Pentagon distortion (deleted k subsequence) was encountered
    // Pentagon being encountered is not itself a problem; really the deleted
    // k-subsequence is the problem, but for compatibility reasons we fail on
    // the pentagon.
    if (k < 0) {
        return E_DOMAIN;
    }

    // k must be >= 0, so origin is always needed
    int idx = 0;
    out[idx] = origin;
    if (distances) {
        distances[idx] = 0;
    }
    idx++;

    if (H3_EXPORT(isPentagon)(origin)) {
        // Pentagon was encountered; bail out as user doesn't want this.
        return E_PENTAGON;
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
            H3Error neighborResult = h3NeighborRotations(
                origin, NEXT_RING_DIRECTION, &rotations, &origin);
            if (neighborResult) {
                // Should not be possible because `origin` would have to be a
                // pentagon
                // TODO: Reachable via fuzzer
                return neighborResult;
            }

            if (H3_EXPORT(isPentagon)(origin)) {
                // Pentagon was encountered; bail out as user doesn't want this.
                return E_PENTAGON;
            }
        }

        H3Error neighborResult = h3NeighborRotations(
            origin, DIRECTIONS[direction], &rotations, &origin);
        if (neighborResult) {
            return neighborResult;
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

        if (H3_EXPORT(isPentagon)(origin)) {
            // Pentagon was encountered; bail out as user doesn't want this.
            return E_PENTAGON;
        }
    }
    return E_SUCCESS;
}

/**
 * gridDisksUnsafe takes an array of input hex IDs and a max k-ring and returns
 * an array of hexagon IDs sorted first by the original hex IDs and then by the
 * k-ring (0 to max), with no guaranteed sorting within each k-ring group.
 *
 * @param h3Set A pointer to an array of H3Indexes
 * @param length The total number of H3Indexes in h3Set
 * @param k The number of rings to generate
 * @param out A pointer to the output memory to dump the new set of H3Indexes to
 *            The memory block should be equal to maxGridDiskSize(k) * length
 * @return 0 if no pentagon is encountered. Cannot trust output otherwise
 */
H3Error H3_EXPORT(gridDisksUnsafe)(H3Index *h3Set, int length, int k,
                                   H3Index *out) {
    H3Index *segment;
    int64_t segmentSize;
    H3Error err = H3_EXPORT(maxGridDiskSize)(k, &segmentSize);
    if (err) {
        return err;
    }
    for (int i = 0; i < length; i++) {
        // Determine the appropriate segment of the output array to operate on
        segment = out + i * segmentSize;
        H3Error failed = H3_EXPORT(gridDiskUnsafe)(h3Set[i], k, segment);
        if (failed) return failed;
    }
    return E_SUCCESS;
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
H3Error H3_EXPORT(gridRingUnsafe)(H3Index origin, int k, H3Index *out) {
    // Short-circuit on 'identity' ring
    if (k == 0) {
        out[0] = origin;
        return E_SUCCESS;
    }
    int idx = 0;
    // Number of 60 degree ccw rotations to perform on the direction (based on
    // which faces have been crossed.)
    int rotations = 0;
    // Scratch structure for checking for pentagons
    if (H3_EXPORT(isPentagon)(origin)) {
        // Pentagon was encountered; bail out as user doesn't want this.
        return E_PENTAGON;
    }

    for (int ring = 0; ring < k; ring++) {
        H3Error neighborResult = h3NeighborRotations(
            origin, NEXT_RING_DIRECTION, &rotations, &origin);
        if (neighborResult) {
            // Should not be possible because `origin` would have to be a
            // pentagon
            // TODO: Reachable via fuzzer
            return neighborResult;
        }

        if (H3_EXPORT(isPentagon)(origin)) {
            return E_PENTAGON;
        }
    }

    H3Index lastIndex = origin;

    out[idx] = origin;
    idx++;

    for (int direction = 0; direction < 6; direction++) {
        for (int pos = 0; pos < k; pos++) {
            H3Error neighborResult = h3NeighborRotations(
                origin, DIRECTIONS[direction], &rotations, &origin);
            if (neighborResult) {
                // Should not be possible because `origin` would have to be a
                // pentagon
                // TODO: Reachable via fuzzer
                return neighborResult;
            }

            // Skip the very last index, it was already added. We do
            // however need to traverse to it because of the pentagonal
            // distortion check, below.
            if (pos != k - 1 || direction != 5) {
                out[idx] = origin;
                idx++;

                if (H3_EXPORT(isPentagon)(origin)) {
                    return E_PENTAGON;
                }
            }
        }
    }

    // Check that this matches the expected lastIndex, if it doesn't,
    // it indicates pentagonal distortion occurred and we should report
    // failure.
    if (lastIndex != origin) {
        return E_PENTAGON;
    } else {
        return E_SUCCESS;
    }
}

/**
 * maxPolygonToCellsSize returns the number of cells to allocate space for
 * when performing a polygonToCells on the given GeoJSON-like data structure.
 *
 * The size is the maximum of either the number of points in the geoloop or the
 * number of cells in the bounding box of the geoloop.
 *
 * @param geoPolygon A GeoJSON-like data structure indicating the poly to fill
 * @param res Hexagon resolution (0-15)
 * @param out number of cells to allocate for
 * @return 0 (E_SUCCESS) on success.
 */
H3Error H3_EXPORT(maxPolygonToCellsSize)(const GeoPolygon *geoPolygon, int res,
                                         uint32_t flags, int64_t *out) {
    if (flags != 0) {
        return E_OPTION_INVALID;
    }
    // Get the bounding box for the GeoJSON-like struct
    BBox bbox;
    const GeoLoop geoloop = geoPolygon->geoloop;
    bboxFromGeoLoop(&geoloop, &bbox);
    int64_t numHexagons;
    H3Error estimateErr = bboxHexEstimate(&bbox, res, &numHexagons);
    if (estimateErr) {
        return estimateErr;
    }
    // This algorithm assumes that the number of vertices is usually less than
    // the number of hexagons, but when it's wrong, this will keep it from
    // failing
    int totalVerts = geoloop.numVerts;
    for (int i = 0; i < geoPolygon->numHoles; i++) {
        totalVerts += geoPolygon->holes[i].numVerts;
    }
    if (numHexagons < totalVerts) numHexagons = totalVerts;
    // When the polygon is very small, near an icosahedron edge and is an odd
    // resolution, the line tracing needs an extra buffer than the estimator
    // function provides (but beefing that up to cover causes most situations to
    // overallocate memory)
    numHexagons += POLYGON_TO_CELLS_BUFFER;
    *out = numHexagons;
    return E_SUCCESS;
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
H3Error h3SetToVertexGraph(const H3Index *h3Set, const int numHexes,
                           VertexGraph *graph) {
    CellBoundary vertices;
    LatLng *fromVtx;
    LatLng *toVtx;
    VertexNode *edge;
    if (numHexes < 1) {
        // We still need to init the graph, or calls to destroyVertexGraph will
        // fail
        initVertexGraph(graph, 0, 0);
        return E_SUCCESS;
    }
    int res = H3_GET_RESOLUTION(h3Set[0]);
    const int minBuckets = 6;
    // TODO: Better way to calculate/guess?
    int numBuckets = numHexes > minBuckets ? numHexes : minBuckets;
    initVertexGraph(graph, numBuckets, res);
    // Iterate through every hexagon
    for (int i = 0; i < numHexes; i++) {
        H3Error boundaryErr = H3_EXPORT(cellToBoundary)(h3Set[i], &vertices);
        if (boundaryErr) {
            // Destroy vertex graph as caller will not know to do so.
            destroyVertexGraph(graph);
            return boundaryErr;
        }
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
    return E_SUCCESS;
}

/**
 * Internal: Create a LinkedGeoPolygon from a vertex graph. It is the
 * responsibility of the caller to call destroyLinkedMultiPolygon on the
 * populated linked geo structure, or the memory for that structure will not be
 * freed.
 * @private
 * @param graph Input graph
 * @param out   Output polygon
 */
void _vertexGraphToLinkedGeo(VertexGraph *graph, LinkedGeoPolygon *out) {
    *out = (LinkedGeoPolygon){0};
    LinkedGeoLoop *loop;
    VertexNode *edge;
    LatLng nextVtx;
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
 * It is the responsibility of the caller to call destroyLinkedMultiPolygon on
 * the populated linked geo structure, or the memory for that structure will not
 * be freed.
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
H3Error H3_EXPORT(cellsToLinkedMultiPolygon)(const H3Index *h3Set,
                                             const int numHexes,
                                             LinkedGeoPolygon *out) {
    VertexGraph graph;
    H3Error err = h3SetToVertexGraph(h3Set, numHexes, &graph);
    if (err) {
        return err;
    }
    _vertexGraphToLinkedGeo(&graph, out);
    destroyVertexGraph(&graph);
    H3Error normalizeResult = normalizeMultiPolygon(out);
    if (normalizeResult) {
        H3_EXPORT(destroyLinkedMultiPolygon)(out);
    }
    return normalizeResult;
}
