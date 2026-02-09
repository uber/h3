#include "cellsToMultiPoly.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "algos.h"
#include "alloc.h"
#include "area.h"
#include "h3Assert.h"
#include "h3api.h"

static inline H3Error validateCellSet(const H3Index *cells,
                                      const int64_t numCells) {
    if (numCells < 0) {
        return E_DOMAIN;
    }
    if (numCells == 0) {
        return E_SUCCESS;
    }

    // Check that all cells are valid and have the same resolution
    int res = H3_EXPORT(getResolution)(cells[0]);
    for (int64_t i = 0; i < numCells; i++) {
        if (!H3_EXPORT(isValidCell)(cells[i])) {
            return E_CELL_INVALID;
        }
        if (H3_EXPORT(getResolution)(cells[i]) != res) {
            return E_RES_MISMATCH;
        }
    }

    // Check for duplicate cells by sorting a copy and looking for adjacent
    // duplicates
    if (numCells >= 2) {
        H3Index *cellsCopy = H3_MEMORY(malloc)(numCells * sizeof(H3Index));
        if (!cellsCopy) {
            return E_MEMORY_ALLOC;
        }
        memcpy(cellsCopy, cells, numCells * sizeof(H3Index));
        qsort(cellsCopy, numCells, sizeof(H3Index), cmp_uint64);
        for (int64_t i = 1; i < numCells; i++) {
            if (cellsCopy[i] == cellsCopy[i - 1]) {
                H3_MEMORY(free)(cellsCopy);
                return E_DUPLICATE_INPUT;
            }
        }
        H3_MEMORY(free)(cellsCopy);
    }

    return E_SUCCESS;
}

/**
 * Hash an H3Index to a bucket index for hash table lookups.
 *
 * Uses a mixing function based on SplitMix64 to ensure good distribution
 * of hash values.
 *
 * @param x H3Index value to hash
 * @param n Number of hash table buckets
 * @return Bucket index in range [0, n-1]
 *
 * Reference: Steele et al., "Fast splittable pseudorandom number generators"
 * OOPSLA 2014. https://doi.org/10.1145/2660193.2660195
 */
static inline uint64_t hashEdge(H3Index x, uint64_t n) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;

    return x % n;
}

static int64_t getNumEdges(const H3Index *cells, const int64_t numCells) {
    int64_t numEdges = 6 * numCells;

    for (int64_t i = 0; i < numCells; i++) {
        if (H3_EXPORT(isPentagon)(cells[i])) {
            numEdges--;
        }
    }
    return numEdges;
}

/*
Fill in edge arcs for a single cell:

- create one Arc for each edge of the cell
- set prev/next arcs in linked loop. ensures edges in counter-clockwise order
- initialize parent and rank for union_find (each loop of cell edges starts as
its own separate connected component)
*/
static inline H3Error cellToEdgeArcs(H3Index h, Arc *arcs,
                                     int64_t *numEdgesOut) {
    int64_t numEdges;
    H3Index _edges[6];
    H3Index *edges;

    static const uint8_t idxh[6] = {0, 4, 3, 5, 1, 2};
    static const uint8_t idxp[5] = {0, 1, 3, 2, 4};
    const uint8_t *idx;

    H3Error err = H3_EXPORT(originToDirectedEdges)(h, _edges);
    if (NEVER(err)) {
        // Since already checked with validateCellSet, this should never error.
        return err;
    }

    // Set `edges` to contain the indices of cell edges in counter-clockwise
    // order the first directed edge of a pentagon is H3_NULL
    if (_edges[0] == H3_NULL) {
        numEdges = 5;
        idx = idxp;
        edges = &_edges[1];
    } else {
        numEdges = 6;
        idx = idxh;
        edges = &_edges[0];
    }

    for (int64_t i = 0; i < numEdges; i++) {
        // Arcs stay in same order as output of originToDirectedEdges.
        // That is, they are not in CCW order in the `arcs` array, but they
        // are in CCW in the linked loop.
        arcs[i].id = edges[i];
        arcs[i].isRemoved = false;
        arcs[i].isVisited = false;

        // initialize union-find datastructure
        // all edges in loop have same parent: first edge
        arcs[i].parent = &arcs[0];
        arcs[i].rank = 1;

        // Connect so prev/next point to neighboring edges that share a vertex.
        // Edges/vertexes should follow right-hand rule as a result (CCW order).
        // TODO: this idx stuff will be cleaner when we use an edge iterator
        int64_t cur = idx[i];
        int64_t prev = idx[(i - 1 + numEdges) % numEdges];
        int64_t next = idx[(i + 1) % numEdges];
        arcs[cur].prev = &arcs[prev];
        arcs[cur].next = &arcs[next];
    }

    *numEdgesOut = numEdges;
    return E_SUCCESS;
}

