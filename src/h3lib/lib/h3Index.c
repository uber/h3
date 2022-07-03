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
/** @file h3Index.c
 * @brief   H3Index utility functions
 *          (see h3api.h for the main library entry functions)
 */
#include "h3Index.h"

#include <faceijk.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "baseCells.h"
#include "faceijk.h"
#include "iterators.h"
#include "mathExtensions.h"

/**
 * Returns the H3 resolution of an H3 index.
 * @param h The H3 index.
 * @return The resolution of the H3 index argument.
 */
int H3_EXPORT(getResolution)(H3Index h) { return H3_GET_RESOLUTION(h); }

/**
 * Returns the H3 base cell "number" of an H3 cell (hexagon or pentagon).
 *
 * Note: Technically works on H3 edges, but will return base cell of the
 * origin cell.
 *
 * @param h The H3 cell.
 * @return The base cell "number" of the H3 cell argument.
 */
int H3_EXPORT(getBaseCellNumber)(H3Index h) { return H3_GET_BASE_CELL(h); }

/**
 * Converts a string representation of an H3 index into an H3 index.
 * @param str The string representation of an H3 index.
 * @return The H3 index corresponding to the string argument, or H3_NULL if
 * invalid.
 */
H3Error H3_EXPORT(stringToH3)(const char *str, H3Index *out) {
    H3Index h = H3_NULL;
    // If failed, h will be unmodified and we should return H3_NULL anyways.
    int read = sscanf(str, "%" PRIx64, &h);
    if (read != 1) {
        return E_FAILED;
    }
    *out = h;
    return E_SUCCESS;
}

/**
 * Converts an H3 index into a string representation.
 * @param h The H3 index to convert.
 * @param str The string representation of the H3 index.
 * @param sz Size of the buffer `str`
 */
H3Error H3_EXPORT(h3ToString)(H3Index h, char *str, size_t sz) {
    // An unsigned 64 bit integer will be expressed in at most
    // 16 digits plus 1 for the null terminator.
    if (sz < 17) {
        // Buffer is potentially not large enough.
        return E_MEMORY_BOUNDS;
    }
    sprintf(str, "%" PRIx64, h);
    return E_SUCCESS;
}

/**
 * Returns whether or not an H3 index is a valid cell (hexagon or pentagon).
 * @param h The H3 index to validate.
 * @return 1 if the H3 index if valid, and 0 if it is not.
 */
int H3_EXPORT(isValidCell)(H3Index h) {
    if (H3_GET_HIGH_BIT(h) != 0) return 0;

    if (H3_GET_MODE(h) != H3_CELL_MODE) return 0;

    if (H3_GET_RESERVED_BITS(h) != 0) return 0;

    int baseCell = H3_GET_BASE_CELL(h);
    if (baseCell < 0 || baseCell >= NUM_BASE_CELLS) {  // LCOV_EXCL_BR_LINE
        // Base cells less than zero can not be represented in an index
        return 0;
    }

    int res = H3_GET_RESOLUTION(h);
    if (res < 0 || res > MAX_H3_RES) {  // LCOV_EXCL_BR_LINE
        // Resolutions less than zero can not be represented in an index
        return 0;
    }

    bool foundFirstNonZeroDigit = false;
    for (int r = 1; r <= res; r++) {
        Direction digit = H3_GET_INDEX_DIGIT(h, r);

        if (!foundFirstNonZeroDigit && digit != CENTER_DIGIT) {
            foundFirstNonZeroDigit = true;
            if (_isBaseCellPentagon(baseCell) && digit == K_AXES_DIGIT) {
                return 0;
            }
        }

        if (digit < CENTER_DIGIT || digit >= NUM_DIGITS) return 0;
    }

    for (int r = res + 1; r <= MAX_H3_RES; r++) {
        Direction digit = H3_GET_INDEX_DIGIT(h, r);
        if (digit != INVALID_DIGIT) return 0;
    }

    return 1;
}

/**
 * Initializes an H3 index.
 * @param hp The H3 index to initialize.
 * @param res The H3 resolution to initialize the index to.
 * @param baseCell The H3 base cell to initialize the index to.
 * @param initDigit The H3 digit (0-7) to initialize all of the index digits to.
 */
void setH3Index(H3Index *hp, int res, int baseCell, Direction initDigit) {
    H3Index h = H3_INIT;
    H3_SET_MODE(h, H3_CELL_MODE);
    H3_SET_RESOLUTION(h, res);
    H3_SET_BASE_CELL(h, baseCell);
    for (int r = 1; r <= res; r++) H3_SET_INDEX_DIGIT(h, r, initDigit);
    *hp = h;
}

/**
 * cellToParent produces the parent index for a given H3 index
 *
 * @param h H3Index to find parent of
 * @param parentRes The resolution to switch to (parent, grandparent, etc)
 *
 * @return H3Index of the parent, or H3_NULL if you actually asked for a child
 */
