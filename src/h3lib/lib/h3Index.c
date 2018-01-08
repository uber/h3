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
/** @file h3Index.c
 * @brief   H3Index utility functions
 *          (see h3api.h for the main library entry functions)
 */
#include "h3Index.h"
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "faceijk.h"
#include "h3IndexFat.h"
#include "mathExtensions.h"

/**
 * Returns the H3 resolution of an H3 index.
 * @param h The H3 index.
 * @return The resolution of the H3 index argument.
 */
int H3_EXPORT(h3GetResolution)(H3Index h) { return H3_GET_RESOLUTION(h); }

/**
 * Returns the H3 base cell number of an H3 index.
 * @param h The H3 index.
 * @return The base cell of the H3 index argument.
 */
int H3_EXPORT(h3GetBaseCell)(H3Index h) { return H3_GET_BASE_CELL(h); }

/**
 * Converts a string representation of an H3 index into an H3 index.
 * @param str The string representation of an H3 index.
 * @return The H3 index corresponding to the string argument, or 0 if invalid.
 */
H3Index H3_EXPORT(stringToH3)(const char* str) {
    H3Index h = 0;
    // If failed, h will be unmodified and we should return 0 anyways.
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
 * Converts an H3 index into an H3Fat representation.
 * @param h The H3 index to convert.
 * @param hf The corresponding H3Fat representation.
 */
void h3ToH3Fat(H3Index h, H3IndexFat* hf) {
    initH3IndexFat(hf, H3_GET_RESOLUTION(h));
    hf->mode = H3_GET_MODE(h);
    hf->baseCell = H3_GET_BASE_CELL(h);
    for (int r = 1; r <= MAX_H3_RES; r++)
        hf->index[r - 1] = H3_GET_INDEX_DIGIT(h, r);
}

/**
 * Converts an H3Fat index into an H3 index.
 * @param hf The H3Fat index to convert.
 * @return The H3 index corresponding to the H3Fat index.
 */
H3Index h3FatToH3(const H3IndexFat* hf) {
    H3Index h = H3_INIT;
    H3_SET_MODE(h, hf->mode);
    H3_SET_RESOLUTION(h, hf->res);
    H3_SET_BASE_CELL(h, hf->baseCell);
    for (int r = 1; r < hf->res + 1; r++)
        H3_SET_INDEX_DIGIT(h, r, hf->index[r - 1]);
    return h;
}

/**
 * Returns whether or not an H3 index is valid.
 * @param h The H3 index to validate.
 * @return 1 if the H3 index if valid, and 0 if it is not.
 */
int H3_EXPORT(h3IsValid)(H3Index h) {
    H3IndexFat hf;
    h3ToH3Fat(h, &hf);
    return h3FatIsValid(&hf);
}

/**
 * Initializes an H3 index.
 * @param hp The H3 index to initialize.
 * @param res The H3 resolution to initialize the index to.
 * @param baseCell The H3 base cell to initialize the index to.
 * @param initDigit The H3 digit (0-7) to initialize all of the index digits to.
 */
void setH3Index(H3Index* hp, int res, int baseCell, int initDigit) {
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
 * @return H3Index of the parent, or 0 if you actually asked for a child
 */
H3Index H3_EXPORT(h3ToParent)(H3Index h, int parentRes) {
    int childRes = H3_GET_RESOLUTION(h);
    if (parentRes > childRes) {
        return 0;
    } else if (parentRes == childRes) {
        return h;
    } else if (parentRes < 0 || parentRes > MAX_H3_RES) {
        return 0;
    }
    H3Index parentH = H3_SET_RESOLUTION(h, parentRes);
    for (int i = parentRes + 1; i <= childRes; i++) {
        H3_SET_INDEX_DIGIT(parentH, i, H3_DIGIT_MASK);
    }
    return parentH;
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
    if (parentRes > childRes) {
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
    if (parentRes > childRes) {
        return;
    } else if (parentRes == childRes) {
        *children = h;
        return;
    }
    int bufferSize = H3_EXPORT(maxH3ToChildrenSize)(h, childRes);
    int bufferChildStep = (bufferSize / 7);
    H3IndexFat hFat;
    h3ToH3Fat(h, &hFat);
    int isAPentagon = isPentagon(&hFat);
    for (int i = 0; i < 7; i++) {
        if (!isAPentagon || i != K_AXES_DIGIT) {
            H3_EXPORT(h3ToChildren)(makeDirectChild(h, i), childRes, children);
        }
        children += bufferChildStep;
    }
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
    int res = H3_GET_RESOLUTION(h3Set[0]);
    if (res == 0) {
        // No compaction possible, just copy the set to output
        for (int i = 0; i < numHexes; i++) {
            compactedSet[i] = h3Set[i];
        }
        return 0;
    }
    H3Index* remainingHexes = calloc(numHexes, sizeof(H3Index));
    for (int i = 0; i < numHexes; i++) {
        remainingHexes[i] = h3Set[i];
    }
    H3Index* hashSetArray = calloc(numHexes, sizeof(H3Index));
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
                    if (loopCount > numRemainingHexes) {
                        // LCOV_EXCL_START
                        // This case should not be possible because at most one
                        // index is placed into hashSetArray per
                        // numRemainingHexes.
                        free(remainingHexes);
                        free(hashSetArray);
                        return -1;
                        // LCOV_EXCL_STOP
                    }
                    H3Index tempIndex =
                        hashSetArray[loc] & H3_RESERVED_MASK_NEGATIVE;
                    if (tempIndex == parent) {
                        int count = H3_GET_RESERVED_BITS(hashSetArray[loc]) + 1;
                        if (count > 7) {
                            // Only possible on duplicate input
                            free(remainingHexes);
                            free(hashSetArray);
                            return -2;
                        }
                        H3_SET_RESERVED_BITS(parent, count);
                        hashSetArray[loc] = 0;
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
        H3Index* compactableHexes =
            calloc(maxCompactableCount, sizeof(H3Index));
        for (int i = 0; i < numRemainingHexes; i++) {
            if (hashSetArray[i] == 0) continue;
            int count = H3_GET_RESERVED_BITS(hashSetArray[i]) + 1;
            H3IndexFat tempFat = {0};
            h3ToH3Fat(hashSetArray[i] & H3_RESERVED_MASK_NEGATIVE, &tempFat);
            // Include the deleted direction for pentagons as implicitly "there"
            if (isPentagon(&tempFat)) {
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
            if (currIndex != 0) {
                H3Index parent = H3_EXPORT(h3ToParent)(currIndex, parentRes);
                // Modulus hash the parent into the temp array
                // to determine if this index was included in
                // the compactableHexes array
                int loc = (int)(parent % numRemainingHexes);
                int loopCount = 0;
                int isUncompactable = 1;
                do {
                    if (loopCount > numRemainingHexes) {
                        // LCOV_EXCL_START
                        // This case should not be possible because at most one
                        // index is placed into hashSetArray per input hexagon.
                        free(remainingHexes);
                        free(compactableHexes);
                        free(hashSetArray);
                        return -1;  // Only possible on duplicate input
                        // LCOV_EXCL_STOP
                    }
                    H3Index tempIndex =
                        hashSetArray[loc] & H3_RESERVED_MASK_NEGATIVE;
                    if (tempIndex == parent) {
                        int count = H3_GET_RESERVED_BITS(hashSetArray[loc]) + 1;
                        if (count == 7) {
                            isUncompactable = 0;
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
        for (int i = 0; i < numRemainingHexes; i++) {
            hashSetArray[i] = 0;
        }
        free(remainingHexes);
        compactedSetOffset += uncompactableCount;
        remainingHexes = compactableHexes;
        numRemainingHexes = compactableCount;
    }
    // Final cleanup
    free(remainingHexes);
    free(hashSetArray);
    return 0;
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
 * smaller
 * than the output resolution.
 */
int H3_EXPORT(uncompact)(const H3Index* compactedSet, const int numHexes,
                         H3Index* h3Set, const int maxHexes, const int res) {
    int outOffset = 0;
    for (int i = 0; i < numHexes; i++) {
        if (outOffset >= maxHexes) {
            // We went too far, abort!
            return -1;
        }
        if (compactedSet[i] == 0) continue;
        int currentRes = H3_GET_RESOLUTION(compactedSet[i]);
        if (currentRes > res) {
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
        if (currentRes > res) {
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
    H3IndexFat hFat;
    h3ToH3Fat(h, &hFat);
    return isPentagon(&hFat);
}
