Generate the cell boundary in latitude/longitude coordinates of an H3Index cell
---

This operation is performed by function `h3ToGeoBoundary`. See the comments in the function source code for more detail.

The conversion is performed as a series of coordinate system conversions described below. See the page <a href="#/documentation/core-library/coordinate-systems">Coordinate Systems used by the **H3 Core Library**</a> for more information on each of these coordinate systems.

* We note that the cell vertices are the center points of cells in an aperture 3 grid one resolution finer than the cell resolution, which we term a *substrate* grid. We precalculate the substrate *ijk* coordinates of a cell with *ijk* coordinates (0,0,0), adding additional aperture 3 and aperture 7 (if required, by Class III cell grid) substrate grid resolutions as required to transform the vertex coordinates into a Class II substrate grid.

<div align="center">
  <img height="300" src="images/substrate3.png" />
</div>

* The function `_faceIjkToGeoBoundary` calculates the *ijk* coordinates of the cell center point in the appropriate substrate grid (determined in the last step), and each of the substrate vertices is translated using the cell center point *ijk*. Each vertex *ijk* is then transformed onto the appropriate face and *Hex2d* coordinate system using the approach taken in <a href="#/documentation/core-library/walkthrough-of-h3togeo">finding a cell center point</a>. If adjacent vertices lie on different icosahedron faces a point is introduced at the intersection of the cell edge and icosahedron face edge.
* The *Hex2d* coordinates are then converted to latitude/longitude using `_hex2dToGeo`.