H3Error H3_EXPORT(cellToParent)(H3Index h, int parentRes, H3Index *out) {
    int childRes = H3_GET_RESOLUTION(h);
    if (parentRes < 0 || parentRes > MAX_H3_RES) {
        return E_RES_DOMAIN;
    } else if (parentRes > childRes) {
        return E_RES_MISMATCH;
    } else if (parentRes == childRes) {
        *out = h;
        return E_SUCCESS;
    }
    H3Index parentH = H3_SET_RESOLUTION(h, parentRes);
    for (int i = parentRes + 1; i <= childRes; i++) {
        H3_SET_INDEX_DIGIT(parentH, i, H3_DIGIT_MASK);
    }
    *out = parentH;
    return E_SUCCESS;
}

/**
 * Determines whether one resolution is a valid child resolution for a cell.
 * Each resolution is considered a valid child resolution of itself.
 *
 * @param h         h3Index  parent cell
 * @param childRes  int      resolution of the child
 *
 * @return The validity of the child resolution
 */
static bool _hasChildAtRes(H3Index h, int childRes) {
    int parentRes = H3_GET_RESOLUTION(h);
    if (childRes < parentRes || childRes > MAX_H3_RES) {
        return false;
    }
    return true;
}

/**
 * cellToChildrenSize returns the exact number of children for a cell at a
 * given child resolution.
 *
 * @param h         H3Index to find the number of children of
 * @param childRes  The child resolution you're interested in
 *
 * @return int      Exact number of children (handles hexagons and pentagons
 *                  correctly)
 */
H3Error H3_EXPORT(cellToChildrenSize)(H3Index h, int childRes, int64_t *out) {
    if (!_hasChildAtRes(h, childRes)) return E_RES_DOMAIN;

    int n = childRes - H3_GET_RESOLUTION(h);

    if (H3_EXPORT(isPentagon)(h)) {
        *out = 1 + 5 * (_ipow(7, n) - 1) / 6;
    } else {
        *out = _ipow(7, n);
    }
    return E_SUCCESS;
}

/**
 * makeDirectChild takes an index and immediately returns the immediate child
 * index based on the specified cell number. Bit operations only, could generate
 * invalid indexes if not careful (deleted cell under a pentagon).
 *
 * @param h H3Index to find the direct child of
 * @param cellNumber int id of the direct child (0-6)
 *
 * @return The new H3Index for the child
 */
H3Index makeDirectChild(H3Index h, int cellNumber) {
    int childRes = H3_GET_RESOLUTION(h) + 1;
    H3Index childH = H3_SET_RESOLUTION(h, childRes);
    H3_SET_INDEX_DIGIT(childH, childRes, cellNumber);
    return childH;
}

/**
 * cellToChildren takes the given hexagon id and generates all of the children
 * at the specified resolution storing them into the provided memory pointer.
 * It's assumed that cellToChildrenSize was used to determine the allocation.
 *
 * @param h H3Index to find the children of
 * @param childRes int the child level to produce
 * @param children H3Index* the memory to store the resulting addresses in
 */
H3Error H3_EXPORT(cellToChildren)(H3Index h, int childRes, H3Index *children) {
    int64_t i = 0;
    for (IterCellsChildren iter = iterInitParent(h, childRes); iter.h;
         iterStepChild(&iter)) {
        children[i] = iter.h;
        i++;
    }
    return E_SUCCESS;
}

/**
 * Zero out index digits from start to end, inclusive.
 * No-op if start > end.
 */
H3Index _zeroIndexDigits(H3Index h, int start, int end) {
    if (start > end) return h;

    H3Index m = 0;

    m = ~m;
    m <<= H3_PER_DIGIT_OFFSET * (end - start + 1);
    m = ~m;
    m <<= H3_PER_DIGIT_OFFSET * (MAX_H3_RES - end);
    m = ~m;

    return h & m;
}

/**
 * cellToCenterChild produces the center child index for a given H3 index at
 * the specified resolution
 *
 * @param h H3Index to find center child of
 * @param childRes The resolution to switch to
 * @param child H3Index of the center child
 * @return 0 (E_SUCCESS) on success
 */
H3Error H3_EXPORT(cellToCenterChild)(H3Index h, int childRes, H3Index *child) {
    if (!_hasChildAtRes(h, childRes)) return E_RES_DOMAIN;

    h = _zeroIndexDigits(h, H3_GET_RESOLUTION(h) + 1, childRes);
    H3_SET_RESOLUTION(h, childRes);
    *child = h;
    return E_SUCCESS;
}

