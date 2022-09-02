---
id: vertex
title: Vertex mode
sidebar_label: Vertex mode
slug: /library/index/vertex
---

<div align="center">
  <img height="300" src="/images/vertex_mode.png" />
</div>

An H3 Vertex index (mode 4) represents a single topological vertex in H3 grid system, shared by three cells. Note that this does not include the distortion vertexes occasionally present in a cell's geographic boundary. An H3 Vertex is arbitrarily assigned one of the three neighboring cells as its "owner", which is used to calculate the canonical index and geographic coordinates for the vertex. The components of the H3 Vertex index are packed into a 64-bit integer in order, highest bit first, as follows:

* 1 bit reserved and set to 0,
* 4 bits to indicate the H3 Vertex index mode,
* 3 bits to indicate the vertex number (0-5) of vertex on the owner cell,
* Subsequent bits matching the index bits of the [owner cell](./cell#h3-cell-index).

## Bit layout of H3Index for vertexes

The layout of an `H3Index` for vertexes is shown below in table form. The interpretation of the "Mode-Dependent" field differs depending on the mode of the index.

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
  <td>Reserved (0)</td>
  <td colspan="4">Mode (4)</td>
  <td colspan="3">Edge</td>
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
