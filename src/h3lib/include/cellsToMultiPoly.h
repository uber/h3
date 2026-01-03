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
#include <stdint.h>

#include "alloc.h"
#include "h3api.h"
#include "mathExtensions.h"

// After rough search, 10 seems to minimize compute time for large sets
#define HASH_TABLE_MULTIPLIER 10

typedef struct Arc {
    H3Index id;

    bool isVisited;
    bool isRemoved;

    // For doubly-arced list of edges in loop.
    struct Arc *next;
    struct Arc *prev;

    // For union-find datastructure
    // https://en.wikipedia.org/wiki/Disjoint-set_data_structure
    struct Arc *parent;
    int64_t rank;
} Arc;

typedef struct {
    int64_t numArcs;
    Arc *arcs;

    // hash buckets for fast edge/arc lookup
    int64_t numBuckets;
    Arc **buckets;
} ArcSet;

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
 * Validates that the two largest allocations won't overflow:
 * 1. arcs array: numArcs * sizeof(Arc) where numArcs ~= 6 * numCells
 * 2. buckets array: numBuckets * sizeof(Arc *)
 *                   where numBuckets = numArcs * HASH_TABLE_MULTIPLIER
 *
 * @param numCells Number of cells to convert
 * @return E_SUCCESS if allocations are safe, E_MEMORY_BOUNDS if overflow would
 * occur
 */
static inline H3Error checkCellsToMultiPolyOverflow(int64_t numCells,
                                                    int64_t hashMultiplier) {
    // Compute the maximum bytes per cell across both allocations
    int64_t arcsPerCell = 6 * sizeof(Arc);
    int64_t bucketsPerCell = 6 * hashMultiplier * sizeof(Arc *);
    int64_t maxBytesPerCell = MAX(arcsPerCell, bucketsPerCell);

    // Check if maxBytesPerCell * numCells would overflow
    if (numCells > 0 && maxBytesPerCell > INT64_MAX / numCells) {
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
    if (arcset->buckets) {
        H3_MEMORY(free)(arcset->buckets);
        arcset->buckets = NULL;
    }
}

/*
Helper function to free memory allocated for a SortableLoopSet.
Frees all vertex arrays in the loops, then the loops array itself.
*/
static inline void destroySortableLoopSet(SortableLoopSet *loopset) {
    if (loopset->sloops) {
        for (int i = 0; i < loopset->numLoops; i++) {
            if (loopset->sloops[i].loop.verts) {
                H3_MEMORY(free)(loopset->sloops[i].loop.verts);
            }
        }
        H3_MEMORY(free)(loopset->sloops);
        loopset->sloops = NULL;
    }
}

/*
Helper function to free memory allocated for an array of SortablePoly.
Frees the holes arrays in each polygon, then the polygon array itself.
numPolys specifies how many polygons to clean up.
*/
static inline void destroySortablePolys(SortablePoly *spolys, int numPolys) {
    if (spolys) {
        for (int i = 0; i < numPolys; i++) {
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
// TODO: do we need two separate? can we avoid passing numPolys?
static inline void destroySortablePolyVerts(SortablePoly *spolys,
                                            int numPolys) {
    if (spolys) {
        for (int i = 0; i < numPolys; i++) {
            if (spolys[i].poly.geoloop.verts) {
                H3_MEMORY(free)(spolys[i].poly.geoloop.verts);
            }
        }
        H3_MEMORY(free)(spolys);
    }
}

#endif
