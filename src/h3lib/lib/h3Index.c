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
#include "mathExtensions.h"

/**
 * Returns the H3 resolution of an H3 index.
 * @param h The H3 index.
 * @return The resolution of the H3 index argument.
 */
int H3_EXPORT(h3GetResolution)(H3Index h) { return H3_GET_RESOLUTION(h); }

/**
 * Returns the H3 base cell "number" of an H3 cell (hexagon or pentagon).
 *
 * Note: Technically works on H3 edges, but will return base cell of the
 * origin cell.
 *
 * @param h The H3 cell.
 * @return The base cell "number" of the H3 cell argument.
 */
int H3_EXPORT(h3GetBaseCell)(H3Index h) { return H3_GET_BASE_CELL(h); }

/**
 * Converts a string representation of an H3 index into an H3 index.
 * @param str The string representation of an H3 index.
 * @return The H3 index corresponding to the string argument, or H3_NULL if
 * invalid.
 */
H3Index H3_EXPORT(stringToH3)(const char* str) {
    H3Index h = H3_NULL;
    // If failed, h will be unmodified and we should return H3_NULL anyways.
    sscanf(str, "%" PRIx64, &h);
    return h;
}

/**
 * Converts an H3 index into a string representation.
 * @param h The H3 index to convert.
 * @param str The string representation of the H3 index.
 * @param sz Size of the buffer `str`
 */
void H3_EXPORT(h3ToString)(H3Index h, char* str, size_t sz) {
    // An unsigned 64 bit integer will be expressed in at most
    // 16 digits plus 1 for the null terminator.
    if (sz < 17) {
        // Buffer is potentially not large enough.
        return;
    }
    sprintf(str, "%" PRIx64, h);
}

/**
 * Returns whether or not an H3 index is a valid cell (hexagon or pentagon).
 * @param h The H3 index to validate.
 * @return 1 if the H3 index if valid, and 0 if it is not.
 */
