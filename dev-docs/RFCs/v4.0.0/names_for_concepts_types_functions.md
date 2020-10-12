# RFC: Names for H3 Concepts, Types, and Functions

- **Authors**:
    - AJ Friend
    - Nick Rabinowitz
    - Isaac Brodsky
    - David Ellis
- **Dates**:
    - Started: 2020-02-02
    - Accepted: 2020-03-26
- **Status**: Accepted
- **Discussions**:
    - <https://github.com/uber/h3/pull/308>

## Motivation

Concepts like `hexagon`, `cell`, and `index` are currently used ambiguously in the H3 codebase and in documentation.
This can cause confusion, for example, when a function might only work on strict H3 *hexagons*, but fails when encountering H3 *pentagons*.

Reaching a consensus on the precise, technical language used when discussing H3 concepts will clarify future library discussions and improve end-user documentation.

We would also like to standardize a function-naming scheme.

## Terminology

The following technical terms should be used in the documentation, the H3 codebase, and precise technical discussions of the library.

- `H3Index`:
    - an unsigned 64-bit integer representing *any* H3 object (hexagon, pentagon, directed edge ...)
    - often represented as a 15-character (or 16-character) hexadecimal string, like `'8928308280fffff'`
    - the full term "H3 index" should be used to avoid confusion with other common uses of "index";
      when a "traditional" index is needed, prefer using "number", "pos", or another term to avoid confusion
- **mode**:
    - an integer describing the type of object being represented by an H3 index
    - this integer is encoded in the `H3Index`
- **cell** or **H3 cell**:
    - a geometric/geographic unit polygon in the H3 grid, corresponding to an `H3Index` of `mode 1` (hexagon or pentagon)
    - for functions that can handle either hexagons or pentagons, the more general term "cell" should be used whenever possible
- **hexagon**:
    - an H3 **cell** that is a **topological** hexagon
    - below, we explain that functions that *only* work with **hexagons** have an `Unsafe` suffix;
      these functions are paired with ones having a `Safe` suffix, meaning they can handle **pentagons**, but are slower
- **pentagon**:
    - an H3 **cell** that is a **topological** pentagon
- **directed edge**:
    - represents a traversal from an origin **cell** to an adjacent destination **cell**
    - corresponds to an `H3Index` of `mode 2`
- **grid**:
    - the graph with nodes corresponding to H3 cells, and edges given by pairs of adjacent cells
    - for example, `gridDistance` is the minimal number of edges in a graph path connecting two cells
- **point**:
    - a representation of a geographic point in terms of a latitude/longitude pair
