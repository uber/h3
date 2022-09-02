---
id: h3Indexing
title: H3 Index
sidebar_label: H3 Index
slug: /core-library/h3Indexing
---

## Introduction

The H3 system assigns a unique hierarchical index to each cell. Each directed edge and vertex is assigned an index based on its origin or owner cell, respectively.

## H3Index Representation

An `H3Index` is the 64-bit integer representation of an H3 index, which may be one of multiple modes to indicate the concept being indexed.

* Mode 0 is reserved and indicates an [invalid H3 index](#invalid-index).
* Mode 1 is an *[H3 Cell](../library/index/cell)* (Hexagon/Pentagon) index.
* Mode 2 is an *[H3 Directed Edge](../library/index/directededge)* (Cell A -> Cell B) index.
* Mode 3 is planned to be a bidirectional edge (Cell A <-> Cell B).
* Mode 4 is an *[H3 Vertex](../library/index/vertex)* (i.e. a single vertex of an H3 Cell).

The canonical string representation of an `H3Index` is the hexadecimal representation of the integer, using lowercase letters. The string representation is variable length (no zero padding) and is not prefixed or suffixed.

### Invalid Index

Mode 0 contains a special index, `H3_NULL`, which is unique: it is bit-equivalent to `0`.
This index indicates, *specifically*, an invalid, missing, or uninitialized H3 index;
it is analogous to `NaN` in floating point.
It should be used instead of an arbitrary Mode 0 index, due to its uniqueness and easy identifiability.
A mode 0 index could also indicate an [error code](../library/errors) has been provided instead of a valid H3 index.

## Bit layout of H3Index

The layout of an `H3Index` is shown below in table form. The interpretation of the "Mode-Dependent" field differs depending on the mode of the index.

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
