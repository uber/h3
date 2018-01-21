H3 Index Representations
---

Introduction
---

The __H3__ system assigns a unique hierarchical index to each cell. The __H3__ index of a resolution _r_ cell begins with the appropriate resolution 0 base cell number. This is followed by a sequence of _r_ digits 0-6, where each _i_<sup>th</sup> digit _d_<sub>i</sub> specifies one of the 7 cells centered on the cell indicated by the coarser resolution digits _d_<sub>1</sub> through _d_<sub>i-1</sub>. A local hexagon coordinate system is assigned to each of the resolution 0 base cells and is used to orient all hierarchical indexing child cells of that base cell. The assignment of digits 0-6 at each resolution uses a _Central Place Indexing_ arrangement (see [Sahr, 2014](http://webpages.sou.edu/~sahrk/sqspc/pubs/autocarto14.pdf)). In the case of the 12 pentagonal cells the indexing hierarchy produced by sub-digit 1 is removed at all resolutions.

Child hexagons are linearly smaller than their parent hexagons.

\image html cpidigits.png


H3Index Representation
---

The __H3Index__ is the integer representation of an __H3__ index, which can be placed into multiple modes to indicate the kind of concept being indexed. Mode 1 is an __H3__ Cell (Hexagon) Index, mode 2 is an __H3__ Unidirectional Edge (Hexagon A -> Hexagon B) Index, mode 3 is planned to be a bidirectional edge (Hexagon A <-> Hexagon B). Mode 0 is reserved and indicates an invalid __H3__ index.

The components of the __H3__ cell index (mode 1) are packed into the lowest order 63 bits of a 64-bit integer in order as follows:

* 4 bits to indicate the index mode,
* 3 bits reserved,
* 4 bits to indicate the cell resolution 0-15,
* 7 bits to indicate the base cell 0-121, and
* 3 bits to indicate each subsequent digit 0-6 from resolution 1 up to the resolution of the cell (45 bits total is reserved for resolutions 1-15)

The components of the __H3__ unidirectional edge index (mode 2) are packed into the lowest order 63 bits of a 64-bit integer in order as follows:

* 4 bits to indicate the index mode,
* 3 bits to indicate the edge 1-6 of the cell to traverse,
* 4 bits to indicate the cell resolution 0-15,
* 7 bits to indicate the base cell 0-121, and
* 3 bits to indicate each subsequent digit 0-6 from resolution 1 up to the resolution of the cell (45 bits total is reserved for resolutions 1-15)

The canonical string representation of an __H3Index__ is the hexadecimal representation of the integer.

The three bits for each unused digit are set to 7.

Bit layout of H3Index
---

The layout of an __H3Index__ is shown below in table form. The interpretation of the "Reserved/edge" field differs depending on the mode of the index.

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
  <td></td>
  <td colspan="4">Mode</td>
  <td colspan="3">Reserved/edge</td>
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
