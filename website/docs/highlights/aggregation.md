---
id: aggregation
title: Aggregation
sidebar_label: Aggregation
slug: /highlights/aggregation
---

Analysis of location data, such as locations of cars in a city, can be done by bucketing locations. ([Sahr et al., 2003](http://webpages.sou.edu/~sahrk/sqspc/pubs/gdggs03.pdf)) Using a regular grid provides smooth gradients and the ability to measure differences between cells.

The cell shape of that grid system is an important consideration. For simplicity, it should be a polygon that tiles regularly: the triangle, the square, or the hexagon. Of these, triangles and squares have neighbors with different distances. Triangles have three different distances, and squares have two different distances. For hexagons, all neighbors are equidistant.

| Triangle | Square | Hexagon
| -------- | ------ | -------
| <img src="/images/neighbors-triangle.png" style={{width:'400px'}} /> | <img src="/images/neighbors-square.png" style={{width:'400px'}} /> | <img src="/images/neighbors-hexagon.png" style={{width:'400px'}} />
| Triangles have 12 neighbors | Squares have 8 neighbors | Hexagons have 6 neighbors

This property allows for simpler analysis of movement. Hexagons have the property of expanding rings of neighbors approximating circles:

<div align="center">
  <img src="/images/neighbors.png" style={{width:'400px'}} /><br />
  <i>All six neighbors of a hexagon (ring 1)</i>
</div>

Hexagons are also optimally space-filling. On average, a polygon may be filled with hexagon tiles with a smaller margin of error than would be present with square tiles.

## References

* Use case: [H3: Uber’s Hexagonal Hierarchical Spatial Index ](https://eng.uber.com/h3/)
* Observable notebook example: [Intro to h3-js](https://observablehq.com/@nrabinowitz/h3-tutorial-the-h3-js-library?collection=@nrabinowitz/h3-tutorial)
* Jupyter notebook example: [H3 Python API](https://github.com/uber/h3-py-notebooks/blob/master/notebooks/usage.ipynb)
