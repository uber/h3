# RFC: Names for H3 Concepts, Types, and Functions

- **Authors**: AJ Friend
- **Date**: 2020-02-02
- **Status**: Draft

## Motivation

Concepts like `hexagon`, `cell`, and `index` are currently used ambiguously in the H3 codebase and in documentation.
This can cause confusion, for example, when a function might only work on strict H3 *hexagons*, but fails when encountering H3 *pentagons*.

Reaching a consensus on the precise, technical language used when discussing H3 concepts will clarify future library discussions and improve end-user documentation.

We would also like to standardize a function-naming scheme.

## Terminology

The following technical terms should be used in the documentation, the H3 codebase, and precise technical discussions of the library.

- `H3Index`:
    - an unsigned 64-bit integer representing *any* H3 object (hexagon, pentagon, directed edge, ...)
    - often represented as a 15-character (or 16-character) hexadecimal string, like `'8928308280fffff'`
    - the full term "H3 index" should be used to avoid confusion with other common uses of "index";
      when a "traditional" index is needed, prefer using "number", "pos", or another term to avoid confusion
- **mode**:
    - an integer describing the type of object being represented by an H3 index
    - this integer is encoded in the `H3Index`
- **cell** or **H3 cell**:
    - a geometric polygon corresponding to an `H3Index` of `mode 1` (hexagon or pentagon)
    - for functions that can handle either hexagons or pentagons, the more general term "cell" should be used whenever possible
- **hexagon**:
    - an H3 **cell** which is a **topological** hexagon
    - todo: advise use of the term "strict" for functions that can only handle hexagons?
- **pentagon**:
    - an H3 **cell** which is a **topological** pentagon
- **directed edge**:
    - represents a traversal from an origin **cell** to an adjacent destination **cell**
    - corresponds to an `H3Index` of `mode 2`
- **grid**:
    - the graph with nodes corresponding to H3 cells, and edges given by pairs of adjacent cells
    - for example, `gridDistance` is the minimal number of edges in a graph path connecting two cells
- **point**:
    - a representation of a geometric point in terms of a latitude/longitude pair
