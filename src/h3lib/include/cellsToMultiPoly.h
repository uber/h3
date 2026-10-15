/*
 * Copyright 2026 Uber Technologies, Inc.
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
/** @file cellsToMultiPoly.h
 * @brief   Internal helper functions for cellsToMultiPolygon
 *
 * Functions exposed here mostly so we can test them separately in
 * testCellsToMultiPolyInternal.c for complete branch coverage.
 */

#ifndef CELLS_TO_MULTI_POLY_H
#define CELLS_TO_MULTI_POLY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "h3api.h"
#include "mathExtensions.h"

typedef struct Arc {
    H3Index id;

    bool isVisited;
    bool isRemoved;

    // For doubly-arced list of edges in loop.
    struct Arc *next;
    struct Arc *prev;

    // The arc for the reversed edge, or NULL if it is not in the set
    struct Arc *twin;

    // For union-find datastructure
    // https://en.wikipedia.org/wiki/Disjoint-set_data_structure
    struct Arc *parent;
    int64_t rank;
} Arc;

typedef struct {
    int64_t numArcs;
    Arc *arcs;
} ArcSet;

/*
Sort key for pairing an arc with its twin. An arc and its twin join the same
two cells, so they share the key (lo, hi): the smaller and larger of the
edge's origin and destination cells.
*/
typedef struct {
    H3Index lo;
    H3Index hi;
    Arc *arc;
} ArcKey;

// Digit width for sortArcKeys. 11 bits keeps the counts array in L1 cache.
#define ARC_KEY_RADIX_BITS 11
#define ARC_KEY_RADIX_SIZE (1 << ARC_KEY_RADIX_BITS)
// Below this many keys, sortArcKeys uses qsort, which is faster there
#define ARC_KEY_RADIX_MIN_KEYS 256

typedef struct {
    H3Index root;
    double area;

    GeoLoop loop;
} SortableLoop;

typedef struct {
    int64_t numLoops;
    SortableLoop *sloops;
} SortableLoopSet;

typedef struct {
    double outerArea;
    GeoPolygon poly;
} SortablePoly;

/**
 * Check for potential integer overflow in cellsToMultiPolygon allocations.
 *
 * Validates that the two largest allocations, which are live at the same time,
 * won't overflow together:
 * 1. arcs array: numArcs * sizeof(Arc) where numArcs ~= 6 * numCells
 * 2. keys array: 2 * numArcs * sizeof(ArcKey), for the keys and sort scratch
 *
 * @param numCells Number of cells to convert
 * @return E_SUCCESS if allocations are safe, E_MEMORY_BOUNDS if overflow would
 * occur
 */
static inline H3Error checkCellsToMultiPolyOverflow(int64_t numCells) {
    // Compute the bytes per cell across both allocations
    uint64_t arcsPerCell = 6 * sizeof(Arc);
    uint64_t keysPerCell = 2 * 6 * sizeof(ArcKey);
    uint64_t bytesPerCell = arcsPerCell + keysPerCell;

    // Check if bytesPerCell * numCells would overflow size_t, which is what
    // is used for allocations. Use SIZE_MAX since size_t may be 32 bits.
    if (numCells > 0 && numCells > SIZE_MAX / bytesPerCell) {
        return E_MEMORY_BOUNDS;
    }

    return E_SUCCESS;
}

static inline int cmp_SortableLoop(const void *pa, const void *pb) {
    const SortableLoop *a = (const SortableLoop *)pa;
    const SortableLoop *b = (const SortableLoop *)pb;

    // first, sort on connected component
    if (a->root < b->root) return -1;
    if (a->root > b->root) return 1;

    // second, sort on area of loops
    if (a->area < b->area) return -1;
    if (a->area > b->area) return 1;

    return 0;  // same root and equal area
}

static inline int cmp_ArcKey(const void *pa, const void *pb) {
    const ArcKey *a = (const ArcKey *)pa;
    const ArcKey *b = (const ArcKey *)pb;

    if (a->lo < b->lo) return -1;
    if (a->lo > b->lo) return 1;

    if (a->hi < b->hi) return -1;
    if (a->hi > b->hi) return 1;

    return 0;  // same pair of cells
}

/*
Find the lowest and highest set bits of `x`.
For x == 0, `low` is 64 and `high` is -1.
*/
static inline void bitRange(uint64_t x, int *low, int *high) {
    *low = 0;
    while (*low < 64 && !((x >> *low) & 1)) {
        (*low)++;
    }
    *high = 63;
    while (*high >= 0 && !((x >> *high) & 1)) {
        (*high)--;
    }
}

static inline uint64_t arcKeyDigit(const ArcKey *key, int word, int shift) {
    uint64_t x = word ? key->lo : key->hi;
    return (x >> shift) & (ARC_KEY_RADIX_SIZE - 1);
}

