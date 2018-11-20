Conversion from latitude/longitude to containing H3 cell index
---

This operation is performed by function `geoToH3`. See the comments in the function for more detail.

The conversion is performed as a series of coordinate system conversions described below. See the page <a href="#/documentation/core-library/coordinate-systems">Coordinate Systems used by the **H3 Core Library**</a> for more information on each of these coordinate systems.

1. The input latitude/longitude coordinate is first converted into the containing icosahedron face and a *Hex2d* coordinate on that face using function `_geoToHex2d`, which determines the correct face and then performs a face-centered gnomonic projection into face-centered polar coordinates. These polar coordinates are then scaled appropriately to a *Hex2d* coordinate on the input grid resolution *r*.
2. The *Hex2d* coordinate is converted into resolution *r* normalized *ijk* coordinates using function `_hex2dToCoordIJK`.
3. The face and face-centered *ijk* coordinates are then converted into an `H3Index` representation using the following steps:

   * the **H3** index digits are calculated from resolution *r* up to 0, adjusting the *ijk* coordinates at each successively coarser resolution.
   * when resolution 0 is reached, if the remaining *ijk* coordinates are (0,0,0) then the base cell centered on the face is chosen for the index
   * if the remaining resolution 0 *ijk* coordinates are not (0,0,0), then a lookup operation is performed to find the appropriate base cell and the required rotation (if any) to orient the cell in that base cell's coordinate system. The index is then translated and rotated into the coordinate system centered on the new base cell.
