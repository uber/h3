# RFC: Terms for H3 Concepts and Data Types

- **Authors**: AJ Friend
- **Date**: 2020-02-02
- **Status**: Work In Progress

## Motivation

Concepts like `hexagon`, `cell`, and `index` are currently used ambiguously
in the H3 codebase and in documentation. This can cause confusion, for example,
when a function might only work on strict H3 *hexagons*, but fails when
encountering H3 *pentagons*.

Reaching a consensus on the precise, technical language used when discussing H3 concepts will clarify future library discussions and improve end-user documentation.

## Proposals

## Concepts

The following technical terms should be used in the documentation, the H3 codebase, and precise technical discussions of the library.

- `H3Index`, "H3 index", or "index"
    - an unsigned 64-bit integer representing **any** H3 object (hexagon, pengaton, unidirectional edge, ...)
    - often represented as a 15-character (or 16-character) hexadecimal string, like `'8928308280fffff'`
- cell
    - an H3 index of `mode` 1 (hexagon or pentagon)
    - for functions that can handle either hexagons or pentagons, the more general term "cell" should be used whenever possible
- hexagon
    - an H3 cell which is a **topological** hexagon
    - todo: advise use of the term "strict" for functions that can only handle hexagons?
- pentagon
    - an H3 cell which is a **topological** pentagon

### Notes

We realize that "hexagon" will still be used informally to refer to the concept of "cell" (As the development team, we do it ourselves!). This should be expected in casual, informal discussions of H3. When **precision** is required, however, we advise the use of strict technical terms like "index", "cell", "hexagon", "pentagon", etc.

## Use of "Topological"

We should clarify in the documentation that H3 works with **topological** hexagons and pentagons, rather than true **geometric** hexagons and pentagons.

See, for example: https://github.com/uber/h3-js/issues/53

## Names for H3 Edge Types

Instead of `UnidirectionalEdge`, use the term `DirectedEdge` or `DiEdge`.

For a future undirected edge mode, use the term `UndirectedEdge` or `Edge`.

Function name changes:

|                  Current name                 |          Proposed name           |
|-----------------------------------------------|----------------------------------|
| `getH3UnidirectionalEdge`                     | `getH3DiEdge`                    |
| `h3UnidirectionalEdgeIsValid`                 | `h3DiEdgeIsValid`                |
| `getOriginH3IndexFromUnidirectionalEdge`      | `getOriginH3CellFromDiEdge`      |
| `getDestinationH3IndexFromUnidirectionalEdge` | `getDestinationH3CellFromDiEdge` |
| `getH3IndexesFromUnidirectionalEdge`          | `getH3CellsFromDiEdge`           |
| `getH3UnidirectionalEdgesFromHexagon`         | `getH3DiEdgesFromCell`           |
| `getH3UnidirectionalEdgeBoundary`             | `getH3DiEdgeBoundary`            |