/**
 * compactCells takes a set of hexagons all at the same resolution and
 * compresses them by pruning full child branches to the parent level. This is
 * also done for all parents recursively to get the minimum number of hex
 * addresses that perfectly cover the defined space.
 * @param h3Set Set of hexagons
 * @param compactedSet The output array of compressed hexagons (preallocated)
 * @param numHexes The size of the input and output arrays (possible that no
 * contiguous regions exist in the set at all and no compression possible)
 * @return an error code on bad input data
 */
// todo: update internal implementation for int64_t
H3Error H3_EXPORT(compactCells)(const H3Index *h3Set, H3Index *compactedSet,
                                const int64_t numHexes) {
    if (numHexes == 0) {
        return E_SUCCESS;
    }
    int res = H3_GET_RESOLUTION(h3Set[0]);
    if (res == 0) {
        // No compaction possible, just copy the set to output
        for (int i = 0; i < numHexes; i++) {
            compactedSet[i] = h3Set[i];
        }
        return E_SUCCESS;
    }
    H3Index *remainingHexes = H3_MEMORY(malloc)(numHexes * sizeof(H3Index));
    if (!remainingHexes) {
        return E_MEMORY_ALLOC;
    }
    memcpy(remainingHexes, h3Set, numHexes * sizeof(H3Index));
    H3Index *hashSetArray = H3_MEMORY(calloc)(numHexes, sizeof(H3Index));
    if (!hashSetArray) {
        H3_MEMORY(free)(remainingHexes);
        return E_MEMORY_ALLOC;
    }
    H3Index *compactedSetOffset = compactedSet;
    int numRemainingHexes = numHexes;
    while (numRemainingHexes) {
        res = H3_GET_RESOLUTION(remainingHexes[0]);
        int parentRes = res - 1;
        // Put the parents of the hexagons into the temp array
        // via a hashing mechanism, and use the reserved bits
        // to track how many times a parent is duplicated
        for (int i = 0; i < numRemainingHexes; i++) {
            H3Index currIndex = remainingHexes[i];
            if (currIndex != 0) {
                // If the reserved bits were set by the caller, the
                // algorithm below may encounter undefined behavior
                // because it expects to have set the reserved bits
                // itself.
                if (H3_GET_RESERVED_BITS(currIndex) != 0) {
                    H3_MEMORY(free)(remainingHexes);
                    H3_MEMORY(free)(hashSetArray);
                    return E_CELL_INVALID;
                }

                H3Index parent;
                H3Error parentError =
                    H3_EXPORT(cellToParent)(currIndex, parentRes, &parent);
                // Should never be reachable as a result of the compact
                // algorithm. Can happen if cellToParent errors e.g.
                // because of incompatible resolutions.
                if (parentError) {
                    H3_MEMORY(free)(remainingHexes);
                    H3_MEMORY(free)(hashSetArray);
                    return parentError;
                }
                // Modulus hash the parent into the temp array
                int loc = (int)(parent % numRemainingHexes);
                int loopCount = 0;
                while (hashSetArray[loc] != 0) {
                    if (loopCount > numRemainingHexes) {  // LCOV_EXCL_BR_LINE
                        // LCOV_EXCL_START
                        // This case should not be possible because at most one
                        // index is placed into hashSetArray per
                        // numRemainingHexes.
                        H3_MEMORY(free)(remainingHexes);
                        H3_MEMORY(free)(hashSetArray);
                        return E_FAILED;
                        // LCOV_EXCL_STOP
                    }
                    H3Index tempIndex =
                        hashSetArray[loc] & H3_RESERVED_MASK_NEGATIVE;
                    if (tempIndex == parent) {
                        int count = H3_GET_RESERVED_BITS(hashSetArray[loc]) + 1;
                        int limitCount = 7;
                        if (H3_EXPORT(isPentagon)(tempIndex &
                                                  H3_RESERVED_MASK_NEGATIVE)) {
                            limitCount--;
                        }
                        // One is added to count for this check to match one
                        // being added to count later in this function when
                        // checking for all children being present.
                        if (count + 1 > limitCount) {
                            // Only possible on duplicate input
                            H3_MEMORY(free)(remainingHexes);
                            H3_MEMORY(free)(hashSetArray);
                            return E_DUPLICATE_INPUT;
                        }
                        H3_SET_RESERVED_BITS(parent, count);
                        hashSetArray[loc] = H3_NULL;
                    } else {
                        loc = (loc + 1) % numRemainingHexes;
                    }
                    loopCount++;
                }
                hashSetArray[loc] = parent;
            }
        }
        // Determine which parent hexagons have a complete set
        // of children and put them in the compactableHexes array
        int compactableCount = 0;
        int maxCompactableCount =
            numRemainingHexes / 6;  // Somehow all pentagons; conservative
        if (maxCompactableCount == 0) {
            memcpy(compactedSetOffset, remainingHexes,
                   numRemainingHexes * sizeof(remainingHexes[0]));
            break;
        }
        H3Index *compactableHexes =
            H3_MEMORY(calloc)(maxCompactableCount, sizeof(H3Index));
        if (!compactableHexes) {
            H3_MEMORY(free)(remainingHexes);
            H3_MEMORY(free)(hashSetArray);
            return E_MEMORY_ALLOC;
        }
        for (int i = 0; i < numRemainingHexes; i++) {
            if (hashSetArray[i] == 0) continue;
            int count = H3_GET_RESERVED_BITS(hashSetArray[i]) + 1;
            // Include the deleted direction for pentagons as implicitly "there"
            if (H3_EXPORT(isPentagon)(hashSetArray[i] &
                                      H3_RESERVED_MASK_NEGATIVE)) {
                // We need this later on, no need to recalculate
                H3_SET_RESERVED_BITS(hashSetArray[i], count);
                // Increment count after setting the reserved bits,
                // since count is already incremented above, so it
                // will be the expected value for a complete hexagon.
                count++;
            }
            if (count == 7) {
                // Bingo! Full set!
                compactableHexes[compactableCount] =
                    hashSetArray[i] & H3_RESERVED_MASK_NEGATIVE;
                compactableCount++;
            }
        }
        // Uncompactable hexes are immediately copied into the
        // output compactedSetOffset
        int uncompactableCount = 0;
        for (int i = 0; i < numRemainingHexes; i++) {
            H3Index currIndex = remainingHexes[i];
            if (currIndex != H3_NULL) {
                H3Index parent;
                H3Error parentError =
                    H3_EXPORT(cellToParent)(currIndex, parentRes, &parent);
                // LCOV_EXCL_START
                // Should never be reachable as a result of the compact
                // algorithm.
                if (parentError) {
                    // TODO: Determine if this is somehow reachable.
                    H3_MEMORY(free)(remainingHexes);
                    H3_MEMORY(free)(hashSetArray);
                    return parentError;
                }
                // LCOV_EXCL_STOP
                // Modulus hash the parent into the temp array
                // to determine if this index was included in
                // the compactableHexes array
                int loc = (int)(parent % numRemainingHexes);
                int loopCount = 0;
                bool isUncompactable = true;
                do {
                    if (loopCount > numRemainingHexes) {  // LCOV_EXCL_BR_LINE
                        // LCOV_EXCL_START
                        // This case should not be possible because at most one
                        // index is placed into hashSetArray per input hexagon.
                        H3_MEMORY(free)(compactableHexes);
                        H3_MEMORY(free)(remainingHexes);
                        H3_MEMORY(free)(hashSetArray);
                        return E_FAILED;
                        // LCOV_EXCL_STOP
                    }
                    H3Index tempIndex =
                        hashSetArray[loc] & H3_RESERVED_MASK_NEGATIVE;
                    if (tempIndex == parent) {
                        int count = H3_GET_RESERVED_BITS(hashSetArray[loc]) + 1;
                        if (count == 7) {
                            isUncompactable = false;
                        }
                        break;
                    } else {
                        loc = (loc + 1) % numRemainingHexes;
                    }
                    loopCount++;
                } while (hashSetArray[loc] != parent);
                if (isUncompactable) {
                    compactedSetOffset[uncompactableCount] = remainingHexes[i];
                    uncompactableCount++;
                }
            }
        }
        // Set up for the next loop
        memset(hashSetArray, 0, numHexes * sizeof(H3Index));
        compactedSetOffset += uncompactableCount;
        memcpy(remainingHexes, compactableHexes,
               compactableCount * sizeof(H3Index));
        numRemainingHexes = compactableCount;
        H3_MEMORY(free)(compactableHexes);
    }
    H3_MEMORY(free)(remainingHexes);
    H3_MEMORY(free)(hashSetArray);
    return E_SUCCESS;
}