static H3Error createArcSet(const H3Index *cells, const int64_t numCells,
                            ArcSet *arcset) {
    int64_t numArcs = getNumEdges(cells, numCells);
    int64_t numBuckets = numArcs * HASH_TABLE_MULTIPLIER;

    arcset->numArcs = numArcs;
    arcset->numBuckets = numBuckets;
    arcset->arcs = H3_MEMORY(malloc)(numArcs * sizeof(Arc));
    if (!arcset->arcs) {
        return E_MEMORY_ALLOC;
    }

    arcset->buckets = H3_MEMORY(calloc)(numBuckets, sizeof(Arc *));
    if (!arcset->buckets) {
        destroyArcSet(arcset);
        return E_MEMORY_ALLOC;
    }

    int64_t j = 0;
    for (int64_t i = 0; i < numCells; i++) {
        int64_t numEdges;
        H3Error err = cellToEdgeArcs(cells[i], &arcset->arcs[j], &numEdges);
        if (NEVER(err)) {
            destroyArcSet(arcset);
            return err;
        }
        j += numEdges;
    }

    for (int64_t i = 0; i < arcset->numArcs; i++) {
        // hash edge to initial bucket
        int64_t j = hashEdge(arcset->arcs[i].id, arcset->numBuckets);

        // linear probe to find next open bucket. wraps around if needed.
        while (arcset->buckets[j] != NULL) {
            j = (j + 1) % arcset->numBuckets;
        }
        arcset->buckets[j] = &arcset->arcs[i];
    }

    return E_SUCCESS;
}

static inline Arc *findArc(ArcSet arcset, H3Index e) {
    int64_t j = hashEdge(e, arcset.numBuckets);

    // hash + linear probe to find edge
    while (arcset.buckets[j] != NULL && arcset.buckets[j]->id != e) {
        j = (j + 1) % arcset.numBuckets;
    }

    // returns NULL if edge not found
    return arcset.buckets[j];
}

// Part of union-find data structure
// Finds the id of the connected component this arc/edge is a part of
static inline Arc *getRoot(Arc *arc) {
    Arc *parent = arc->parent;

    if (parent == arc) {
        return arc;
    } else {
        parent = getRoot(parent);
        arc->parent = parent;
        return parent;
    }
}

// Part of union-find data structure
// Merge two arcs/edges into a single connected component
static void unionArcs(Arc *a, Arc *b) {
    a = getRoot(a);
    b = getRoot(b);

    if (a->rank < b->rank) {
        // swap so `a` has bigger rank
        Arc *tmp = a;
        a = b;
        b = tmp;
    }

    if (a != b) {
        // `a` has bigger rank
        a->rank += b->rank;
        b->parent = a;
    }
}

/*
Cancel out pairs of edges in the ArcSet, marking them as isRemoved.
Update the doubly-linked loop list to maintain valid loops.
Merge the connected components of edge pairs; each connected component
denotes a separate polygon (outer loop and holes).
*/
static H3Error cancelArcPairs(ArcSet arcset) {
    for (int64_t i = 0; i < arcset.numArcs; i++) {
        Arc *a = &arcset.arcs[i];

        if (a->isRemoved) {
            // Already removed, so we can skip.
            continue;
        }

        H3Index reversedEdge;
        H3Error err = H3_EXPORT(reverseDirectedEdge)(a->id, &reversedEdge);
        if (NEVER(err)) {
            return err;
        }

        Arc *b = findArc(arcset, reversedEdge);
        if (!b) {
            // The reversed edge was *not* in the set, so there's nothing to do.
            continue;
        }

        // If we're at this point, then the two loops overlap at edges
        // `a` and `b`, which are opposites of each other.
        // Remove the two edges, and merge the loops to maintain
        // valid doubly-linked loops. Note that the two loops might be the
        // *same* loop, and the logic is the same either way.

        // mark both as removed
        a->isRemoved = true;
        b->isRemoved = true;

        // stitch together loops at removal site
        a->next->prev = b->prev;
        a->prev->next = b->next;
        b->next->prev = a->prev;
        b->prev->next = a->next;

        // update parent to merge into a single connected component
        unionArcs(a, b);
    }

    return E_SUCCESS;
}

