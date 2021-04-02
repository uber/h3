---
id: hexbin
title: Hexbin
sidebar_label: Hexbin
slug: /comparisons/hexbin
---

Hexbinning is taking coordinates and binning them into hexagonal cells in analytics or mapping software. The size of the hexagons is configurable, and the hexagons can align with the map projection being used.

Hexbins are generally very computationally cheap to create, and for the best performance can be computed directly on a GPU. Their coordinates, while not hierarchical, support many common operations like finding neighbors and grid distances very efficiently.

Hexbins have drawbacks in the ability to reuse the grids. The cell identifiers they use are only useful in the specific hexbin grid, and are not portable to another grid, limiting their ability to be used for joining datasets. The cell identifiers are not hierarchical, so relating data at different grid resolutions can be difficult.

Hexbins are also limited by the projection system they are created on top of. This usually results in discontinuities at the edges of the projections, for example at the anti-meridian or at the poles.