/**
 * uncompactCells takes a compressed set of cells and expands back to the
 * original set of cells.
 *
 * Skips elements that are H3_NULL (i.e., 0).
 *
 * @param   compactSet  Set of compacted cells
 * @param   numCompact  The number of cells in the input compacted set
 * @param   outSet      Output array for decompressed cells (preallocated)
 * @param   numOut      The size of the output array to bound check against
 * @param   res         The H3 resolution to decompress to
 * @return              An error code if output array is too small or any cell
 *                      is smaller than the output resolution.
 */
H3Error H3_EXPORT(uncompactCells)(const H3Index *compactedSet,
                                  const int64_t numCompacted, H3Index *outSet,
                                  const int64_t numOut, const int res) {
    int64_t i = 0;

    for (int64_t j = 0; j < numCompacted; j++) {
        if (!_hasChildAtRes(compactedSet[j], res)) return E_RES_MISMATCH;

        for (IterCellsChildren iter = iterInitParent(compactedSet[j], res);
             iter.h; i++, iterStepChild(&iter)) {
            if (i >= numOut) return E_MEMORY_BOUNDS;  // went too far; abort!
            outSet[i] = iter.h;
        }
    }
    return E_SUCCESS;
}

/**
 * uncompactCellsSize takes a compacted set of hexagons and provides
 * the exact size of the uncompacted set of hexagons.
 *
 * @param   compactedSet  Set of hexagons
 * @param   numHexes      The number of hexes in the input set
 * @param   res           The hexagon resolution to decompress to
 * @param   out           The number of hexagons to allocate memory for
 * @returns E_SUCCESS on success, or another value on error
 */