static inline void resetVisited(ArcSet arcset) {
    for (int64_t i = 0; i < arcset.numArcs; i++) {
        arcset.arcs[i].isVisited = false;
    }
}

// Count number of distinct loops in an ArcSet
static int64_t countLoops(ArcSet arcset) {
    Arc *arcs = arcset.arcs;
    resetVisited(arcset);
    int64_t numLoops = 0;

    for (int64_t i = 0; i < arcset.numArcs; i++) {
        Arc *arc = &arcs[i];
        if (!arc->isVisited && !arc->isRemoved) {
            numLoops++;
            H3Index start = arc->id;

            do {
                arc->isVisited = true;
                arc = arc->next;
            } while (arc->id != start);
        }
    }

    return numLoops;
}

// Starting from a given Arc, create a SortableLoop that contains that Arc
// SortableLoops are sorted by the root (i.e., connected component) and then
// by the area contained by the loop. We use this to merge all loops in a
// connected component into a single polygon. We use the area values to
// determine which loop will be the "outer" loop of the polygon.
static H3Error createSortableLoop(Arc *arc, SortableLoop *sloop) {
    CellBoundary gb;
    H3Index start = arc->id;

    int64_t numVerts;
    LatLng *verts;

    numVerts = 0;
    do {
        // This is an overestimate for numVerts.
        // Most cell edges will just need one vert (we don't use the last vertex
        // in the edge).
        // For even resolutions, all cell edges need just one vert.
        // Over-allocate for now; realloc to actual number later.
        numVerts += 2;
        arc = arc->next;
    } while (arc->id != start);

    verts = H3_MEMORY(malloc)(sizeof(LatLng) * numVerts);
    if (!verts) {
        return E_MEMORY_ALLOC;
    }

    numVerts = 0;
    int64_t j = 0;
    do {
        H3Error err = H3_EXPORT(directedEdgeToBoundary)(arc->id, &gb);
        if (NEVER(err)) {
            free(verts);
            return err;
        }

        for (int64_t i = 0; i < gb.numVerts - 1; i++) {
            verts[j] = gb.verts[i];
            j++;
        }
        numVerts += gb.numVerts - 1;
        arc->isVisited = true;
        arc = arc->next;
    } while (arc->id != start);

    // This memory ends up in GeoMultiPolygon, to be freed by caller of
    // cellsToMultiPolygon()
    LatLng *reallocVerts = H3_MEMORY(realloc)(verts, sizeof(LatLng) * numVerts);
    if (!reallocVerts) {
        H3_MEMORY(free)(verts);
        return E_MEMORY_ALLOC;
    }
    verts = reallocVerts;

    sloop->root = getRoot(arc)->id;
    sloop->loop.numVerts = numVerts;
    sloop->loop.verts = verts;
    geoLoopAreaRads2(sloop->loop, &sloop->area);

    return E_SUCCESS;
}

// Create set of all SortableLoops and sort them
static H3Error createSortableLoopSet(ArcSet arcset, SortableLoopSet *loopset) {
    int64_t numLoops = countLoops(arcset);
    resetVisited(arcset);
    Arc *arcs = arcset.arcs;

    SortableLoop *sloops = H3_MEMORY(malloc)(sizeof(SortableLoop) * numLoops);
    if (!sloops) {
        return E_MEMORY_ALLOC;
    }

    int64_t j = 0;
    for (int64_t i = 0; i < arcset.numArcs; i++) {
        if (!arcs[i].isVisited && !arcs[i].isRemoved) {
            H3Error err = createSortableLoop(&arcs[i], &sloops[j]);
            if (err) {
                // Free any verts already allocated in previous loops
                SortableLoopSet partialLoopSet = {.numLoops = j,
                                                  .sloops = sloops};
                destroySortableLoopSet(&partialLoopSet);
                return err;
            }
            j++;
        }
    }

    // The comparison function makes all polygons (outer loop and holes)
    // contiguous in memory, so that the outer loop "contains" the holes.
    qsort(sloops, numLoops, sizeof(SortableLoop), cmp_SortableLoop);

    loopset->numLoops = numLoops;
    loopset->sloops = sloops;

    return E_SUCCESS;
}

static int64_t countPolys(SortableLoopSet loopset) {
    int64_t numPolys = 0;

    H3Index cur = H3_NULL;
    for (int64_t i = 0; i < loopset.numLoops; i++) {
        if (loopset.sloops[i].root != cur) {
            numPolys++;
            cur = loopset.sloops[i].root;
        }
    }

    return numPolys;
}