- **topological**:
    - H3 cells are **topological** pentagons or hexagons, in the sense that they have 5 or 6 neighbors, respectively, in the H3 **grid**
    - the majority of **hexagons** are also **geometric** hexagons (similarly with **pentagons**), in that they have 6 edges and vertices when represented as polygons of lat/lng points
    - a small number of **hexagons** are not **geometric** hexagons (similarly with **pentagons**), in that they have extra vertices and edges due to distortion around icosahedron boundaries
    - for more details, see this [h3-js issue](https://github.com/uber/h3-js/issues/53) or this [Observable post](https://observablehq.com/@fil/h3-oddities)
- **base cell**:
    - one of the 122 H3 **cells** (110 hexagons and 12 pentagons) of resolution `0`
    - every other cell in H3 is a child of a base cell
    - each base cell has a "base cell number" (0--121), which is encoded into the `H3Index` representation of every H3 cell
    - there is a one-to-one correspondence between the "base cell number" and the `H3Index` representation of resolution `0` cells
        + e.g., base cell 0 has `H3Index` hexadecimal representation `'8001fffffffffff'`
- **boundary**:
    - all or part of the list of geometric points that enclose an H3 cell
    - may include more than 6 points in the case that a cell is not a geometric hexagon, such as when a hexagon crosses an icosahedron boundary
    - may also be used to describe the boundary between two geometric cells, as in the case of an edge
    - represented in the H3 codebase with the `CellBoundary` struct (previously `GeoBoundary` before v4.0)
- `H3_NULL`;
    - equivalent to `0` and guaranteed to never be a valid `H3Index` (even after any future H3 **modes** are added)
    - returned by functions to denote an error, or to denote missing data in arrays of `H3Index`
    - analogous to `NaN` in floating point


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
- do not use `get` or `to` for *computations*
    + e.g., `polyfill`, `compact`, `cellAreaKm2`

There is some ambiguity between property, transform, and computation, so use your best judgement with these guidelines in mind.

### Validity checks

- `isValid*` should mean that a *full* validity check is made
- without `Valid` (like in the case of `isPentagon`), we do not guarantee
  that a full validity check is made; instead, a user should assume only a
  minimal bit mask check is done
    - we could imagine a `isValidPengaton` function, if full verification
      would be convenient
- similarly, a function like `areNeighborCells` will assume each cell
  has passed the `isValidCell` check; the function will do only minimal
  work to determine if they are neighbors


### General Function Names

|          Current name         |     Proposed name     |
|-------------------------------|-----------------------|
| *Does Not Exist (DNE)*        | `isValidIndex`        |
| `h3IsValid`                   | `isValidCell`         |
| `h3UnidirectionalEdgeIsValid` | `isValidDirectedEdge` |
| `h3IsPentagon`                | `isPentagon`          |
| `h3IsResClassIII`             | `isResClassIII`       |
| `h3IndexesAreNeighbors`       | `areNeighborCells`    |
| `h3ToParent`                  | `cellToParent`        |
| `h3ToCenterChild`             | `cellToCenterChild`   |
| `h3ToChildren`                | `cellToChildren`      |
| `numHexagons`                 | `getNumCells`         |
| `getRes0Indexes`              | `getRes0Cells`        |
| `getPentagonIndexes`          | `getPentagons`        |
| `h3GetBaseCell`               | `getBaseCellNumber`   |
| `h3GetResolution`             | `getResolution`       |
| *DNE*                         | `getMode`             |
| `h3GetFaces`                  | `getIcosahedronFaces` |
| `geoToH3`                     | `pointToCell`         |
| `h3ToGeo`                     | `cellToPoint`         |
| `compact`                     | `compactCells`        |
| `uncompact`                   | `uncompactCells`      |
| `polyfill`                    | `polygonToCells`      |

**Note**: `getResolution` and `getBaseCellNumber` should work for both cells and edges.


### H3 Grid Functions

Many of these functions will have three forms:
- `<func_name>`
- `<func_name>Unsafe`
- `<func_name>Safe`

The `Unsafe` version is fast, but may fail if it encounters a pentagon.
It should return a failure code in this case.

The `Safe` version is slower, but will work in all cases.

The version without either suffix is intended to be the one typically
used.
This version will first attempt the `Unsafe` version, and if
it detects failure, will fall back to the `Safe` version.
Encountering pentagons is rare in most use-cases, so this version
should usually be equivalent to the fast version, but with a guarantee
that it will not fail.

Initially, we **will not** expose the `Safe` versions to users in the API.
We may expose them in the future if a need becomes clear.


#### Distance

| Current name |      Proposed name      |
|--------------|-------------------------|
| `h3Distance` | `gridDistance`          |
| `h3Line`     | `gridPathCells`         |
| *DNE*        | `gridPathEdges`         |
| *DNE*        | `gridPathDirectedEdges` |


#### Filled-In Disk With Distances

|     Current name    |       Proposed name       |                 Calls                 |
|---------------------|---------------------------|---------------------------------------|
| `hexRangeDistances` | `gridDiskDistancesUnsafe` | NONE                                  |
| `_kRingInternal`    | `gridDiskDistancesSafe`   | NONE                                  |
| `kRingDistances`    | `gridDiskDistances`       | `hexRangeDistances`, `_kRingInternal` |


#### Filled-In Disk Without Distances

| Current name |   Proposed name   |        Calls        |
|--------------|-------------------|---------------------|
| `hexRange`   | `gridDiskUnsafe`  | `hexRangeDistances` |
| *DNE*        | `gridDiskSafe`    |                     |
| `kRing`      | `gridDisk`        | `kRingDistances`    |
| `hexRanges`  | `gridDisksUnsafe` | N x `hexRange`      |

- **Note**: We may remove `hexRanges` from the API, as it is just a very simple wrapper around
  `hexRange`. Inclusion is a discussion separate from this RFC. We'll simply state that
  *if we do* include it, we will rename it `gridDisksUnsafe`.


#### Hollow Ring

| Current name |  Proposed name   |              Calls               |
|--------------|------------------|----------------------------------|
| `hexRing`    | `gridRingUnsafe` | NONE                             |
| *DNE*        | `gridRingSafe`   | `gridDiskDistancesSafe`          |
| *DNE*        | `gridRing`       | `gridRingUnsafe`, `gridRingSafe` |

### H3 Edge Types

Instead of `UnidirectionalEdge`, use the term `DirectedEdge`.

For a future undirected edge mode, use the term `Edge`.

|                  Current name                 |        Proposed name         |
|-----------------------------------------------|------------------------------|
| `h3UnidirectionalEdgeIsValid`                 | `isValidDirectedEdge`        |
| `getH3UnidirectionalEdge`                     | `cellsToDirectedEdge`        |
| `getH3IndexesFromUnidirectionalEdge`          | `directedEdgeToCells`        |
| `getH3UnidirectionalEdgesFromHexagon`         | `originToDirectedEdges`      |
| *DNE*                                         | `destinationToDirectedEdges` |
| `getH3UnidirectionalEdgeBoundary`             | `directedEdgeToBoundary`     |
| `getOriginH3IndexFromUnidirectionalEdge`      | `getDirectedEdgeOrigin`      |
| `getDestinationH3IndexFromUnidirectionalEdge` | `getDirectedEdgeDestination` |


### Area/Length Functions

|  Current name  |        Proposed name        |
|----------------|-----------------------------|
| `hexAreaKm2`   | `getHexagonAreaAvgKm2`      |
| `hexAreaM2`    | `getHexagonAreaAvgM2`       |
| `edgeLengthKm` | `getHexagonEdgeLengthAvgKm` |
| `edgeLengthM`  | `getHexagonEdgeLengthAvgM`  |
| *DNE*          | `getPentagonAreaAvg*`       |
| *DNE*          | `getPentagonEdgeLengthAvg*` |
| *DNE*          | `cellAreaKm2`               |
| *DNE*          | `cellAreaM2`                |

**Note**: `cellAreaKm2` and `cellAreaM2` would return the actual area of
the passed-in cell.


## Polygons

**Note**: In addition to the changes listed in this section, we are considering
removing the `Linked*` data types from the public API. However, that is a larger
discussion requiring benchmarking, so we will defer that to a
[separate issue](https://github.com/uber/h3/issues/323) outside of this RFC.


### Data Structures

- rename `GeoBoundary` to `CellBoundary` to indicate it is space-limited to describing the geometry of cells

|    Current name   |   Proposed name   |               Notes               |
|-------------------|-------------------|-----------------------------------|
| `GeoBoundary`     | `CellBoundary`    | <= 10 stack-allocated `GeoPoint`s |
| `GeoCoord`        | `GeoPoint`        |                                   |
| `Geofence`        | `GeoLoop`         | heap-allocated `GeoPoint`s        |
| `GeoPolygon`      | `GeoPolygon`      |                                   |
| `GeoMultiPolygon` | `GeoMultiPolygon` | currently not used                |


### Functions

|            Current name           |      Proposed name       |         Notes          |
|-----------------------------------|--------------------------|------------------------|
| `h3ToGeoBoundary`                 | `cellToBoundary`         | returns `CellBoundary` |
| *DNE*                             | `cellToLoop`             | returns `GeoLoop`      |
| *DNE*                             | `loopToBoundary`         |                        |
| *DNE*                             | `boundaryToLoop`         |                        |
| `getH3UnidirectionalEdgeBoundary` | `directedEdgeToBoundary` | returns `CellBoundary` |