H3Error H3_EXPORT(uncompactCellsSize)(const H3Index *compactedSet,
                                      const int64_t numCompacted, const int res,
                                      int64_t *out) {
    int64_t numOut = 0;
    for (int64_t i = 0; i < numCompacted; i++) {
        if (compactedSet[i] == H3_NULL) continue;

        int64_t childrenSize;
        H3Error childrenError =
            H3_EXPORT(cellToChildrenSize)(compactedSet[i], res, &childrenSize);
        if (childrenError) {
            // The parent res does not contain `res`.
            return E_RES_MISMATCH;
        }
        numOut += childrenSize;
    }
    *out = numOut;
    return E_SUCCESS;
}

/**
 * isResClassIII takes a hexagon ID and determines if it is in a
 * Class III resolution (rotated versus the icosahedron and subject
 * to shape distortion adding extra points on icosahedron edges, making
 * them not true hexagons).
 * @param h The H3Index to check.
 * @return Returns 1 if the hexagon is class III, otherwise 0.
 */
int H3_EXPORT(isResClassIII)(H3Index h) { return H3_GET_RESOLUTION(h) % 2; }

/**
 * isPentagon takes an H3Index and determines if it is actually a pentagon.
 * @param h The H3Index to check.
 * @return Returns 1 if it is a pentagon, otherwise 0.
 */
int H3_EXPORT(isPentagon)(H3Index h) {
    return _isBaseCellPentagon(H3_GET_BASE_CELL(h)) &&
           !_h3LeadingNonZeroDigit(h);
}

/**
 * Returns the highest resolution non-zero digit in an H3Index.
 * @param h The H3Index.
 * @return The highest resolution non-zero digit in the H3Index.
 */
Direction _h3LeadingNonZeroDigit(H3Index h) {
    for (int r = 1; r <= H3_GET_RESOLUTION(h); r++)
        if (H3_GET_INDEX_DIGIT(h, r)) return H3_GET_INDEX_DIGIT(h, r);

    // if we're here it's all 0's
    return CENTER_DIGIT;
}

/**
 * Rotate an H3Index 60 degrees counter-clockwise about a pentagonal center.
 * @param h The H3Index.
 */
H3Index _h3RotatePent60ccw(H3Index h) {
    // rotate in place; skips any leading 1 digits (k-axis)

    int foundFirstNonZeroDigit = 0;
    for (int r = 1, res = H3_GET_RESOLUTION(h); r <= res; r++) {
        // rotate this digit
        H3_SET_INDEX_DIGIT(h, r, _rotate60ccw(H3_GET_INDEX_DIGIT(h, r)));

        // look for the first non-zero digit so we
        // can adjust for deleted k-axes sequence
        // if necessary
        if (!foundFirstNonZeroDigit && H3_GET_INDEX_DIGIT(h, r) != 0) {
            foundFirstNonZeroDigit = 1;

            // adjust for deleted k-axes sequence
            if (_h3LeadingNonZeroDigit(h) == K_AXES_DIGIT)
                h = _h3Rotate60ccw(h);
        }
    }
    return h;
}

/**
 * Rotate an H3Index 60 degrees clockwise about a pentagonal center.
 * @param h The H3Index.
 */
H3Index _h3RotatePent60cw(H3Index h) {
    // rotate in place; skips any leading 1 digits (k-axis)

    int foundFirstNonZeroDigit = 0;
    for (int r = 1, res = H3_GET_RESOLUTION(h); r <= res; r++) {
        // rotate this digit
        H3_SET_INDEX_DIGIT(h, r, _rotate60cw(H3_GET_INDEX_DIGIT(h, r)));

        // look for the first non-zero digit so we
        // can adjust for deleted k-axes sequence
        // if necessary
        if (!foundFirstNonZeroDigit && H3_GET_INDEX_DIGIT(h, r) != 0) {
            foundFirstNonZeroDigit = 1;

            // adjust for deleted k-axes sequence
            if (_h3LeadingNonZeroDigit(h) == K_AXES_DIGIT) h = _h3Rotate60cw(h);
        }
    }
    return h;
}

/**
 * Rotate an H3Index 60 degrees counter-clockwise.
 * @param h The H3Index.
 */
