# RFC: Names for H3 Concepts, Types, and Functions

- **Authors**: AJ Friend
- **Date**: 2020-02-02
- **Status**: Draft

## Motivation

Concepts like `hexagon`, `cell`, and `index` are currently used ambiguously
in the H3 codebase and in documentation. This can cause confusion, for example,
when a function might only work on strict H3 *hexagons*, but fails when
encountering H3 *pentagons*.

Reaching a consensus on the precise, technical language used when discussing H3 concepts will clarify future library discussions and improve end-user documentation.

## Proposals

## Concepts

The following technical terms should be used in the documentation, the H3 codebase, and precise technical discussions of the library.

- `Index` or `H3Index`:
    - an unsigned 64-bit integer representing **any** H3 object (hexagon, pentagon, directed edge, ...)
    - often represented as a 15-character (or 16-character) hexadecimal string, like `'8928308280fffff'`
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

### Notes

We realize that "hexagon" will still be used informally to refer to the concept of "cell" (As the development team, we do it ourselves!). This should be expected in casual, informal discussions of H3. When **precision** is required, however, we advise the use of strict technical terms like "index", "cell", "hexagon", "pentagon", etc.


## Use of "Topological"

We should clarify in the documentation that H3 works with **topological** hexagons and pentagons, rather than true **geometric** hexagons and pentagons.

See, for example: https://github.com/uber/h3-js/issues/53


## Names for H3 Edge Types

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


## Function Names

A few more function name change proposals:

|          Current name         |     Proposed name     |                  Notes                  |
|-------------------------------|-----------------------|-----------------------------------------|
| (doesn't exist)               | `isValidIndex`        |                                         |
| `h3IsValid`                   | `isValidCell`         |                                         |
| `h3UnidirectionalEdgeIsValid` | `isValidDirectedEdge` |                                         |
| `h3IsPentagon`                | `isPentagon`          |                                         |
| `h3ToParent`                  | `cellToParent`        |                                         |
| `h3ToChildren`                | `cellToChildren`      |                                         |
| `h3IndexesAreNeighbors`       | `cellsAreNeighbors`   |                                         |
| `geoToH3`                     | `geoToCell`           |                                         |
| `h3ToGeo`                     | `cellToGeo`           |                                         |
| `kRing`                       | `disk`                | filled-in disk                          |
| `hexRing`                     | `ring`                | hollow ring                             |
| `numHexagons`                 | `numCells`            |                                         |
| `getRes0Indexes`              | `getRes0Cells`        |                                         |
| `getPentagonIndexes`          | `getPentagons`        |                                         |
| `h3GetBaseCell`               | `getBaseCellNumber`   |                                         |
| `h3Distance`                  | `gridDistance`        | no `get` prefix?                        |
| `h3Line`                      | `cellPath`            | future: `edgePath`, `directedEdgePath`? |


For the `kRing`, `hexRange`, `hexRing`, etc. family of functions, should we come up with some standard prefix or suffix to denote that the function will fail if it encounters a pentagon?
I'm anticipating that, at least in the wrappers, we'd probably just expose users to the "works in all cases" version of the function.


