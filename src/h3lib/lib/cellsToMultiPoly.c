#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "algos.h"
#include "alloc.h"
#include "area.h"
#include "h3Assert.h"
#include "h3api.h"

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

    double A = a->outerArea;
    double B = b->outerArea;

    // Sort by area of outer loop, in descending order.
    // NOTE: Using comparison trick because it is hard to hit the equality
    // branch for coverage.
    return (B > A) - (B < A);
}

static inline int cmp_raw(const void *a, const void *b) {
    H3Index ha = *(const H3Index *)a;
    H3Index hb = *(const H3Index *)b;
    if (ha < hb) return -1;
    if (ha > hb) return +1;
    return 0;
}

static inline H3Error validateCellSet(const H3Index *cells,
                                      const int64_t numCells) {
    if (numCells < 0) {
        return E_DOMAIN;
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
        memcpy(cellsCopy, cells, numCells * sizeof(H3Index));
        qsort(cellsCopy, numCells, sizeof(H3Index), cmp_raw);
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

- set prev/next arcs in arced loop. ensures edges in CCW order
- set parent and rank for union_find
- returns number of edges written
*/
static inline int64_t cellToEdgeArcs(H3Index h, Arc *arcs) {
    int64_t numEdges;
    H3Index _edges[6];
    H3Index *edges;

    static const uint8_t idxh[6] = {0, 4, 3, 5, 1, 2};
    static const uint8_t idxp[5] = {0, 1, 3, 2, 4};
    const uint8_t *idx;

    H3_EXPORT(originToDirectedEdges)(h, _edges);

    // the first directed edge of a pentagon is H3_NULL
    if (_edges[0] == H3_NULL) {
        numEdges = 5;
        idx = idxp;
        edges = &_edges[1];
    } else {
        numEdges = 6;
        idx = idxh;
        edges = &_edges[0];
    }

    for (int i = 0; i < numEdges; i++) {
        // arcs stay in same order as output of originToDirectedEdges
        arcs[i].id = edges[i];
        arcs[i].isRemoved = false;
        arcs[i].isVisited = false;

        // initialize union-find datastructure
        arcs[i].parent = &arcs[0];
        arcs[i].rank = 1;

        // connect so prev/next point to neighboring edges that share a vertex
        // edges/vertexes should follow right-hand rule as a result
        // TODO: this idx stuff will be cleaner when we use an edge iterator
        int cur = idx[i];
        int prev = idx[(i - 1 + numEdges) % numEdges];
        int next = idx[(i + 1) % numEdges];
        arcs[cur].prev = &arcs[prev];
        arcs[cur].next = &arcs[next];
    }

    return numEdges;
}

static ArcSet createArcSet(const H3Index *cells, const int64_t numCells) {
    int64_t numArcs = getNumEdges(cells, numCells);
    int64_t numBuckets = numArcs * HASH_TABLE_MULTIPLIER;

    ArcSet arcset = {
        .numArcs = numArcs,
        .numBuckets = numBuckets,
        .arcs = H3_MEMORY(malloc)(numArcs * sizeof(Arc)),
        .buckets = H3_MEMORY(calloc)(numBuckets, sizeof(Arc *)),
    };

    int64_t j = 0;
    for (int64_t i = 0; i < numCells; i++) {
        j += cellToEdgeArcs(cells[i], &arcset.arcs[j]);
    }

    for (int64_t i = 0; i < arcset.numArcs; i++) {
        // hash edge to initial bucket
        int64_t j = hashEdge(arcset.arcs[i].id, arcset.numBuckets);

        // linear probe to find next open bucket. wraps around if needed.
        while (arcset.buckets[j] != NULL) {
            j = (j + 1) % arcset.numBuckets;
        }
        arcset.buckets[j] = &arcset.arcs[i];
    }

    return arcset;
}

static inline Arc *findArc(ArcSet arcset, H3Index e) {
    int64_t j = hashEdge(e, arcset.numBuckets);

    while (arcset.buckets[j] != NULL && arcset.buckets[j]->id != e) {
        j = (j + 1) % arcset.numBuckets;
    }

    // returns NULL if edge not found
    return arcset.buckets[j];
}

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
static void cancelArcPairs(ArcSet arcset) {
    for (int i = 0; i < arcset.numArcs; i++) {
        Arc *a = &arcset.arcs[i];

        if (!a->isRemoved) {
            H3Index reversedEdge;
            H3_EXPORT(reverseDirectedEdge)(a->id, &reversedEdge);
            Arc *b = findArc(arcset, reversedEdge);

            if (b) {
                // Two loops overlap at edge a, and its reversed edge b.
                // Remove the two edges, and merge the loops to maintain
                // valid loops. The two loops might be the same loop.

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
        }
    }
}

static inline void resetVisited(ArcSet arcset) {
    for (int i = 0; i < arcset.numArcs; i++) {
        arcset.arcs[i].isVisited = false;
    }
}

static int countLoops(ArcSet arcset) {
    Arc *arcs = arcset.arcs;
    resetVisited(arcset);
    int numLoops = 0;

    for (int i = 0; i < arcset.numArcs; i++) {
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

static SortableLoop createSortableLoop(Arc *arc) {
    CellBoundary gb;
    H3Index start = arc->id;

    int numVerts;
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

    numVerts = 0;
    int j = 0;
    do {
        H3_EXPORT(directedEdgeToBoundary)(arc->id, &gb);

        for (int i = 0; i < gb.numVerts - 1; i++) {
            verts[j] = gb.verts[i];
            j++;
        }
        numVerts += gb.numVerts - 1;
        arc->isVisited = true;
        arc = arc->next;
    } while (arc->id != start);

    // This memory ends up in GeoMultiPolygon, to be freed by caller of
    // cellsToMultiPolygon()
    verts = H3_MEMORY(realloc)(verts, sizeof(LatLng) * numVerts);

    SortableLoop sloop = {
        .root = getRoot(arc)->id,
        .loop = {.numVerts = numVerts, .verts = verts},
    };
    geoLoopAreaRads2(sloop.loop, &sloop.area);

    return sloop;
}

static SortableLoopSet createSortableLoopSet(ArcSet arcset) {
    int numLoops = countLoops(arcset);
    resetVisited(arcset);
    Arc *arcs = arcset.arcs;

    SortableLoop *sloops = H3_MEMORY(malloc)(sizeof(SortableLoop) * numLoops);
    int j = 0;
    for (int i = 0; i < arcset.numArcs; i++) {
        if (!arcs[i].isVisited && !arcs[i].isRemoved) {
            sloops[j] = createSortableLoop(&arcs[i]);
            j++;
        }
    }

    // The comparison function makes all polygons (outer loop and holes)
    // contiguous in memory, so that the outer loop "contains" the holes.
    qsort(sloops, numLoops, sizeof(SortableLoop), cmp_SortableLoop);

    SortableLoopSet loopset = {
        .numLoops = numLoops,
        .sloops = sloops,
    };

    return loopset;
}

static int countPolys(SortableLoopSet loopset) {
    int numPolys = 0;

    H3Index cur = H3_NULL;
    for (int i = 0; i < loopset.numLoops; i++) {
        if (loopset.sloops[i].root != cur) {
            numPolys++;
            cur = loopset.sloops[i].root;
        }
    }

    return numPolys;
}

static SortablePoly createSortablePoly(SortableLoop *sloop, int numHoles) {
    GeoPolygon poly = {
        .geoloop = sloop[0].loop,
        .numHoles = numHoles,
        .holes = H3_MEMORY(malloc)(sizeof(GeoLoop) * numHoles),
    };
    for (int k = 0; k < numHoles; k++) {
        poly.holes[k] = sloop[k + 1].loop;
    }

    SortablePoly spoly = {
        .outerArea = sloop[0].area,  // area of outer loop
        .poly = poly,
    };

    return spoly;
}

static GeoMultiPolygon createMultiPolygon(SortableLoopSet loopset) {
    if (loopset.numLoops == 0) {
        return createGlobeMultiPolygon();
    }

    int numPolys = countPolys(loopset);
    SortablePoly *spolys = H3_MEMORY(malloc)(sizeof(SortablePoly) * numPolys);

    SortableLoop *sloop = loopset.sloops;
    int i = 0;  // index of first loop in polygon (outer loop)
    int j = 0;  // index + 1 of last loop in polygon (last hole + 1)
    int p = 0;  // index of polygon we're working on
    for (; j <= loopset.numLoops; j++) {
        if (j == loopset.numLoops || sloop[i].root != sloop[j].root) {
            // We've reached the end of the loops in the polygon, so
            // now construct a polygon from the start of those loops.
            spolys[p] = createSortablePoly(&sloop[i], (j - i) - 1);
            p++;
            i = j;
        }
    }

    // NOTE: sorted by area of outer loop
    qsort(spolys, numPolys, sizeof(SortablePoly), cmp_SortablePoly);

    GeoMultiPolygon mpoly = {
        .numPolygons = numPolys,
        .polygons = H3_MEMORY(malloc)(sizeof(GeoPolygon) * numPolys),
    };

    for (int i = 0; i < numPolys; i++) {
        mpoly.polygons[i] = spolys[i].poly;
    }

    H3_MEMORY(free)(spolys);

    return mpoly;
}

/**
 * Create a GeoMultiPolygon from a set of H3 cells.
 *
 * This function converts a set of H3 cells into a GeoMultiPolygon
 * representing the region they cover. Note the difference with
 * cellsToLinkedMultiPolygon, which returns a arced-list LinkedGeoPolygon.
 * A GeoMultiPolygon has provides the sizes of its elements and supports
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
    if (numCells == 0) {
        out->numPolygons = 0;
        out->polygons = NULL;
        return E_SUCCESS;
    }

    H3Error err = validateCellSet(cells, numCells);
    if (err) return err;

    // arcset initializes with separate doubly-linked loops for each cell,
    // each in their own connected component
    ArcSet arcset = createArcSet(cells, numCells);

    // Cancel out pairs of edges, updating the doubly-linked loop and merging
    // them into a single connected component
    cancelArcPairs(arcset);

    /*
    Extract all loops and sort them by:
      1) their connected component, and then by
      2) the loop area.
    This makes loops for each polygon contiguous in memory, and within
    each polygon, the sorting makes the largest loop come first, which is
    what we take to be the outer loop.
    */
    SortableLoopSet loopset = createSortableLoopSet(arcset);

    // Extract polygons, since loops are contiguous in SortableLoopSet memory.
    // Polygons sorted by outer loop area, decreasing.
    *out = createMultiPolygon(loopset);

    H3_MEMORY(free)(arcset.arcs);
    H3_MEMORY(free)(arcset.buckets);
    H3_MEMORY(free)(loopset.sloops);

    return E_SUCCESS;
}
