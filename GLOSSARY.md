# h3d

h3d extends the H3 hierarchical geospatial index with altitude, so that volumes of space from near the Earth's surface out to geostationary orbit can be indexed alongside the existing 2D H3 cells, which remain unchanged.

## Language

### Existing H3 terms

**Cell**:
An existing 2D H3 cell (hexagon or pentagon) at some resolution, exactly as defined by upstream H3.
_Avoid_: Hex, hexagon (when pentagons are included), tile

**Resolution**:
The level of an H3 cell in the hierarchy, from 0 (coarsest) to 15 (finest); a voxel shares the resolution of its footprint.

### Altitude

**Altitude**:
Height above the WGS84 ellipsoid; the single vertical coordinate on which layers and voxels are defined.
_Avoid_: Elevation, height (unqualified)

**Altitude type**:
The reference an input altitude is expressed against (for example ellipsoidal, mean sea level, or above ground), which must be converted to altitude before locating a voxel.
_Avoid_: Datum, height mode

**Altitude range**:
The span of altitudes in which voxels exist at a given resolution. Resolutions 0–10 share the full range (about −1,780 km to +443,000 km); resolutions 11–13 each have a narrower range starting near −600 m and made of whole layers of the next coarser resolution; resolutions 14–15 have no voxels. Altitude zero is always a layer boundary.
_Avoid_: Extent, bounds, domain, band

### Volumes

**Layer**:
A span of altitudes between a lower and upper bound at one resolution.
_Avoid_: Slice, shell, level

**Voxel**:
The volume of space whose footprint is one cell and whose altitudes fall within one layer of the same resolution.
_Avoid_: 3D cell, frustum, slice, prism

**Footprint**:
The cell a voxel or column lies over; a column's cross-section at altitude zero is exactly this cell.
_Avoid_: Base cell (that is an H3 term for resolution-0 cells)

**Column**:
All voxels of one resolution over a single footprint, stacked from the bottom of the covered altitude range to the top.
_Avoid_: Stack, tower, frustum

**Layer number**:
A signed count of layers from altitude zero at one resolution: layer 0 is the first layer above zero, layer −1 the first below. A layer includes its lower bound and excludes its upper bound.
_Avoid_: Level, floor, altitude index

**Voxel disk**:
All voxels whose footprints lie within k grid steps of a voxel's footprint and whose layer numbers lie within k of its layer number; the 3D counterpart of an H3 grid disk.
_Avoid_: Sphere, ball, neighbourhood

**Face neighbour**:
A voxel sharing a wall, floor or ceiling with another voxel of the same resolution: up to six beside it, one above and one below.
_Avoid_: Adjacent voxel (ambiguous with the voxel disk)

**Parent layer** / **Child layer**:
A layer at one resolution and the layers at the next finer resolution that exactly subdivide it; layer boundaries at a coarser resolution are always boundaries at every finer one.
_Avoid_: Sub-layer, nested slice
