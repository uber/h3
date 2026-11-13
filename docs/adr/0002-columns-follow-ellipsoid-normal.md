# Columns rise along the WGS84 ellipsoid normal, not radially from the Earth's centre

A voxel is the set of points whose WGS84 geodetic lat/lng falls in its footprint cell and whose ellipsoidal altitude falls in its layer, so column walls follow the ellipsoid normal. We chose this over radial walls (pointing at the Earth's centre) because geodetic and geocentric latitude differ by up to 0.19° (~21 km on the ground): with radial walls a point directly overhead in the GPS sense could land in a different footprint than the ground below it, and the footprint would no longer equal the cell `latLngToCell` already returns for that lat/lng. Every point in space (outside a region within ~40 km of the Earth's centre) has exactly one geodetic coordinate, so voxels still tile space with no gaps or overlaps.

## Consequences

- Column walls are slightly twisted ruled surfaces, not planes; a column is only approximately a frustum.
- A column's cross-section equals its footprint cell exactly at altitude zero, and is slightly scaled above and below it.