H3Index _h3Rotate60ccw(H3Index h) {
    for (int r = 1, res = H3_GET_RESOLUTION(h); r <= res; r++) {
        Direction oldDigit = H3_GET_INDEX_DIGIT(h, r);
        H3_SET_INDEX_DIGIT(h, r, _rotate60ccw(oldDigit));
    }

    return h;
}

/**
 * Rotate an H3Index 60 degrees clockwise.
 * @param h The H3Index.
 */
H3Index _h3Rotate60cw(H3Index h) {
    for (int r = 1, res = H3_GET_RESOLUTION(h); r <= res; r++) {
        H3_SET_INDEX_DIGIT(h, r, _rotate60cw(H3_GET_INDEX_DIGIT(h, r)));
    }

    return h;
}

/**
 * Convert an FaceIJK address to the corresponding H3Index.
 * @param fijk The FaceIJK address.
 * @param res The cell resolution.
 * @return The encoded H3Index (or H3_NULL on failure).
 */
H3Index _faceIjkToH3(const FaceIJK *fijk, int res) {
    // initialize the index
    H3Index h = H3_INIT;
    H3_SET_MODE(h, H3_CELL_MODE);
    H3_SET_RESOLUTION(h, res);

    // check for res 0/base cell
    if (res == 0) {
        if (fijk->coord.i > MAX_FACE_COORD || fijk->coord.j > MAX_FACE_COORD ||
            fijk->coord.k > MAX_FACE_COORD) {
            // out of range input
            return H3_NULL;
        }

        H3_SET_BASE_CELL(h, _faceIjkToBaseCell(fijk));
        return h;
    }

    // we need to find the correct base cell FaceIJK for this H3 index;
    // start with the passed in face and resolution res ijk coordinates
    // in that face's coordinate system
    FaceIJK fijkBC = *fijk;

    // build the H3Index from finest res up
    // adjust r for the fact that the res 0 base cell offsets the indexing
    // digits
    CoordIJK *ijk = &fijkBC.coord;
    for (int r = res - 1; r >= 0; r--) {
        CoordIJK lastIJK = *ijk;
        CoordIJK lastCenter;
        if (isResolutionClassIII(r + 1)) {
            // rotate ccw
            _upAp7(ijk);
            lastCenter = *ijk;
            _downAp7(&lastCenter);
        } else {
            // rotate cw
            _upAp7r(ijk);
            lastCenter = *ijk;
            _downAp7r(&lastCenter);
        }

        CoordIJK diff;
        _ijkSub(&lastIJK, &lastCenter, &diff);
        _ijkNormalize(&diff);

        H3_SET_INDEX_DIGIT(h, r + 1, _unitIjkToDigit(&diff));
    }

    // fijkBC should now hold the IJK of the base cell in the
    // coordinate system of the current face

    if (fijkBC.coord.i > MAX_FACE_COORD || fijkBC.coord.j > MAX_FACE_COORD ||
        fijkBC.coord.k > MAX_FACE_COORD) {
        // out of range input
        return H3_NULL;
    }

    // lookup the correct base cell
    int baseCell = _faceIjkToBaseCell(&fijkBC);
    H3_SET_BASE_CELL(h, baseCell);

    // rotate if necessary to get canonical base cell orientation
    // for this base cell
    int numRots = _faceIjkToBaseCellCCWrot60(&fijkBC);
    if (_isBaseCellPentagon(baseCell)) {
        // force rotation out of missing k-axes sub-sequence
        if (_h3LeadingNonZeroDigit(h) == K_AXES_DIGIT) {
            // check for a cw/ccw offset face; default is ccw
            if (_baseCellIsCwOffset(baseCell, fijkBC.face)) {
                h = _h3Rotate60cw(h);
            } else {
                h = _h3Rotate60ccw(h);
            }
        }

        for (int i = 0; i < numRots; i++) h = _h3RotatePent60ccw(h);
    } else {
        for (int i = 0; i < numRots; i++) {
            h = _h3Rotate60ccw(h);
        }
    }

    return h;
}

/**
 * Encodes a coordinate on the sphere to the H3 index of the containing cell at
 * the specified resolution.
 *
 * Returns 0 on invalid input.
 *
 * @param g The spherical coordinates to encode.
 * @param res The desired H3 resolution for the encoding.
 * @param out The encoded H3Index.
 * @returns E_SUCCESS (0) on success, another value otherwise
 */
H3Error H3_EXPORT(latLngToCell)(const LatLng *g, int res, H3Index *out) {
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    if (!isfinite(g->lat) || !isfinite(g->lng)) {
        return E_LATLNG_DOMAIN;
    }

    FaceIJK fijk;
    _geoToFaceIjk(g, res, &fijk);
    *out = _faceIjkToH3(&fijk, res);
    if (*out) {
        return E_SUCCESS;
    } else {
        return E_FAILED;
    }
}

