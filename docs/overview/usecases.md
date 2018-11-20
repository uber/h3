Why H3
---

Use cases
---

Analysis of data sets of locations, such as locations of cars in a city, may be done by bucketing locations. ([Sahr et al., 2003](http://webpages.sou.edu/~sahrk/sqspc/pubs/gdggs03.pdf)) There are several options for partitioning an area into buckets, such as manually drawing regions using human knowledge, or to partition the surface using a regular grid.

Manually drawn partitions can better incorporate human knowledge, but have a number of drawbacks, such as:
* The center of a partition may not represent the center of the data points.
* Edges of partitions may exhibit undesirable boundary effects.
* Manually defined partitions may require updating if our understanding of the system changes.
* Manually defining a large number of partitions may be very costly and time consuming.

Using a regular grid can avoid these drawbacks by providing smooth gradients and the ability to measure differences between cells. One defining characteristic of a grid system is the cell shape. There are only three polygons that tile regularly: the triangle, the square, and the hexagon. Of these, triangles and squares have neighbors with different distances. Triangles have three different distances, and squares have two different distances. For hexagons, all neighbors are equidistant. This property allows for simpler analysis of movement.

| Triangle | Square | Hexagon
| -------- | ------ | -------
| <img src="images/neighbors-triangle.png" style="width:400px"> | <img src="images/neighbors-square.png" style="width:400px"> | <img src="images/neighbors-hexagon.png" style="width:400px">
| Triangles have 12 neighbors | Squares have 8 neighbors | Hexagons have 6 neighbors

In addition to indexing locations to cells, **H3** provides a number of algorithms operating on indexes.

Hexagons have the property of expanding rings of neighbors (`kRing`) approximating circles:

<div align="center">
  <img src="images/neighbors.png" style="width:400px"><br>
  <i>All six neighbors of a hexagon (ring 1)</i>
</div>

Squares cleanly subdivide into four finer squares. Hexagons do not cleanly subdivide into seven finer hexagons. However, by alternating the orientation of grids a subdivision into seven cells (referred to as *aperture 7*) can be approximated. This makes it possible to truncate the precision (within a fixed margin of error) of an **H3** index using a few bitwise operations (`h3ToParent`). It is also possible to determine all the children of a parent **H3** index (`h3ToChildren`). Approximate containment only applies when truncating the precision of an **H3** index. The borders of hexagons indexed at a specific resolution are not approximate.

<div align="center">
  <img src="images/parent-child.png" style="width:400px"><br>
  <i>A parent hexagon approximately contains seven children</i>
</div>

Hierarchical containment allows for use cases like making contiguous sets of hexagons `compact`. It is then possible to `uncompact` to the same input set of hexagons.

| Uncompact (dense) | Compact (sparse)
| ----------------- | ----------------
| <img src="images/ca_uncompact_6_10633.png" style="width:500px"> | <img src="images/ca_compact_6_901.png" style="width:500px">
| California represented by 10633 uncompact hexagons | California represented by 901 compact hexagons

Comparisons
---

| System    | Index representation | Cell shape | Projection system
| --------- | -------------------- | ---------- | -----------------
| H3        | 64 bit integer       | hexagon    | Icosahedron face centered gnomonic
| [S2]      | 64 bit integer       | rectangle  | Cube face centered quadratic transform
| [Geohash] | alphanumeric string  | rectangle  | None, encodes latitude and longitude

[S2]: https://docs.google.com/presentation/d/1Hl4KapfAENAOf4gv-pSngKwvS_jwNVHRPZTTDzXXn6Q/edit
[Geohash]: https://en.wikipedia.org/wiki/Geohash
