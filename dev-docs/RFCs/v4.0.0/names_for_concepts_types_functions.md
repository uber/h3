# RFC: Names for H3 Concepts, Types, and Functions

- **Authors**: AJ Friend
- **Date**: 2020-02-02
- **Status**: Draft

## Motivation

Concepts like `hexagon`, `cell`, and `index` are currently used ambiguously in the H3 codebase and in documentation.
This can cause confusion, for example, when a function might only work on strict H3 *hexagons*, but fails when encountering H3 *pentagons*.

Reaching a consensus on the precise, technical language used when discussing H3 concepts will clarify future library discussions and improve end-user documentation.

## Proposals

## Concepts

The following technical terms should be used in the documentation, the H3 codebase, and precise technical discussions of the library.

- `H3Index`:
    - an unsigned 64-bit integer representing **any** H3 object (hexagon, pentagon, directed edge, ...)
    - often represented as a 15-character (or 16-character) hexadecimal string, like `'8928308280fffff'`
    - the full term "H3 index" should be used to avoid confusion with other common uses of "index"; when a "traditional" index is needed, prefer using "number", "pos", or another term to avoid confusion
- `Cell`:
    - an H3 index of `mode 1` (hexagon or pentagon)
    - for functions that can handle either hexagons or pentagons, the more general term "cell" should be used whenever possible
- hexagon:
    - an H3 cell which is a **topological** hexagon
    - todo: advise use of the term "strict" for functions that can only handle hexagons?
- pentagon:
    - an H3 cell which is a **topological** pentagon
- `DirectedEdge`
    - an H3 index of `mode 2`
    - representing a traversal from an origin cell to an adjacent destination cell
- grid:
    - the graph with nodes corresponding to H3 cells, and edges given by pairs of adjacent cells
    - for example, `gridDistance` is the minimal number of edges in a graph path connecting two cells

### Notes

We realize that "hexagon" will still be used informally to refer to the concept of "cell" (As the development team, we do it ourselves!).
This should be expected in casual, informal discussions of H3.
However, when **precision** is required, we advise the use of strict technical terms like "index", "cell", "hexagon", "pentagon", etc.
In the codebase and in the documentation, strictly correct terminology should **always** be used.


## Use of "Topological"

We should clarify in the documentation that H3 works with **topological** hexagons and pentagons, rather than true **geometric** hexagons and pentagons.

See, for example: https://github.com/uber/h3-js/issues/53


## General Function Names

|          Current name         |     Proposed name     |
|-------------------------------|-----------------------|
| *Does Not Exist (DNE)*        | `isValidIndex`        |
| `h3IsValid`                   | `isValidCell`         |
| `h3UnidirectionalEdgeIsValid` | `isValidDirectedEdge` |
| `h3IsPentagon`                | `isPentagon`          |
| `h3ToParent`                  | `cellParent`          |
| `h3ToChildren`                | `cellChildren`        |
| `h3IndexesAreNeighbors`       | `cellsAreNeighbors`   |
| `geoToH3`                     | `geoToCell`           |
| `h3ToGeo`                     | `cellToGeo`           |
| `numHexagons`                 | `numCells`            |
| `getRes0Indexes`              | `getRes0Cells`        |
| `getPentagonIndexes`          | `getPentagons`        |
| `h3GetBaseCell`               | `getBaseCellNumber`   |

### Naming note

- `isValid*` should mean that a full validity check is made
- without `Valid` (like in the case of `isPentagon`), we do not guarantee
  that a full validity check is made; instead, a user should assume only a
  minimal bit mask check is done
    - we could imagine a `isValidPengaton` function, if full verification
      would be convenient
- similarly, a function like `cellsAreNeighbors` will assume each cell
  has passed the `isValidCell` check; the function will do only minimal
  work to determine if they are neighbors

### todo
- Do we have general guidance on when to use a `get` prefix with a function and when not?


## H3 Grid Functions

| Current name |      Proposed name      |
|--------------|-------------------------|
| `h3Distance` | `gridDistance`          |
| `h3Line`     | `gridPathCells`         |
| *DNE*        | `gridPathEdges`         |
| *DNE*        | `gridPathDirectedEdges` |


## H3 Edge Types

Instead of `UnidirectionalEdge`, use the term `DirectedEdge`.

For a future undirected edge mode, use the term `Edge`.

Function name changes:

|                  Current name                 |        Proposed name         |
|-----------------------------------------------|------------------------------|
| `getH3UnidirectionalEdge`                     | `getDirectedEdge`            |
| `h3UnidirectionalEdgeIsValid`                 | `isValidDirectedEdge`        |
| `getOriginH3IndexFromUnidirectionalEdge`      | `getDirectedEdgeOrigin`      |
| `getDestinationH3IndexFromUnidirectionalEdge` | `getDirectedEdgeDestination` |
| `getH3IndexesFromUnidirectionalEdge`          | `getDirectedEdgeCells`       |
| `getH3UnidirectionalEdgesFromHexagon`         | `getDirectedEdgesFromCell`   |
| `getH3UnidirectionalEdgeBoundary`             | `getDirectedEdgeBoundary`    |


todo: do we want/need to use `get` to prefix all these functions?

## Area/Length Functions

|  Current name  |    Proposed name     |                   Notes                    |
|----------------|----------------------|--------------------------------------------|
| `hexAreaKm2`   | `hexAreaAvgKm2`      | todo: `hexAreaMaxKm2` and `hexAreaMinKm2`? |
| `hexAreaM2`    | `hexAreaAvgM2`       | todo: add min/max version?                 |
| `edgeLengthKm` | `hexEdgeLengthAvgKm` | todo: add min/max version?                 |
| `edgeLengthM`  | `hexEdgeLengthAvgM`  | todo: add min/max version?                 |
| *DNE*          | `pentagonAreaAvgKm2` | plus others                                |

todo: Could we add functions with signatures like `float cellAreaKm2(H3Index h)` to compute the actual area of a specific H3 cell?

## Cell Neighborhood Functions

| Current name |    Proposed names    |     Notes      |
|--------------|----------------------|----------------|
| `kRing`      | `disk` or `gridDisk` | filled-in disk |
| `hexRing`    | `ring` or `gridRing` | hollow ring    |


todo: For the `kRing`, `hexRange`, `hexRing`, etc. family of functions, should we come up with some standard prefix or suffix to denote that the function will fail if it encounters a pentagon?

I'm anticipating that, at least in the wrappers, we'd probably just expose users to the "works in all cases" version of the function.