/**
 * Convert an H3Index to the FaceIJK address on a specified icosahedral face.
 * @param h The H3Index.
 * @param fijk The FaceIJK address, initialized with the desired face
 *        and normalized base cell coordinates.
 * @return Returns 1 if the possibility of overage exists, otherwise 0.
 */
int _h3ToFaceIjkWithInitializedFijk(H3Index h, FaceIJK *fijk) {
    CoordIJK *ijk = &fijk->coord;
    int res = H3_GET_RESOLUTION(h);

    // center base cell hierarchy is entirely on this face
    int possibleOverage = 1;
    if (!_isBaseCellPentagon(H3_GET_BASE_CELL(h)) &&
        (res == 0 ||
         (fijk->coord.i == 0 && fijk->coord.j == 0 && fijk->coord.k == 0)))
        possibleOverage = 0;

    for (int r = 1; r <= res; r++) {
        if (isResolutionClassIII(r)) {
            // Class III == rotate ccw
            _downAp7(ijk);
        } else {
            // Class II == rotate cw
            _downAp7r(ijk);
        }

        _neighbor(ijk, H3_GET_INDEX_DIGIT(h, r));
    }

    return possibleOverage;
}

/**
 * Convert an H3Index to a FaceIJK address.
 * @param h The H3Index.
 * @param fijk The corresponding FaceIJK address.
 */
H3Error _h3ToFaceIjk(H3Index h, FaceIJK *fijk) {
    int baseCell = H3_GET_BASE_CELL(h);
    if (baseCell < 0 || baseCell >= NUM_BASE_CELLS) {  // LCOV_EXCL_BR_LINE
        // Base cells less than zero can not be represented in an index
        // To prevent reading uninitialized memory, we zero the output.
        fijk->face = 0;
        fijk->coord.i = fijk->coord.j = fijk->coord.k = 0;
        return E_CELL_INVALID;
    }
    // adjust for the pentagonal missing sequence; all of sub-sequence 5 needs
    // to be adjusted (and some of sub-sequence 4 below)
    if (_isBaseCellPentagon(baseCell) && _h3LeadingNonZeroDigit(h) == 5)
        h = _h3Rotate60cw(h);

    // start with the "home" face and ijk+ coordinates for the base cell of c
    *fijk = baseCellData[baseCell].homeFijk;
    if (!_h3ToFaceIjkWithInitializedFijk(h, fijk))
        return E_SUCCESS;  // no overage is possible; h lies on this face

    // if we're here we have the potential for an "overage"; i.e., it is
    // possible that c lies on an adjacent face

    CoordIJK origIJK = fijk->coord;

    // if we're in Class III, drop into the next finer Class II grid
    int res = H3_GET_RESOLUTION(h);
    if (isResolutionClassIII(res)) {
        // Class III
        _downAp7r(&fijk->coord);
        res++;
    }

    // adjust for overage if needed
    // a pentagon base cell with a leading 4 digit requires special handling
    int pentLeading4 =
        (_isBaseCellPentagon(baseCell) && _h3LeadingNonZeroDigit(h) == 4);
    if (_adjustOverageClassII(fijk, res, pentLeading4, 0) != NO_OVERAGE) {
        // if the base cell is a pentagon we have the potential for secondary
        // overages
        if (_isBaseCellPentagon(baseCell)) {
            while (_adjustOverageClassII(fijk, res, 0, 0) != NO_OVERAGE)
                continue;
        }

        if (res != H3_GET_RESOLUTION(h)) _upAp7r(&fijk->coord);
    } else if (res != H3_GET_RESOLUTION(h)) {
        fijk->coord = origIJK;
    }
    return E_SUCCESS;
}

/**
 * Determines the spherical coordinates of the center point of an H3 index.
 *
 * @param h3 The H3 index.
 * @param g The spherical coordinates of the H3 cell center.
 */
H3Error H3_EXPORT(cellToLatLng)(H3Index h3, LatLng *g) {
    FaceIJK fijk;
    H3Error e = _h3ToFaceIjk(h3, &fijk);
    if (e) {
        return e;
    }
    _faceIjkToGeo(&fijk, H3_GET_RESOLUTION(h3), g);
    return E_SUCCESS;
}

/**
 * Determines the cell boundary in spherical coordinates for an H3 index.
 *
 * @param h3 The H3 index.
 * @param cb The boundary of the H3 cell in spherical coordinates.
 */
H3Error H3_EXPORT(cellToBoundary)(H3Index h3, CellBoundary *cb) {
    FaceIJK fijk;
    H3Error e = _h3ToFaceIjk(h3, &fijk);
    if (e) {
        return e;
    }
    if (H3_EXPORT(isPentagon)(h3)) {
        _faceIjkPentToCellBoundary(&fijk, H3_GET_RESOLUTION(h3), 0,
                                   NUM_PENT_VERTS, cb);
    } else {
        _faceIjkToCellBoundary(&fijk, H3_GET_RESOLUTION(h3), 0, NUM_HEX_VERTS,
                               cb);
    }
    return E_SUCCESS;
}

