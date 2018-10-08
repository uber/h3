Coordinate Systems used by the H3 Core Library
---

IJK Coordinates
---

Discrete hexagon planar grid systems naturally have 3 coordinate axes spaced 120&deg; apart. We refer to such a system as an *ijk coordinate system*, for the three coordinate axes *i*, *j*, and *k*. A single *ijk* coordinate triplet is represented in the **H3 Core Library** using the structure type `CoordIJK`.

Using an *ijk* coordinate system to address hexagon grid cells provides multiple valid addresses for each cell. *Normalizing* an *ijk* address (function `_ijkNormalize`) creates a unique address consisting of the minimal positive *ijk* components; this always results in at most two non-zero components.

<div align="center">
  <img height="300" src="images/ijkp.png" />
</div>

FaceIJK Coordinates
---

The **H3 Core Library** centers an *ijk* coordinate system on each face of the icosahedron; the combination of a face number and *ijk* coordinates on that face's coordinate system is represented using the structure type `FaceIJK`.

Each grid resolution is rotated ~19.1&deg; relative to the next coarser resolution. The rotation alternates between counterclockwise and clockwise at each successive resolution, so that each resolution will have one of two possible orientations: *Class II* or *Class III* (using a terminology coined by R. Buckminster Fuller). The base cells, which make up resolution 0, are *Class II*.

<div align="center">
  <img height="300" src="images/classII.III.png" />
</div>

Hex2d Coordinates
---

A *Hex2d* coordinate system is a cartesian coordinate system associated with a specific *ijk* coordinate system, where:

* the origin of the *Hex2d* system is centered on the origin cell of the *ijk* system, 
* the positive *x*-axis of the *Hex2d* system is aligned with the *i*-axis of the *ijk* system, and
* 1.0 unit distance in the *Hex2d* system is the distance between adjacent cell centers in the *ijk* coordinate system.

*Hex2d* coordinates are represented using the structure type `Vec2d`.

Local IJ Coordinates
---

Algorithms working with hexagons may want to refer to grid coordinates that are not interrupted by base cells or faces. These coordinates have 2 coordinate axes spaced 120&deg; apart, with the coordinates anchored by an *origin* H3 index.

* local coordinates are only comparable when they have the same *origin* index.
* local coordinates are only valid near the *origin*. Pratically, this is within the same base cell or a neighboring base cell, except for pentagons.
* the coordinate space may have deleted or warped regions due to pentagon distortion.
* there may be multiple coordinates for the same index, with the same *origin*.
* the *origin* may not be at `(0, 0)` in the local coordinate space.

*Local IJ* coordinates are represented using the structure type `CoordIJ` and an associated *origin* `H3Index`.
