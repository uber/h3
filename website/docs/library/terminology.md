---
id: terminology
title: Terminology
sidebar_label: Terminology
slug: /library/terminology
---

The following are technical terms used by H3.

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
- **lat/lng point**:
    - a representation of a geographic point in terms of a latitude/longitude pair
    - when abbreviating, we use "lng" (instead of "lon") for longitude
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
- `H3_NULL`:
    - equivalent to `0` and guaranteed to never be a valid `H3Index` (even after any future H3 **modes** are added)
    - returned by functions to denote an error, or to denote missing data in arrays of `H3Index`
    - analogous to `NaN` in floating point


### Use of "hex", "hexagon", "cell", "pentagon", etc.

We realize that "hex" or "hexagon" will still be used informally to refer to the concept of "cell" (As the development team, we do it ourselves!).
This should be expected in casual, informal discussions of H3.
However, when *precision* is required, we advise the use of strict technical terms like "index", "cell", "hexagon", "pentagon", etc.
In the codebase and in the documentation, strictly correct terminology should *always* be used, as many functions and algorithms distinguish between hexagons and pentagons.

## References

* [Technical RFC for naming concepts](https://github.com/uber/h3/blob/master/dev-docs/RFCs/v4.0.0/names_for_concepts_types_functions.md)