/**
 * Returns the max number of possible icosahedron faces an H3 index
 * may intersect.
 *
 * @return int count of faces
 */
H3Error H3_EXPORT(maxFaceCount)(H3Index h3, int *out) {
    // a pentagon always intersects 5 faces, a hexagon never intersects more
    // than 2 (but may only intersect 1)
    *out = H3_EXPORT(isPentagon)(h3) ? 5 : 2;
    return E_SUCCESS;
}

/**
 * Find all icosahedron faces intersected by a given H3 index, represented
 * as integers from 0-19. The array is sparse; since 0 is a valid value,
 * invalid array values are represented as -1. It is the responsibility of
 * the caller to filter out invalid values.
 *
 * @param h3 The H3 index
 * @param out Output array. Must be of size maxFaceCount(h3).
 */
H3Error H3_EXPORT(getIcosahedronFaces)(H3Index h3, int *out) {
    int res = H3_GET_RESOLUTION(h3);
    int isPent = H3_EXPORT(isPentagon)(h3);

    // We can't use the vertex-based approach here for class II pentagons,
    // because all their vertices are on the icosahedron edges. Their
    // direct child pentagons cross the same faces, so use those instead.
    if (isPent && !isResolutionClassIII(res)) {
        // Note that this would not work for res 15, but this is only run on
        // Class II pentagons, it should never be invoked for a res 15 index.
        H3Index childPentagon = makeDirectChild(h3, 0);
        return H3_EXPORT(getIcosahedronFaces)(childPentagon, out);
    }

    // convert to FaceIJK
    FaceIJK fijk;
    H3Error err = _h3ToFaceIjk(h3, &fijk);
    if (err) {
        return err;
    }

    // Get all vertices as FaceIJK addresses. For simplicity, always
    // initialize the array with 6 verts, ignoring the last one for pentagons
    FaceIJK fijkVerts[NUM_HEX_VERTS];
    int vertexCount;

    if (isPent) {
        vertexCount = NUM_PENT_VERTS;
        _faceIjkPentToVerts(&fijk, &res, fijkVerts);
    } else {
        vertexCount = NUM_HEX_VERTS;
        _faceIjkToVerts(&fijk, &res, fijkVerts);
    }

    // We may not use all of the slots in the output array,
    // so fill with invalid values to indicate unused slots
    int faceCount;
    H3Error maxFaceCountError = H3_EXPORT(maxFaceCount)(h3, &faceCount);
    if (maxFaceCountError != E_SUCCESS) {
        return maxFaceCountError;
    }
    for (int i = 0; i < faceCount; i++) {
        out[i] = INVALID_FACE;
    }

    // add each vertex face, using the output array as a hash set
    for (int i = 0; i < vertexCount; i++) {
        FaceIJK *vert = &fijkVerts[i];

        // Adjust overage, determining whether this vertex is
        // on another face
        if (isPent) {
            _adjustPentVertOverage(vert, res);
        } else {
            _adjustOverageClassII(vert, res, 0, 1);
        }

        // Save the face to the output array
        int face = vert->face;
        int pos = 0;
        // Find the first empty output position, or the first position
        // matching the current face
        while (out[pos] != INVALID_FACE && out[pos] != face) {
            pos++;
            if (pos >= faceCount) {
                // Mismatch between the heuristic used in maxFaceCount and
                // calculation here - indicates an invalid index.
                return E_FAILED;
            }
        }
        out[pos] = face;
    }
    return E_SUCCESS;
}

/**
 * pentagonCount returns the number of pentagons (same at any resolution)
 *
 * @return int count of pentagon indexes
 */
int H3_EXPORT(pentagonCount)() { return NUM_PENTAGONS; }

/**
 * Generates all pentagons at the specified resolution
 *
 * @param res The resolution to produce pentagons at.
 * @param out Output array. Must be of size pentagonCount().
 */
H3Error H3_EXPORT(getPentagons)(int res, H3Index *out) {
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    int i = 0;
    for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
        if (_isBaseCellPentagon(bc)) {
            H3Index pentagon;
            setH3Index(&pentagon, res, bc, 0);
            out[i++] = pentagon;
        }
    }
    return E_SUCCESS;
}

/**
 * Returns whether or not a resolution is a Class III grid. Note that odd
 * resolutions are Class III and even resolutions are Class II.
 * @param res The H3 resolution.
 * @return 1 if the resolution is a Class III grid, and 0 if the resolution is
 *         a Class II grid.
 */
int isResolutionClassIII(int res) { return res % 2; }