/*
Sort keys by (lo, hi), in the order of cmp_ArcKey, with a least significant
digit radix sort.

Bits that are equal in every key don't affect the order, so each word is
sorted only over the range of bits that varies, ARC_KEY_RADIX_BITS per pass.
For cells of one resolution, the mode, resolution, and unused digit bits never
vary, and neither do the leading digits of a compact region.

Small inputs use qsort instead, since each radix pass has a fixed cost.

@param keys Keys to sort
@param scratch Buffer with room for `n` keys
@param n Number of keys
@param counts Buffer with room for ARC_KEY_RADIX_SIZE counts
@return Whichever of `keys` or `scratch` holds the sorted keys
*/
static inline ArcKey *sortArcKeys(ArcKey *keys, ArcKey *scratch, int64_t n,
                                  int64_t *counts) {
    if (n < ARC_KEY_RADIX_MIN_KEYS) {
        qsort(keys, n, sizeof(ArcKey), cmp_ArcKey);
        return keys;
    }

    uint64_t loOr = 0, loAnd = UINT64_MAX;
    uint64_t hiOr = 0, hiAnd = UINT64_MAX;
    for (int64_t i = 0; i < n; i++) {
        loOr |= keys[i].lo;
        loAnd &= keys[i].lo;
        hiOr |= keys[i].hi;
        hiAnd &= keys[i].hi;
    }

    // Sort on the less significant word first
    uint64_t varying[2] = {hiOr ^ hiAnd, loOr ^ loAnd};
    for (int word = 0; word < 2; word++) {
        int low, high;
        bitRange(varying[word], &low, &high);

        for (int shift = low; shift <= high; shift += ARC_KEY_RADIX_BITS) {
            memset(counts, 0, ARC_KEY_RADIX_SIZE * sizeof(int64_t));
            for (int64_t i = 0; i < n; i++) {
                counts[arcKeyDigit(&keys[i], word, shift)]++;
            }

            // Turn counts into starting offsets
            int64_t total = 0;
            for (int64_t d = 0; d < ARC_KEY_RADIX_SIZE; d++) {
                int64_t count = counts[d];
                counts[d] = total;
                total += count;
            }

            for (int64_t i = 0; i < n; i++) {
                scratch[counts[arcKeyDigit(&keys[i], word, shift)]++] = keys[i];
            }

            ArcKey *tmp = keys;
            keys = scratch;
            scratch = tmp;
        }
    }

    return keys;
}

static inline int cmp_SortablePoly(const void *pa, const void *pb) {
    const SortablePoly *a = (const SortablePoly *)pa;
    const SortablePoly *b = (const SortablePoly *)pb;

    // Sort by area of outer loop, in descending order
    if (a->outerArea > b->outerArea) return -1;
    if (a->outerArea < b->outerArea) return 1;

    return 0;  // equal area
}

/*
Compare H3Index values, interpreting them as uint64s.

Note that, usually, we only use this ordering when we know that the
cells in the set are all the same resolution.
*/
static inline int cmp_uint64(const void *a, const void *b) {
    H3Index ha = *(const H3Index *)a;
    H3Index hb = *(const H3Index *)b;
    if (ha < hb) return -1;
    if (ha > hb) return +1;
    return 0;
}

/*
Helper function to free memory allocated for an ArcSet.
Safe to call with partially initialized ArcSet (NULL pointers are skipped).
*/
static inline void destroyArcSet(ArcSet *arcset) {
    if (arcset->arcs) {
        H3_MEMORY(free)(arcset->arcs);
        arcset->arcs = NULL;
    }
}

/*
Helper function to free the SortableLoopSet array without freeing vertex data.
Used when vertex ownership has been transferred to the output GeoMultiPolygon.
*/
static inline void destroySortableLoopSetShallow(SortableLoopSet *loopset) {
    if (loopset->sloops) {
        H3_MEMORY(free)(loopset->sloops);
        loopset->sloops = NULL;
    }
}

/*
Helper function to free memory allocated for a SortableLoopSet.
Frees all vertex arrays in the loops, then the loops array itself.
*/
static inline void destroySortableLoopSet(SortableLoopSet *loopset) {
    if (loopset->sloops) {
        for (int64_t i = 0; i < loopset->numLoops; i++) {
            if (loopset->sloops[i].loop.verts) {
                H3_MEMORY(free)(loopset->sloops[i].loop.verts);
            }
        }
    }
    destroySortableLoopSetShallow(loopset);
}

/*
Helper function to free memory allocated for an array of SortablePoly.
Frees the holes arrays in each polygon, then the polygon array itself.
numPolys specifies how many polygons to clean up.
*/
static inline void destroySortablePolys(SortablePoly *spolys,
                                        int64_t numPolys) {
    if (spolys) {
        for (int64_t i = 0; i < numPolys; i++) {
            if (spolys[i].poly.holes) {
                H3_MEMORY(free)(spolys[i].poly.holes);
            }
        }
        H3_MEMORY(free)(spolys);
    }
}

/*
Helper function to free outer loop vertices from an array of SortablePoly.
Frees the verts arrays from each polygon's geoloop, then the polygon array.
Used during partial cleanup when constructing the polygon array fails.
numPolys specifies how many polygons to clean up.
*/
static inline void destroySortablePolyVerts(SortablePoly *spolys,
                                            int64_t numPolys) {
    if (spolys) {
        for (int64_t i = 0; i < numPolys; i++) {
            if (spolys[i].poly.geoloop.verts) {
                H3_MEMORY(free)(spolys[i].poly.geoloop.verts);
            }
        }
        H3_MEMORY(free)(spolys);
    }
}

/** @brief Create a GeoMultiPolygon from a set of cells
 *
 * NOTE: This definition is tentative as we work to finish the implementation.
 * TODO: This will be moved to h3api.h.in when it is ready to release.
 * */
DECLSPEC H3Error H3_EXPORT(cellsToMultiPolygon)(const H3Index *cells,
                                                const int64_t numCells,
                                                GeoMultiPolygon *out);

/** @brief Free all memory created for a GeoMultiPolygon */
DECLSPEC void H3_EXPORT(destroyGeoMultiPolygon)(GeoMultiPolygon *mpoly);

#endif
