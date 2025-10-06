---
id: overview
title: Overview of the H3 Geospatial Indexing System
sidebar_label: Overview
slug: /core-library/overview
---

The H3 geospatial indexing system is a discrete global grid system (see [Sahr et al., 2003](https://www.discreteglobalgrids.org/wp-content/uploads/2016/01/gdggs03.pdf)) consisting of a multi-precision hexagonal tiling of the sphere with hierarchical indexes.

The hexagonal grid system is created on the planar faces of a sphere-circumscribed icosahedron, and the grid cells are then projected to the surface of the sphere using an inverse face-centered polyhedral gnomonic projection. The coordinate reference system (CRS) is spherical coordinates with the [WGS84](https://en.wikipedia.org/wiki/WGS84)/[EPSG:4326](https://epsg.io/4326) authalic radius. It is common to use WGS84 CRS data with the H3 library.

The icosahedron is fixed relative to the sphere using a *Dymaxion* orientation (due to R. Buckminster Fuller). This orientation of a spherical icosahedron places all 12 icosahedron vertices in the ocean. (At the time of H3's development, this was the only known orientation with this property. [Others have since been found](https://richard.science/sci/2019_barnes_dgg_published.pdf).)

The H3 grid is constructed on the icosahedron by recursively creating increasingly higher precision hexagon grids until the desired resolution is achieved. Note that it is impossible to tile the sphere/icosahedron completely with hexagons; each resolution of an icosahedral hexagon grid must contain exactly 12 pentagons at every resolution, with one pentagon centered on each of the icosahedron vertices.

The first H3 resolution (resolution 0) consists of 122 cells (110 hexagons and 12 icosahedron vertex-centered pentagons), referred to as the *base cells*. These were chosen to capture as much of the symmetry of the spherical icosahedron as possible. These base cells are assigned numbers from 0 to 121 based on the latitude of their center points; base cell 0 has the northern most center point, while base cell 121 has the southern most center point.

Each subsequent resolution beyond resolution 0 is created using an aperture 7 resolution spacing (aperture refers to the number of cells in the next finer resolution grid for each cell); as resolution increases the unit length is scaled by `sqrt(7)` and each hexagon has 1/7<sup>th</sup> the area of a hexagon at the next coarser resolution (as measured on the icosahedron). H3 provides 15 finer grid resolutions in addition to the resolution 0 base cells. The finest resolution, resolution 15, has cells with an area of less than 1 m<sup>2</sup>. A table detailing the average cell area for each H3 resolution is available [here](/docs/core-library/restable).

*Note:* you can create KML files to visualize the H3 grids by running the `kml` make target. It will place the files in the `KML` output sub-directory.
