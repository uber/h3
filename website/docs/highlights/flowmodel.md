---
id: flowmodel
title: Flow Modelling
sidebar_label: Flow Modelling
slug: /highlights/flowmodel
---

H3's hexagonal grid is well suited to analyzing movement. In addition to the benefits of the hexagonal grid shape, H3 includes other features for modelling flow.

H3 can create indexes that refer to the movement from one cell to a neighbor. These directed edge indexes share the advantages with their cell index counterparts, such as being 64 bit integers. The use of directed edges makes it possible to associate a weight with a movement in the grid.

(A planned improvement for the H3 system is to create indexes that refer to the edge between two cells without regards of direction.)

## Links

* Observable notebook example: [H3 Travel Times](https://observablehq.com/@nrabinowitz/h3-travel-times)
