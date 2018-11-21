Overview of the H3 Geospatial Indexing System
---

The **H3** geospatial indexing system is a discrete global grid system (see [Sahr et al., 2003](http://webpages.sou.edu/~sahrk/sqspc/pubs/gdggs03.pdf)) consisting of a multi-precision hexagonal tiling of the sphere with hierarchical indexes. The hexagonal grid system is created on the planar faces of a sphere-circumscribed icosahedron, and the grid cells are then projected to the surface of the sphere using an inverse face-centered polyhedral gnomonic projection.

The icosahedron is fixed relative to the sphere using a *Dymaxion* orientation (due to R. Buckminster Fuller). This is the only known orientation of a spherical icosahedron that places all 12 icosahedron vertices in the ocean.

The **H3** grid is constructed on the icosahedron by recursively creating increasingly higher precision hexagon grids until the desired resolution is achieved. Note that it is impossible to tile the sphere/icosahedron completely with hexagons; each resolution of an icosahedral hexagon grid must contain exactly 12 pentagons at every resolution, with one pentagon centered on each of the icosahedron vertices.

The first **H3** resolution (resolution 0) consists of 122 cells (110 hexagons and 12 icosahedron vertex-centered pentagons), referred to as the *base cells*. These were chosen to capture as much of the symmetry of the spherical icosahedron as possible. These base cells are assigned numbers from 0 to 121 based on the latitude of their center points; base cell 0 has the northern most center point, while base cell 121 has the southern most center point.

Each subsequent resolution beyond resolution 0 is created using an aperture 7 resolution spacing (aperture refers to the number of cells in the next finer resolution grid for each cell); as resolution increases the unit length is scaled by \\(\sqrt{7}\\) and each hexagon has \\(1/7th\\) the area of a hexagon at the next coarser resolution (as measured on the icosahedron). **H3** provides 15 finer grid resolutions in addition to the resolution 0 base cells. The finest resolution, resolution 15, has cells with an area of less than 1 \\(m^2\\). A table detailing the average cell area for each H3 resolution is available <a href="#/documentation/core-library/resolution-table">here</a>.

*Note:* you can create KML files to visualize the **H3** grids by running the `kml` make target. It will place the files in the `KML` output sub-directory.
