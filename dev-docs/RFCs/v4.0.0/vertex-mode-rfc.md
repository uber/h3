# RFC: Vertex mode

* **Authors**: Nick Rabinowitz
* **Date**: -
* **Status**: Draft

## Abstract

This proposal would add a new H3 index mode, Vertex Mode, to provide a canonical id for a vertex in a cell boundary. Each vertex shared by 3 adjacent cells would be represented by a single index.

## Motivation

The advantages of this approach:

* Shared vertices can be compared between hexagons with a strict equality check
* Vertices can be represented and stored without the trig calls required to produce a lat/lon point
* Vertex to lat/lon can be guaranteed to return the same result across different cells that share the vertex

At a minimum, this should open up significant performance and correctness gains in `h3SetToLinkedGeo` and `directedEdgeToBoundary`, and there are likely other uses as well.

## Approaches

The proposed approach below is fairly straightforward. The main questions are:

* Whether to have one vertex mode, `H3_VERTEX_MODE`, representing the 5 or 6 topological vertices of each cell, or whether to additionally offer a second vertex mode, `H3_ICOSAEDGE_VERTEX_MODE`, to represent distortion vertices shared by two cells whose edge crosses an edge of the icosahedron. 

* If we have two different modes, which functions return both types, in order, and which functions return only one type.

These two modes are largely orthogonal, and it would be possible to implement only `H3_VERTEX_MODE` first without considering `H3_ICOSAEDGE_VERTEX_MODE`. This simplifies the implementation and keeps the new mode wholly in the topological space, without considering the geometric issues posed by the distortion vertices. However this may make it harder or more expensive to implement transformations from sets of vertices to the geometry of a polygon or edge.

## Proposal

### Format of the index

* The H3 index of the "owner" cell. The owner would be one of the cells that share the vertex (see below).
* The mode set to `H3_VERTEX_MODE`
* The reserved bits used to identify the index (0-6) of the vertex on the owner cell, using the index order used by `cellToBoundary`

### Choice of owner

Proposed simple algorithm for determining ownership:

* Find all three neighbors that share a vertex
* Use the one with the numerically lowest index as the owner

### New library functions

* `void getCellVertexes(H3Index cell, H3Vertex *vertices)`
* `H3Vertex getCellVertex(H3Index cell, int vertIndex)`
* `GeoCoord vertexToGeoPoint(H3Vertex vertex)`
