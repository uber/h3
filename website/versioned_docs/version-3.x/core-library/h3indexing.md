---
id: h3Indexing
title: H3 Index Representations
sidebar_label: H3 Index Representations
slug: /core-library/h3Indexing
---

## Introduction

The H3 system assigns a unique hierarchical index to each cell. The H3 index of a resolution *r* cell begins with the appropriate resolution 0 base cell number. This is followed by a sequence of *r* digits 0-6, where each *i*<sup>th</sup> digit *d*<sub>i</sub> specifies one of the 7 cells centered on the cell indicated by the coarser resolution digits *d*<sub>1</sub> through *d*<sub>i-1</sub>. A local hexagon coordinate system is assigned to each of the resolution 0 base cells and is used to orient all hierarchical indexing child cells of that base cell. The assignment of digits 0-6 at each resolution uses a *Central Place Indexing* arrangement (see [Sahr, 2014](http://webpages.sou.edu/~sahrk/sqspc/pubs/autocarto14.pdf)). In the case of the 12 pentagonal cells the indexing hierarchy produced by sub-digit 1 is removed at all resolutions.

Child hexagons are linearly smaller than their parent hexagons.

<div align="center">
  <img height="300" src="/images/cpidigits.png" />
</div>


## H3Index Representation

An `H3Index` is the integer representation of an H3 index, which may be one of multiple modes to indicate the concept being indexed.

* Mode 0 is reserved and indicates an invalid H3 index.
* Mode 1 is an *H3 Cell* (Hexagon/Pentagon) index.
* Mode 2 is an *H3 Unidirectional Edge* (Cell A -> Cell B) index.
* Mode 3 is planned to be a bidirectional edge (Cell A <-> Cell B).
* Mode 4 is an *H3 Vertex* (i.e. a single vertex of an H3 Cell).

The canonical string representation of an `H3Index` is the hexadecimal representation of the integer, using lowercase letters. The string representation is variable length (no zero padding) and is not prefixed or suffixed.

### Invalid Index

Mode 0 contains a special index, `H3_NULL`, which is unique: it is bit-equivalent to `0`.
This index indicates, *specifically*, an invalid, missing, or uninitialized H3 index;
it is analogous to `NaN` in floating point.
It should be used instead of an arbitrary Mode 0 index, due to its uniqueness and easy identifiability.

### H3 Cell Index

An H3 Cell index (mode 1) represents a cell (hexagon or pentagon) in the H3 grid system at a particular resolution. The components of the H3 Cell index are packed into a 64-bit integer in order, highest bit first, as follows:

* 1 bit reserved and set to 0,
* 4 bits to indicate the H3 Cell index mode,
* 3 bits reserved and set to 0,
* 4 bits to indicate the cell resolution 0-15,
* 7 bits to indicate the base cell 0-121,
* 3 bits to indicate each subsequent digit 0-6 from resolution 1 up to the resolution of the cell (45 bits total are reserved for resolutions 1-15)

The three bits for each unused digit are set to 7.

### H3 Unidirectional Edge Index

An H3 Unidirectional Edge index (mode 2) represents a single directed edge between two cells (an "origin" cell and a neighboring "destination" cell). The components of the H3 Unidirectional Edge index are packed into a 64-bit integer in order, highest bit first, as follows:

* 1 bit reserved and set to 0,
* 4 bits to indicate the H3 Unidirectional Edge index mode,
* 3 bits to indicate the edge (1-6) of the origin cell,
* Subsequent bits matching the index bits of the origin cell.

### H3 Vertex Index

An H3 Vertex index (mode 4) represents a single topological vertex in H3 grid system, shared by three cells. Note that this does not include the distortion vertexes occasionally present in a cell's geo boundary. An H3 Vertex is arbitrarily assigned one of the three neighboring cells as its "owner", which is used to calculate the canonical index and geo coordinate for the vertex. The components of the H3 Vertex index are packed into a 64-bit integer in order, highest bit first, as follows:

* 1 bit reserved and set to 0,
* 4 bits to indicate the H3 Vertex index mode,
* 3 bits to indicate the vertex number (0-5) of vertex on the owner cell,
* Subsequent bits matching the index bits of the owner cell.

## Bit layout of H3Index

The layout of an `H3Index` is shown below in table form. The interpretation of the "Reserved" field differs depending on the mode of the index.

<table>
<tr>
  <th></th>
  <th>0x0F</th>
  <th>0x0E</th>
  <th>0x0D</th>
  <th>0x0C</th>
  <th>0x0B</th>
  <th>0x0A</th>
  <th>0x09</th>
  <th>0x08</th>
  <th>0x07</th>
  <th>0x06</th>
  <th>0x05</th>
  <th>0x04</th>
  <th>0x03</th>
  <th>0x02</th>
  <th>0x01</th>
  <th>0x00</th>
</tr>
<tr>
  <th>0x30</th>
  <td>Reserved</td>
  <td colspan="4">Mode</td>
  <td colspan="3">Mode-Dependent</td>
  <td colspan="4">Resolution</td>
  <td colspan="4">Base cell</td>
</tr>
<tr>
  <th>0x20</th>
  <td colspan="3">Base cell</td>
  <td colspan="3">Digit 1</td>
  <td colspan="3">Digit 2</td>
  <td colspan="3">Digit 3</td>
  <td colspan="3">Digit 4</td>
  <td>Digit 5</td>
</tr>
<tr>
  <th>0x10</th>
  <td colspan="2">Digit 5</td>
  <td colspan="3">Digit 6</td>
  <td colspan="3">Digit 7</td>
  <td colspan="3">Digit 8</td>
  <td colspan="3">Digit 9</td>
  <td colspan="2">Digit 10</td>
</tr>
<tr>
  <th>0x00</th>
  <td>Digit 10</td>
  <td colspan="3">Digit 11</td>
  <td colspan="3">Digit 12</td>
  <td colspan="3">Digit 13</td>
  <td colspan="3">Digit 14</td>
  <td colspan="3">Digit 15</td>
</tr>
</table>

## Links

* Observable notebook example: [H3 Index Bit Layout](https://observablehq.com/@nrabinowitz/h3-index-bit-layout?collection=@nrabinowitz/h3)
* Observable notebook example: [H3 Index Inspector](https://observablehq.com/@nrabinowitz/h3-index-inspector?collection=@nrabinowitz/h3)