- **topological**:
    - H3 cells are **topological** pentagons or hexagons, in the sense that they have 5 or 6 neighbors, respectively, in the H3 **grid**
    - the majority of **hexagons** are also **geometric** hexagons (similarly with **pentagons**), in that they have 6 edges and vertices when represented as polygons of lat/lng points
    - a small number of **hexagons** are not **geometric** hexagons (similarly with **pentagons**), in that they have extra vertices and edges due to distortion around icosahedron boundaries
    - for more details, see this [h3-js issue](https://github.com/uber/h3-js/issues/53) or this [Observable post](https://observablehq.com/@fil/h3-oddities)

### Use of "hex", "hexagon", "cell", "pentagon", etc.

We realize that "hex" or "hexagon" will still be used informally to refer to the concept of "cell" (As the development team, we do it ourselves!).
This should be expected in casual, informal discussions of H3.
However, when *precision* is required, we advise the use of strict technical terms like "index", "cell", "hexagon", "pentagon", etc.
In the codebase and in the documentation, strictly correct terminology should *always* be used, as many functions and algorithms distinguish between hexagons and pentagons.


## `H3_EXPORT` and C name collisions

To avoid C name collisions, we should build the bindings (`h3-py`, `h3-java`, `h3-go`, and `h3-node`) with a standard function prefix, using the `H3_EXPORT` C macro.
The `h3-js` binding would not need this.

The proposed prefix is `h3_`.

## Functions

### Conventions for "property getters" and transforms

- use `get` prefix for
    + constant data (`getNumCells`, `getRes0Cells`)
    + object properties (`getResolution`, `getBaseCellNumber`)
- use `to` to denote transforms
    + different representations of the same object
    + when doing a lossy transformation to a new object (`cellToParent`, `pointToCell`)
- do not use `get` or `two` for *computations*
    + e.g., `polyfill`, `compact`, `cellAreaKm2`

There is some ambiguity between property, transform, and computation, so use your best judgement with these guidelines in mind.

### Validity checks

- `isValid*` should mean that a full validity check is made
- without `Valid` (like in the case of `isPentagon`), we do not guarantee
  that a full validity check is made; instead, a user should assume only a
  minimal bit mask check is done
    - we could imagine a `isValidPengaton` function, if full verification
      would be convenient
- similarly, a function like `areNeighborCells` will assume each cell
  has passed the `isValidCell` check; the function will do only minimal
  work to determine if they are neighbors


### General Function Names

|          Current name         |     Proposed name     |                      Notes                      |
|-------------------------------|-----------------------|-------------------------------------------------|
| *Does Not Exist (DNE)*        | `isValidIndex`        |                                                 |
| `h3IsValid`                   | `isValidCell`         |                                                 |
| `h3UnidirectionalEdgeIsValid` | `isValidDirectedEdge` |                                                 |
| `h3IsPentagon`                | `isPentagon`          |                                                 |
| `h3IsResClassIII`             | `isResClassIII`       |                                                 |
| `h3IndexesAreNeighbors`       | `areNeighborCells`    |                                                 |
| `h3ToParent`                  | `cellToParent`        |                                                 |
| `h3ToChildren`                | `cellToChildren`      |                                                 |
| `numHexagons`                 | `getNumCells`         |                                                 |
| `getRes0Indexes`              | `getRes0Cells`        |                                                 |
| `getPentagonIndexes`          | `getPentagons`        |                                                 |
| `h3GetBaseCell`               | `getBaseCellNumber`   |                                                 |
| `h3GetResolution`             | `getResolution`       | should this work for all modes, not just cells? |
| `geoToH3`                     | `pointToCell`         |                                                 |
| `h3ToGeo`                     | `cellToPoint`         |                                                 |
| `h3ToGeoBoundary`             | `cellToPoly`          |                                                 |
| `compact`                     | same                  |                                                 |
| `uncompact`                   | same                  |                                                 |
| `polyfill`                    | same                  |                                                 |
| *DNE*                         | `getMode`             |                                                 |


### H3 Grid Functions


#### Distance

| Current name |      Proposed name      |     Notes      |
|--------------|-------------------------|----------------|
| `h3Distance` | `gridDistance`          |                |
| `h3Line`     | `gridPathCells`         |                |
| *DNE*        | `gridPathEdges`         |                |
| *DNE*        | `gridPathDirectedEdges` |                |


#### Filled-In Disk With Distances

|     Current name    |      Proposed name       |    Type   |     Notes      |                Calls                 |
|---------------------|--------------------------|-----------|----------------|--------------------------------------|
| `hexRangeDistances` | `gridDiskDistances_fast` | hex only  | disk/distances | NONE                                 |
| `_kRingInternal`    | `gridDiskDistances_slow` | pentagons | disk/distances | NONE                                 |
| `kRingDistances`    | `gridDiskDistances`      | general   | disk/distances | `hexRangeDistances`,`_kRingInternal` |

#### Filled-In Disk Without Distances

| Current name |  Proposed name   |    Type   | Notes |                Calls                 |
|--------------|------------------|-----------|-------|--------------------------------------|
| `kRing`      | `gridDisk`       | general   | disk  | `kRingDistances`, drops distances    |
| `hexRange`   | `gridDisk_fast`? | hex only  | disk  | `hexRangeDistances`, drops distances |
| *DNE*        | `gridDisk_slow`? | pentagons | disk  |                                      |

#### Hollow Ring

| Current name |  Proposed name  |    Type   | Notes | Calls |
|--------------|-----------------|-----------|-------|-------|
| *DNE*        | `gridRing`      | general   | ring  |       |
| `hexRing`    | `gridRing_fast` | hex only  | ring  | NONE  |
| *DNE*        | `gridRing_slow` | pentagons | ring  |       |


#### To Remove

| Current name | Proposed name |   Type   |     Calls      |
|--------------|---------------|----------|----------------|
| `hexRanges`  | (remove)      | hex only | N x `hexRange` |


#### Todo
- maybe there's a better suffix than `_fast`/`_slow`?
- what's going on with these distance allocations (or lack of)?

```C
// unnecessary malloc?
void H3_EXPORT(kRing)(H3Index origin, int k, H3Index* out) {
    int maxIdx = H3_EXPORT(maxKringSize)(k);
    int* distances = malloc(maxIdx * sizeof(int));
    H3_EXPORT(kRingDistances)(origin, k, out, distances);
    free(distances);
}

// missing malloc?
int H3_EXPORT(hexRange)(H3Index origin, int k, H3Index* out) {
    return H3_EXPORT(hexRangeDistances)(origin, k, out, 0);
}
```

### H3 Edge Types

Instead of `UnidirectionalEdge`, use the term `DirectedEdge`.

For a future undirected edge mode, use the term `Edge`.

|                  Current name                 |                       Proposed name                       |
|-----------------------------------------------|-----------------------------------------------------------|
| `h3UnidirectionalEdgeIsValid`                 | `isValidDirectedEdge`                                     |
| `getH3UnidirectionalEdge`                     | `cellsToDirectedEdge`                                     |
| `getH3IndexesFromUnidirectionalEdge`          | `directedEdgeToCells`                                     |
| `getH3UnidirectionalEdgesFromHexagon`         | `originToDirectedEdges`                                   |
| *DNE*                                         | `destinationToDirectedEdges`                              |
| `getH3UnidirectionalEdgeBoundary`             | `directedEdgeToLine`                                      |
| `getOriginH3IndexFromUnidirectionalEdge`      | `getDirectedEdgeOrigin`, `directedEdgeToOrigin`           |
| `getDestinationH3IndexFromUnidirectionalEdge` | `getDirectedEdgeDestination`, `directedEdgeToDestination` |


### Area/Length Functions

|  Current name  |      Proposed name      |         Notes         |
|----------------|-------------------------|-----------------------|
| `hexAreaKm2`   | `getHexAreaAvgKm2`      |                       |
| `hexAreaM2`    | `getHexAreaAvgM2`       |                       |
| `edgeLengthKm` | `getHexEdgeLengthAvgKm` |                       |
| `edgeLengthM`  | `getHexEdgeLengthAvgM`  |                       |
| *DNE*          | `getPentagonAreaAvgKm2` | + others              |
| *DNE*          | `cellAreaKm2`           | area of specific cell |

todo: add min/max versions in addition to avg