// Create a SortablePolygon from a given SortableLoop.
// The "outer ring" SortableLoop is first in memory, followed by its holes
// Later, we sort the Polygons by the size of their outer loops.
static H3Error createSortablePoly(SortableLoop *sloop, int64_t numHoles,
                                  SortablePoly *spoly) {
    GeoLoop *holes = NULL;
    if (numHoles > 0) {
        holes = H3_MEMORY(malloc)(sizeof(GeoLoop) * numHoles);
        if (!holes) {
            return E_MEMORY_ALLOC;
        }
        for (int64_t k = 0; k < numHoles; k++) {
            holes[k] = sloop[k + 1].loop;
        }
    }

    spoly->poly.geoloop = sloop[0].loop;
    spoly->poly.numHoles = numHoles;
    spoly->poly.holes = holes;
    spoly->outerArea = sloop[0].area;  // area of outer loop

    return E_SUCCESS;
}

/**
 * Allocate a GeoMultiPolygon representing the entire globe.
 * The globe is represented using 8 triangular polygons, with
 * all edge arcs of exactly 90 degrees (i.e., pi/2 radians).
 * Memory should be freed with `destroyGeoMultiPolygon`.
 *
 * @param mpoly Output parameter for the resulting GeoMultiPolygon
 * @return E_SUCCESS on success, E_MEMORY_ALLOC on allocation failure
 */
static H3Error createGlobeMultiPolygon(GeoMultiPolygon *mpoly) {
    const int numPolygons = 8;
    const int numVerts = 3;
    const LatLng verts[8][3] = {
        {{M_PI_2, 0.0}, {0.0, 0.0}, {0.0, M_PI_2}},
        {{M_PI_2, 0.0}, {0.0, M_PI_2}, {0.0, M_PI}},
        {{M_PI_2, 0.0}, {0.0, M_PI}, {0.0, -M_PI_2}},
        {{M_PI_2, 0.0}, {0.0, -M_PI_2}, {0.0, 0.0}},
        {{-M_PI_2, 0.0}, {0.0, 0.0}, {0.0, -M_PI_2}},
        {{-M_PI_2, 0.0}, {0.0, -M_PI_2}, {0.0, -M_PI}},
        {{-M_PI_2, 0.0}, {0.0, -M_PI}, {0.0, M_PI_2}},
        {{-M_PI_2, 0.0}, {0.0, M_PI_2}, {0.0, 0.0}},
    };

    SortablePoly *spolys =
        H3_MEMORY(malloc)(sizeof(SortablePoly) * numPolygons);
    if (!spolys) {
        return E_MEMORY_ALLOC;
    }

    for (int i = 0; i < numPolygons; i++) {
        GeoPolygon *poly = &spolys[i].poly;
        poly->numHoles = 0;
        poly->holes = NULL;
        poly->geoloop.numVerts = numVerts;
        poly->geoloop.verts = H3_MEMORY(malloc)(sizeof(LatLng) * numVerts);
        if (!poly->geoloop.verts) {
            // Free any verts already allocated in previous iterations
            destroySortablePolyVerts(spolys, i);
            return E_MEMORY_ALLOC;
        }

        for (int j = 0; j < numVerts; j++) {
            poly->geoloop.verts[j] = verts[i][j];
        }

        // Calculate outer area for sorting
        geoLoopAreaRads2(poly->geoloop, &spolys[i].outerArea);
    }

    qsort(spolys, numPolygons, sizeof(SortablePoly), cmp_SortablePoly);

    mpoly->polygons = H3_MEMORY(malloc)(sizeof(GeoPolygon) * numPolygons);
    if (!mpoly->polygons) {
        destroySortablePolyVerts(spolys, numPolygons);
        return E_MEMORY_ALLOC;
    }

    mpoly->numPolygons = numPolygons;
    for (int i = 0; i < numPolygons; i++) {
        mpoly->polygons[i] = spolys[i].poly;
    }

    H3_MEMORY(free)(spolys);

    return E_SUCCESS;
}

