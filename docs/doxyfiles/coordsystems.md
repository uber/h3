Coordinate Systems used by the H3 Core Library
---

IJK Coordinates
---

Discrete hexagon planar grid systems naturally have 3 coordinate axes spaced 120&deg; apart. We refer to such a system as an _ijk coordinate system_, for the three coordinate axes _i_, _j_, and _k_. A single _ijk_ coordinate triplet is represented in the __H3 Core Library__ using the structure type CoordIJK.

Using an _ijk_ coordinate system to address hexagon grid cells provides multiple valid addresses for each cell. _Normalizing_ an _ijk_ address (function ::\_ijkNormalize) creates a unique address consisting of the minimal positive _ijk_ components; this always results in at most two non-zero components.

\image html ijkp.png

FaceIJK Coordinates
---

The __H3 Core Library__ centers an _ijk_ coordinate system on each face of the icosahedron; the combination of a face number and _ijk_ coordinates on that face's coordinate system is represented using the structure type FaceIJK.

Each grid resolution is rotated ~19.1&deg; relative to the next coarser resolution. The rotation alternates between counterclockwise and clockwise at each successive resolution, so that each resolution will have one of two possible orientations: _Class II_ or _Class III_ (using a terminology coined by R. Buckminster Fuller). The base cells, which make up resolution 0, are _Class II_.

\image html classII.III.png

Hex2d Coordinates
---

A _Hex2d_ coordinate system is a cartesian coordinate system associated with a specific _ijk_ coordinate system, where:

* the origin of the _Hex2d_ system is centered on the origin cell of the _ijk_ system, 
* the positive _x_-axis of the _Hex2d_ system is aligned with the _i_-axis of the _ijk_ system, and
* 1.0 unit distance in the _Hex2d_ system is the distance between adjacent cell centers in the _ijk_ coordinate system.

_Hex2d_ coordinates are represented using the structure type Vec2d.