int H3_EXPORT(h3IsValid)(H3Index h) {
    if (H3_GET_HIGH_BIT(h) != 0) return 0;

    if (H3_GET_MODE(h) != H3_HEXAGON_MODE) return 0;

    if (H3_GET_RESERVED_BITS(h) != 0) return 0;

    int baseCell = H3_GET_BASE_CELL(h);
    if (baseCell < 0 || baseCell >= NUM_BASE_CELLS) return 0;

    int res = H3_GET_RESOLUTION(h);
    if (res < 0 || res > MAX_H3_RES) return 0;

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
void setH3Index(H3Index* hp, int res, int baseCell, Direction initDigit) {
    H3Index h = H3_INIT;
    H3_SET_MODE(h, H3_HEXAGON_MODE);
    H3_SET_RESOLUTION(h, res);
    H3_SET_BASE_CELL(h, baseCell);
    for (int r = 1; r <= res; r++) H3_SET_INDEX_DIGIT(h, r, initDigit);
    *hp = h;
}

/**
 * h3ToParent produces the parent index for a given H3 index
 *
 * @param h H3Index to find parent of
 * @param parentRes The resolution to switch to (parent, grandparent, etc)
 *
 * @return H3Index of the parent, or H3_NULL if you actually asked for a child
 */
H3Index H3_EXPORT(h3ToParent)(H3Index h, int parentRes) {
    int childRes = H3_GET_RESOLUTION(h);
    if (parentRes > childRes) {
        return H3_NULL;
    } else if (parentRes == childRes) {
        return h;
    } else if (parentRes < 0 || parentRes > MAX_H3_RES) {
        return H3_NULL;
    }
    H3Index parentH = H3_SET_RESOLUTION(h, parentRes);
    for (int i = parentRes + 1; i <= childRes; i++) {
        H3_SET_INDEX_DIGIT(parentH, i, H3_DIGIT_MASK);
    }
    return parentH;
}

/**
 * Determines whether one resolution is a valid child resolution of another.
 * Each resolution is considered a valid child resolution of itself.
 *
 * @param parentRes int resolution of the parent
 * @param childRes int resolution of the child
 *
 * @return The validity of the child resolution
 */
static bool _isValidChildRes(int parentRes, int childRes) {
    if (childRes < parentRes || childRes > MAX_H3_RES) {
        return false;
    }
    return true;
}

/**
 * maxH3ToChildrenSize returns the maximum number of children possible for a
 * given child level.
 *
 * @param h H3Index to find the number of children of
 * @param childRes The resolution of the child level you're interested in
 *
 * @return int count of maximum number of children (equal for hexagons, less for
 * pentagons
 */
int H3_EXPORT(maxH3ToChildrenSize)(H3Index h, int childRes) {
    int parentRes = H3_GET_RESOLUTION(h);
    if (!_isValidChildRes(parentRes, childRes)) {
        return 0;
    }
    return _ipow(7, (childRes - parentRes));
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
 * h3ToChildren takes the given hexagon id and generates all of the children
 * at the specified resolution storing them into the provided memory pointer.
 * It's assumed that maxH3ToChildrenSize was used to determine the allocation.
 *
 * @param h H3Index to find the children of
 * @param childRes int the child level to produce
 * @param children H3Index* the memory to store the resulting addresses in
 */
void H3_EXPORT(h3ToChildren)(H3Index h, int childRes, H3Index* children) {
    int parentRes = H3_GET_RESOLUTION(h);
    if (!_isValidChildRes(parentRes, childRes)) {
        return;
    } else if (parentRes == childRes) {
        *children = h;
        return;
    }
    int bufferSize = H3_EXPORT(maxH3ToChildrenSize)(h, childRes);
    int bufferChildStep = (bufferSize / 7);
    int isAPentagon = H3_EXPORT(h3IsPentagon)(h);
    for (int i = 0; i < 7; i++) {
        if (isAPentagon && i == K_AXES_DIGIT) {
            H3Index* nextChild = children + bufferChildStep;
            while (children < nextChild) {
                *children = H3_NULL;
                children++;
            }
        } else {
            H3_EXPORT(h3ToChildren)(makeDirectChild(h, i), childRes, children);
            children += bufferChildStep;
        }
    }
}

/**
 * h3ToCenterChild produces the center child index for a given H3 index at
 * the specified resolution
 *
 * @param h H3Index to find center child of
 * @param childRes The resolution to switch to
 *
 * @return H3Index of the center child, or H3_NULL if you actually asked for a
 * parent
 */
H3Index H3_EXPORT(h3ToCenterChild)(H3Index h, int childRes) {
    int parentRes = H3_GET_RESOLUTION(h);
    if (!_isValidChildRes(parentRes, childRes)) {
        return H3_NULL;
    } else if (childRes == parentRes) {
        return h;
    }
    H3Index child = H3_SET_RESOLUTION(h, childRes);
    for (int i = parentRes + 1; i <= childRes; i++) {
        H3_SET_INDEX_DIGIT(child, i, 0);
    }
    return child;
}

/**
 * compact takes a set of hexagons all at the same resolution and compresses
 * them by pruning full child branches to the parent level. This is also done
 * for all parents recursively to get the minimum number of hex addresses that
 * perfectly cover the defined space.
 * @param h3Set Set of hexagons
 * @param compactedSet The output array of compressed hexagons (preallocated)
 * @param numHexes The size of the input and output arrays (possible that no
 * contiguous regions exist in the set at all and no compression possible)
 * @return an error code on bad input data
 */
int H3_EXPORT(compact)(const H3Index* h3Set, H3Index* compactedSet,
                       const int numHexes) {
    if (numHexes == 0) {
        return COMPACT_SUCCESS;
    }
    int res = H3_GET_RESOLUTION(h3Set[0]);
    if (res == 0) {
        // No compaction possible, just copy the set to output
        for (int i = 0; i < numHexes; i++) {
            compactedSet[i] = h3Set[i];
        }
        return COMPACT_SUCCESS;
    }
    H3Index* remainingHexes = H3_MEMORY(malloc)(numHexes * sizeof(H3Index));
    if (!remainingHexes) {
        return COMPACT_ALLOC_FAILED;
    }
    memcpy(remainingHexes, h3Set, numHexes * sizeof(H3Index));
    H3Index* hashSetArray = H3_MEMORY(calloc)(numHexes, sizeof(H3Index));
    if (!hashSetArray) {
        H3_MEMORY(free)(remainingHexes);
        return COMPACT_ALLOC_FAILED;
    }
    H3Index* compactedSetOffset = compactedSet;
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
                H3Index parent = H3_EXPORT(h3ToParent)(currIndex, parentRes);
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
                        return COMPACT_LOOP_EXCEEDED;
                        // LCOV_EXCL_STOP
                    }
                    H3Index tempIndex =
                        hashSetArray[loc] & H3_RESERVED_MASK_NEGATIVE;
                    if (tempIndex == parent) {
                        int count = H3_GET_RESERVED_BITS(hashSetArray[loc]) + 1;
                        int limitCount = 7;
                        if (H3_EXPORT(h3IsPentagon)(
                                tempIndex & H3_RESERVED_MASK_NEGATIVE)) {
                            limitCount--;
                        }
                        // One is added to count for this check to match one
                        // being added to count later in this function when
                        // checking for all children being present.
                        if (count + 1 > limitCount) {
                            // Only possible on duplicate input
                            H3_MEMORY(free)(remainingHexes);
                            H3_MEMORY(free)(hashSetArray);
                            return COMPACT_DUPLICATE;
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
        H3Index* compactableHexes =
            H3_MEMORY(calloc)(maxCompactableCount, sizeof(H3Index));
        if (!compactableHexes) {
            H3_MEMORY(free)(remainingHexes);
            H3_MEMORY(free)(hashSetArray);
            return COMPACT_ALLOC_FAILED;
        }
        for (int i = 0; i < numRemainingHexes; i++) {
            if (hashSetArray[i] == 0) continue;
            int count = H3_GET_RESERVED_BITS(hashSetArray[i]) + 1;
            // Include the deleted direction for pentagons as implicitly "there"
            if (H3_EXPORT(h3IsPentagon)(hashSetArray[i] &
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
                H3Index parent = H3_EXPORT(h3ToParent)(currIndex, parentRes);
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
                        return COMPACT_LOOP_EXCEEDED;
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
    return COMPACT_SUCCESS;
}

/**
 * uncompact takes a compressed set of hexagons and expands back to the
 * original set of hexagons.
 * @param compactedSet Set of hexagons
 * @param numHexes The number of hexes in the input set
 * @param h3Set Output array of decompressed hexagons (preallocated)
 * @param maxHexes The size of the output array to bound check against
 * @param res The hexagon resolution to decompress to
 * @return An error code if output array is too small or any hexagon is
 * smaller than the output resolution.
 */
int H3_EXPORT(uncompact)(const H3Index* compactedSet, const int numHexes,
                         H3Index* h3Set, const int maxHexes, const int res) {
    int outOffset = 0;
    for (int i = 0; i < numHexes; i++) {
        if (compactedSet[i] == 0) continue;
        if (outOffset >= maxHexes) {
            // We went too far, abort!
            return -1;
        }
        int currentRes = H3_GET_RESOLUTION(compactedSet[i]);
        if (!_isValidChildRes(currentRes, res)) {
            // Nonsensical. Abort.
            return -2;
        }
        if (currentRes == res) {
            // Just copy and move along
            h3Set[outOffset] = compactedSet[i];
            outOffset++;
        } else {
            // Bigger hexagon to reduce in size
            int numHexesToGen =
                H3_EXPORT(maxH3ToChildrenSize)(compactedSet[i], res);
            if (outOffset + numHexesToGen > maxHexes) {
                // We're about to go too far, abort!
                return -1;
            }
            H3_EXPORT(h3ToChildren)(compactedSet[i], res, h3Set + outOffset);
            outOffset += numHexesToGen;
        }
    }
    return 0;
}

/**
 * maxUncompactSize takes a compacted set of hexagons are provides an
 * upper-bound estimate of the size of the uncompacted set of hexagons.
 * @param compactedSet Set of hexagons
 * @param numHexes The number of hexes in the input set
 * @param res The hexagon resolution to decompress to
 * @return The number of hexagons to allocate memory for, or a negative
 * number if an error occurs.
 */
int H3_EXPORT(maxUncompactSize)(const H3Index* compactedSet, const int numHexes,
                                const int res) {
    int maxNumHexagons = 0;
    for (int i = 0; i < numHexes; i++) {
        if (compactedSet[i] == 0) continue;
        int currentRes = H3_GET_RESOLUTION(compactedSet[i]);
        if (!_isValidChildRes(currentRes, res)) {
            // Nonsensical. Abort.
            return -1;
        }
        if (currentRes == res) {
            maxNumHexagons++;
        } else {
            // Bigger hexagon to reduce in size
            int numHexesToGen =
                H3_EXPORT(maxH3ToChildrenSize)(compactedSet[i], res);
            maxNumHexagons += numHexesToGen;
        }
    }
    return maxNumHexagons;
}

/**
 * h3IsResClassIII takes a hexagon ID and determines if it is in a
 * Class III resolution (rotated versus the icosahedron and subject
 * to shape distortion adding extra points on icosahedron edges, making
 * them not true hexagons).
 * @param h The H3Index to check.
 * @return Returns 1 if the hexagon is class III, otherwise 0.
 */
int H3_EXPORT(h3IsResClassIII)(H3Index h) { return H3_GET_RESOLUTION(h) % 2; }

/**
 * h3IsPentagon takes an H3Index and determines if it is actually a
 * pentagon.
 * @param h The H3Index to check.
 * @return Returns 1 if it is a pentagon, otherwise 0.
 */
int H3_EXPORT(h3IsPentagon)(H3Index h) {
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
H3Index _faceIjkToH3(const FaceIJK* fijk, int res) {
    // initialize the index
    H3Index h = H3_INIT;
    H3_SET_MODE(h, H3_HEXAGON_MODE);
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
    CoordIJK* ijk = &fijkBC.coord;
    for (int r = res - 1; r >= 0; r--) {
        CoordIJK lastIJK = *ijk;
        CoordIJK lastCenter;
        if (isResClassIII(r + 1)) {
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
 * @return The encoded H3Index (or H3_NULL on failure).
 */
H3Index H3_EXPORT(geoToH3)(const GeoCoord* g, int res) {
    if (res < 0 || res > MAX_H3_RES) {
        return H3_NULL;
    }
    if (!isfinite(g->lat) || !isfinite(g->lon)) {
        return H3_NULL;
    }

    FaceIJK fijk;
    _geoToFaceIjk(g, res, &fijk);
    return _faceIjkToH3(&fijk, res);
}

/**
 * Convert an H3Index to the FaceIJK address on a specified icosahedral face.
 * @param h The H3Index.
 * @param fijk The FaceIJK address, initialized with the desired face
 *        and normalized base cell coordinates.
 * @return Returns 1 if the possibility of overage exists, otherwise 0.
 */
int _h3ToFaceIjkWithInitializedFijk(H3Index h, FaceIJK* fijk) {
    CoordIJK* ijk = &fijk->coord;
    int res = H3_GET_RESOLUTION(h);

    // center base cell hierarchy is entirely on this face
    int possibleOverage = 1;
    if (!_isBaseCellPentagon(H3_GET_BASE_CELL(h)) &&
        (res == 0 ||
         (fijk->coord.i == 0 && fijk->coord.j == 0 && fijk->coord.k == 0)))
        possibleOverage = 0;

    for (int r = 1; r <= res; r++) {
        if (isResClassIII(r)) {
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
void _h3ToFaceIjk(H3Index h, FaceIJK* fijk) {
    int baseCell = H3_GET_BASE_CELL(h);
    // adjust for the pentagonal missing sequence; all of sub-sequence 5 needs
    // to be adjusted (and some of sub-sequence 4 below)
    if (_isBaseCellPentagon(baseCell) && _h3LeadingNonZeroDigit(h) == 5)
        h = _h3Rotate60cw(h);

    // start with the "home" face and ijk+ coordinates for the base cell of c
    *fijk = baseCellData[baseCell].homeFijk;
    if (!_h3ToFaceIjkWithInitializedFijk(h, fijk))
        return;  // no overage is possible; h lies on this face

    // if we're here we have the potential for an "overage"; i.e., it is
    // possible that c lies on an adjacent face

    CoordIJK origIJK = fijk->coord;

    // if we're in Class III, drop into the next finer Class II grid
    int res = H3_GET_RESOLUTION(h);
    if (isResClassIII(res)) {
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
}

/**
 * Determines the spherical coordinates of the center point of an H3 index.
 *
 * @param h3 The H3 index.
 * @param g The spherical coordinates of the H3 cell center.
 */
void H3_EXPORT(h3ToGeo)(H3Index h3, GeoCoord* g) {
    FaceIJK fijk;
    _h3ToFaceIjk(h3, &fijk);
    _faceIjkToGeo(&fijk, H3_GET_RESOLUTION(h3), g);
}

/**
 * Determines the cell boundary in spherical coordinates for an H3 index.
 *
 * @param h3 The H3 index.
 * @param gb The boundary of the H3 cell in spherical coordinates.
 */
void H3_EXPORT(h3ToGeoBoundary)(H3Index h3, GeoBoundary* gb) {
    FaceIJK fijk;
    _h3ToFaceIjk(h3, &fijk);
    if (H3_EXPORT(h3IsPentagon)(h3)) {
        _faceIjkPentToGeoBoundary(&fijk, H3_GET_RESOLUTION(h3), 0,
                                  NUM_PENT_VERTS, gb);
    } else {
        _faceIjkToGeoBoundary(&fijk, H3_GET_RESOLUTION(h3), 0, NUM_HEX_VERTS,
                              gb);
    }
}

/**
 * Returns the max number of possible icosahedron faces an H3 index
 * may intersect.
 *
 * @return int count of faces
 */
int H3_EXPORT(maxFaceCount)(H3Index h3) {
    // a pentagon always intersects 5 faces, a hexagon never intersects more
    // than 2 (but may only intersect 1)
    return H3_EXPORT(h3IsPentagon)(h3) ? 5 : 2;
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
void H3_EXPORT(h3GetFaces)(H3Index h3, int* out) {
    int res = H3_GET_RESOLUTION(h3);
    int isPentagon = H3_EXPORT(h3IsPentagon)(h3);

    // We can't use the vertex-based approach here for class II pentagons,
    // because all their vertices are on the icosahedron edges. Their
    // direct child pentagons cross the same faces, so use those instead.
    if (isPentagon && !isResClassIII(res)) {
        // Note that this would not work for res 15, but this is only run on
        // Class II pentagons, it should never be invoked for a res 15 index.
        H3Index childPentagon = makeDirectChild(h3, 0);
        H3_EXPORT(h3GetFaces)(childPentagon, out);
        return;
    }

    // convert to FaceIJK
    FaceIJK fijk;
    _h3ToFaceIjk(h3, &fijk);

    // Get all vertices as FaceIJK addresses. For simplicity, always
    // initialize the array with 6 verts, ignoring the last one for pentagons
    FaceIJK fijkVerts[NUM_HEX_VERTS];
    int vertexCount;

    if (isPentagon) {
        vertexCount = NUM_PENT_VERTS;
        _faceIjkPentToVerts(&fijk, &res, fijkVerts);
    } else {
        vertexCount = NUM_HEX_VERTS;
        _faceIjkToVerts(&fijk, &res, fijkVerts);
    }

    // We may not use all of the slots in the output array,
    // so fill with invalid values to indicate unused slots
    int faceCount = H3_EXPORT(maxFaceCount)(h3);
    for (int i = 0; i < faceCount; i++) {
        out[i] = INVALID_FACE;
    }

    // add each vertex face, using the output array as a hash set
    for (int i = 0; i < vertexCount; i++) {
        FaceIJK* vert = &fijkVerts[i];

        // Adjust overage, determining whether this vertex is
        // on another face
        if (isPentagon) {
            _adjustPentVertOverage(vert, res);
        } else {
            _adjustOverageClassII(vert, res, 0, 1);
        }

        // Save the face to the output array
        int face = vert->face;
        int pos = 0;
        // Find the first empty output position, or the first position
        // matching the current face
        while (out[pos] != INVALID_FACE && out[pos] != face) pos++;
        out[pos] = face;
    }
}

/**
 * pentagonIndexCount returns the number of pentagons (same at any resolution)
 *
 * @return int count of pentagon indexes
 */
int H3_EXPORT(pentagonIndexCount)() { return NUM_PENTAGONS; }

/**
 * Generates all pentagons at the specified resolution
 *
 * @param res The resolution to produce pentagons at.
 * @param out Output array. Must be of size pentagonIndexCount().
 */
void H3_EXPORT(getPentagonIndexes)(int res, H3Index* out) {
    int i = 0;
    for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
        if (_isBaseCellPentagon(bc)) {
            H3Index pentagon;
            setH3Index(&pentagon, res, bc, 0);
            out[i++] = pentagon;
        }
    }
}

/**
 * Returns whether or not a resolution is a Class III grid. Note that odd
 * resolutions are Class III and even resolutions are Class II.
 * @param res The H3 resolution.
 * @return 1 if the resolution is a Class III grid, and 0 if the resolution is
 *         a Class II grid.
 */
int isResClassIII(int res) { return res % 2; }