static H3Error createMultiPolygon(SortableLoopSet loopset,
                                  GeoMultiPolygon *mpoly) {
    if (loopset.numLoops == 0) {
        return createGlobeMultiPolygon(mpoly);
    }

    int64_t numPolys = countPolys(loopset);
    SortablePoly *spolys = H3_MEMORY(malloc)(sizeof(SortablePoly) * numPolys);
    if (!spolys) {
        return E_MEMORY_ALLOC;
    }

    SortableLoop *sloop = loopset.sloops;
    int64_t i = 0;  // index of first loop in polygon (outer loop)
    int64_t j = 0;  // index + 1 of last loop in polygon (last hole + 1)
    int64_t p = 0;  // index of polygon we're working on
    for (; j <= loopset.numLoops; j++) {
        if (j == loopset.numLoops || sloop[i].root != sloop[j].root) {
            // We've reached the end of the loops in the polygon, so
            // now construct a polygon from the start of those loops.
            H3Error err =
                createSortablePoly(&sloop[i], (j - i) - 1, &spolys[p]);
            if (err) {
                destroySortablePolys(spolys, p);
                return err;
            }
            p++;
            i = j;
        }
    }

    // Sort polygons by their outer loop area. For example, in a multipolygon
    // representing the USA, the continental US will come before any of the
    // Hawaiian islands
    qsort(spolys, numPolys, sizeof(SortablePoly), cmp_SortablePoly);

    mpoly->polygons = H3_MEMORY(malloc)(sizeof(GeoPolygon) * numPolys);
    if (!mpoly->polygons) {
        destroySortablePolys(spolys, numPolys);
        return E_MEMORY_ALLOC;
    }

    mpoly->numPolygons = numPolys;
    for (int64_t i = 0; i < numPolys; i++) {
        mpoly->polygons[i] = spolys[i].poly;
    }

    H3_MEMORY(free)(spolys);

    return E_SUCCESS;
}

/**
 * Create a GeoMultiPolygon from a set of H3 cells.
 *
 * This function converts a set of H3 cells into a GeoMultiPolygon
 * representing the region they cover. Note the difference with
 * cellsToLinkedMultiPolygon, which returns a linked-list LinkedGeoPolygon.
 * A GeoMultiPolygon provides the sizes of its elements and supports
 * direct indexing.
 *
 * Polygons follow the right hand rule, with the outer loop oriented
 * counter-clockwise, and the inner loops oriented clockwise.
 *
 * Polygons within a GeoMultiPolygon are ordered by decreasing area of the outer
 * loop.
 *
 * Note that for polygons with multiple loops
 * (one outer loop + at least one hole), *any* loop can serve as the outer
 * loop and still produce the *same* valid polygon. We use the convention of
 * choosing as the outer loop the one that would give the largest area
 * "outside" of that outer loop. This results in what users would probably
 * expect: a polygon for the land within a state/province with a large lake
 * would have the outer loop be the state's boundary, instead of the lake's
 * boundary.
 *
 * @param cells Array of H3 cell indexes. Must be valid cells at the same
 *              resolution with no duplicates.
 * @param numCells Number of cells in the array.
 * @param out Output parameter for the resulting GeoMultiPolygon. The caller
 *            is responsible for freeing this with destroyGeoMultiPolygon.
 * @return E_SUCCESS on success
 */
H3Error H3_EXPORT(cellsToMultiPolygon)(const H3Index *cells,
                                       const int64_t numCells,
                                       GeoMultiPolygon *out) {
    H3Error err =
        checkCellsToMultiPolyOverflow(numCells, HASH_TABLE_MULTIPLIER);
    if (err) return err;

    err = validateCellSet(cells, numCells);
    if (err) return err;

    if (numCells == 0) {
        out->numPolygons = 0;
        out->polygons = NULL;
        return E_SUCCESS;
    }

    // arcset initializes with separate doubly-linked loops for each cell,
    // each in their own connected component
    ArcSet arcset;
    err = createArcSet(cells, numCells, &arcset);
    if (err) return err;

    // Cancel out pairs of edges, updating the doubly-linked loops and merging
    // them into a single connected component
    err = cancelArcPairs(arcset);
    if (NEVER(err)) {
        destroyArcSet(&arcset);
        return err;
    }

    /*
    Extract all loops and sort them by:
      1) their connected component, and then by
      2) the loop area.
    This makes loops for each polygon contiguous in memory.
    Within each polygon, the sorting makes the loop with the smallest enclosed
    area come first (accounting for loop winding direction),
    which is what we take to be the outer loop for that polygon.
    */
    SortableLoopSet loopset;
    err = createSortableLoopSet(arcset, &loopset);
    if (err) {
        destroyArcSet(&arcset);
        return err;
    }

    // Extract polygons, since loops are contiguous in SortableLoopSet memory.
    // Polygons sorted by outer loop area, decreasing.
    err = createMultiPolygon(loopset, out);
    if (err) {
        destroySortableLoopSet(&loopset);
        destroyArcSet(&arcset);
        return err;
    }

    destroyArcSet(&arcset);
    destroySortableLoopSetShallow(&loopset);

    return E_SUCCESS;
}
