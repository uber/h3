---
id: restable
title: Tables of Cell Statistics Across Resolutions
sidebar_label: Tables of cell stats
slug: /core-library/restable
---

## Cell counts

We list the number of hexagons and pentagons at each H3 resolution.
[There are always exactly $12$ pentagons at every resolution](../core-library/overview.md).


|   Res |   Total number of cells |   Number of hexagons |   Number of pentagons |
|------:|------------------------:|---------------------:|----------------------:|
|     0 |                     122 |                  110 |                    12 |
|     1 |                     842 |                  830 |                    12 |
|     2 |                   5,882 |                5,870 |                    12 |
|     3 |                  41,162 |               41,150 |                    12 |
|     4 |                 288,122 |              288,110 |                    12 |
|     5 |               2,016,842 |            2,016,830 |                    12 |
|     6 |              14,117,882 |           14,117,870 |                    12 |
|     7 |              98,825,162 |           98,825,150 |                    12 |
|     8 |             691,776,122 |          691,776,110 |                    12 |
|     9 |           4,842,432,842 |        4,842,432,830 |                    12 |
|    10 |          33,897,029,882 |       33,897,029,870 |                    12 |
|    11 |         237,279,209,162 |      237,279,209,150 |                    12 |
|    12 |       1,660,954,464,122 |    1,660,954,464,110 |                    12 |
|    13 |      11,626,681,248,842 |   11,626,681,248,830 |                    12 |
|    14 |      81,386,768,741,882 |   81,386,768,741,870 |                    12 |
|    15 |     569,707,381,193,162 |  569,707,381,193,150 |                    12 |


## Cell areas

:::caution
Cell areas are computed with a **spherical** model of the earth using the
[authalic radius](https://en.wikipedia.org/wiki/Earth_radius#Authalic_radius)
given by
[WGS84](https://en.wikipedia.org/wiki/WGS84)/[EPSG:4326](https://epsg.io/4326).
:::

### Average area in km<sup>2</sup>

The area of an H3 cell varies based on its position relative to the
[icosahedron vertices](../core-library/overview.md).
We show the **average** hexagon areas for each resolution.
All pentagons within a resolution have the same area.


|   Res |   Average <ins>Hexagon</ins> Area (km<sup>2</sup>) |   Pentagon Area* (km<sup>2</sup>) |   Ratio (P/H) |
|------:|---------------------------------------------------:|----------------------------------:|--------------:|
|     0 |                                4,357,449.416078381 |               2,562,182.162955496 |        0.5880 |
|     1 |                                  609,788.441794133 |                 328,434.586246469 |        0.5386 |
|     2 |                                   86,801.780398997 |                  44,930.898497879 |        0.5176 |
|     3 |                                   12,393.434655088 |                   6,315.472267516 |        0.5096 |
|     4 |                                    1,770.347654491 |                     896.582383141 |        0.5064 |
|     5 |                                      252.903858182 |                     127.785583023 |        0.5053 |
|     6 |                                       36.129062164 |                      18.238749548 |        0.5048 |
|     7 |                                        5.161293360 |                       2.604669397 |        0.5047 |
|     8 |                                        0.737327598 |                       0.372048038 |        0.5046 |
|     9 |                                        0.105332513 |                       0.053147195 |        0.5046 |
|    10 |                                        0.015047502 |                       0.007592318 |        0.5046 |
|    11 |                                        0.002149643 |                       0.001084609 |        0.5046 |
|    12 |                                        0.000307092 |                       0.000154944 |        0.5046 |
|    13 |                                        0.000043870 |                       0.000022135 |        0.5046 |
|    14 |                                        0.000006267 |                       0.000003162 |        0.5046 |
|    15 |                                        0.000000895 |                       0.000000452 |        0.5046 |

*: Within a given resolution, all pentagons have the same area.


### Average area in m<sup>2</sup>

Here are the same areas, but in m<sup>2</sup>.

|   Res |   Average <ins>Hexagon</ins> Area (m<sup>2</sup>) |   Pentagon Area* (m<sup>2</sup>) |
|------:|--------------------------------------------------:|---------------------------------:|
|     0 |                             4,357,449,416,078.392 |            2,562,182,162,955.496 |
|     1 |                               609,788,441,794.134 |              328,434,586,246.469 |
|     2 |                                86,801,780,398.997 |               44,930,898,497.879 |
|     3 |                                12,393,434,655.088 |                6,315,472,267.516 |
|     4 |                                 1,770,347,654.491 |                  896,582,383.141 |
|     5 |                                   252,903,858.182 |                  127,785,583.023 |
|     6 |                                    36,129,062.164 |                   18,238,749.548 |
|     7 |                                     5,161,293.360 |                    2,604,669.397 |
|     8 |                                       737,327.598 |                      372,048.038 |
|     9 |                                       105,332.513 |                       53,147.195 |
|    10 |                                        15,047.502 |                        7,592.318 |
|    11 |                                         2,149.643 |                        1,084.609 |
|    12 |                                           307.092 |                          154.944 |
|    13 |                                            43.870 |                           22.135 |
|    14 |                                             6.267 |                            3.162 |
|    15 |                                             0.895 |                            0.452 |

*: Within a given resolution, all pentagons have the same area.


### Hexagon min and max areas

The area of an H3 cell varies based on its position relative to the
[icosahedron vertices](../core-library/overview.md).
We compute the minimum and maximum values for the **hexagon** areas (excluding
the pentagons) at each resolution, and show their ratio.


|   Res |   Min <ins>Hexagon</ins> Area (km^2) |   Max <ins>Hexagon</ins> Area (km^2) |   Ratio (max/min) |
|------:|-------------------------------------:|-------------------------------------:|------------------:|
|     0 |                  4,106,166.334463915 |                  4,977,807.027442012 |          1.212276 |
|     1 |                    447,684.201817940 |                    729,486.875275344 |          1.629468 |
|     2 |                     56,786.622889474 |                    104,599.807218925 |          1.841980 |
|     3 |                      7,725.505769639 |                     14,950.773301379 |          1.935248 |
|     4 |                      1,084.005635363 |                      2,135.986983965 |          1.970457 |
|     5 |                        153.766244448 |                        305.144308779 |          1.984469 |
|     6 |                         21.910021013 |                         43.592111685 |          1.989597 |
|     7 |                          3.126836030 |                          6.227445905 |          1.991613 |
|     8 |                          0.446526174 |                          0.889635157 |          1.992347 |
|     9 |                          0.063780227 |                          0.127090737 |          1.992635 |
|    10 |                          0.009110981 |                          0.018155820 |          1.992740 |
|    11 |                          0.001301542 |                          0.002593689 |          1.992782 |
|    12 |                          0.000185933 |                          0.000370527 |          1.992797 |
|    13 |                          0.000026562 |                          0.000052932 |          1.992802 |
|    14 |                          0.000003795 |                          0.000007562 |          1.992805 |
|    15 |                          0.000000542 |                          0.000001080 |          1.992805 |


## Edge lengths

:::caution
Edge lengths are computed with a **spherical** model of the earth using the
[authalic radius](https://en.wikipedia.org/wiki/Earth_radius#Authalic_radius)
given by
[WGS84](https://en.wikipedia.org/wiki/WGS84)/[EPSG:4326](https://epsg.io/4326).
:::

:::danger
These values are hard coded into the `getHexagonEdgeLengthAvgKm` function and
related functions in the H3 library. There are [known issues](https://github.com/uber/h3/issues/666)
with these numbers and they are expected to be updated later.
:::

| Res | Average edge length (Km) |
|----:|-------------------------:|
|   0 | 1107.712591              |
|   1 | 418.6760055              |
|   2 | 158.2446558              |
|   3 | 59.81085794              |
|   4 | 22.6063794               |
|   5 | 8.544408276              |
|   6 | 3.229482772              |
|   7 | 1.220629759              |
|   8 | 0.461354684              |
|   9 | 0.174375668              |
|  10 | 0.065907807              |
|  11 | 0.024910561              |
|  12 | 0.009415526              |
|  13 | 0.003559893              |
|  14 | 0.001348575              |
|  15 | 0.000509713              |

## Appendix: Methodology

<div align="center">
  <img src="/images/pentagon_hexagon_children.png" style={{width:'800px'}} /><br />
  <i>Hexagons have 7 hexagon children. Pentagons have 6 children: 5 hexagons and 1 pentagon.</i>
</div>

### Cell counts

[By definition](../core-library/overview.md), resolution `0` has $110$
**hexagons** and $12$ **pentagons**, for a total of $122$ **cells**.

In fact, *every* H3 resolution has exactly $12$ **pentagons**, which are always
centered at the icosahedron vertices; the number of **hexagons** increases
with each resolution.

:::tip Formula
Accounting for both **hexagons** and **pentagons**,
the total number of **cells** at resolution $r$ is

$$
c(r) = 2 + 120 \cdot 7^r.
$$
:::

#### Derivation of the cell count formula

We can derive the formula above with the following steps.

First, let $h(n)$ be the number of
children $n \geq 0$ resolution levels below any single **hexagaon**.
Any **hexagon** has $7$ immediate children, so recursion gives us
that

$$
h(n) = 7^n.
$$

Next, let $p(n)$ be the number of children $n \geq 0$ resolution levels below
any single **pentagon**.
Any **pentagon** has $5$ hexagonal immediate children and $1$ pentagonal
immediate child.
Thus, $p(0) = 1$ and $p(1) = 6$.

For $n \geq 1$, we get the general recurrence relation
 
$$
\begin{aligned}
p(n) &= 5 \cdot  h(n-1) + p(n-1) \\
     &= 5 \cdot 7^{n-1} + p(n-1).
\end{aligned}
$$

For $n \geq 0$, after working through the recurrence, we get that

$$
\begin{aligned}
p(n) &= 1 + 5 \cdot \sum_{k=1}^n\ 7^{k-1} \\
     &= 1 + 5 \cdot \frac{7^n - 1}{6},
\end{aligned}
$$

using the closed form for a
[geometric series](https://en.wikipedia.org/wiki/Geometric_series).


Finally, using the closed forms for $h(n)$ and $p(n)$,
and the fact that ([by definition](../core-library/overview.md))
resolution `0` has
$12$ **pentagons** and $110$ **hexagons**,
we get the closed form for the total number of **cells**
at resolution $r$ as

$$
\begin{aligned}
c(r) &= 12 \cdot p(r) + 110 \cdot h(r) \\
     &= 2 + 120 \cdot 7^r.
\end{aligned}
$$

#### Jupyter notebook

A notebook to produce the cell count table above can be
[found here](https://github.com/uber/h3-py-notebooks/blob/master/notebooks/stats_tables/cell_counts.ipynb).

### Cell areas

Cell areas are computed with a **spherical** model of the earth using the
[authalic radius](https://en.wikipedia.org/wiki/Earth_radius#Authalic_radius)
given by
[WGS84](https://en.wikipedia.org/wiki/WGS84)/[EPSG:4326](https://epsg.io/4326).

The [`h3-py-notebooks` repo](https://github.com/uber/h3-py-notebooks)
has notebooks for producing the
[average cell area table](https://github.com/uber/h3-py-notebooks/blob/master/notebooks/stats_tables/avg_area_table.ipynb)
and the
[min/max area table](https://github.com/uber/h3-py-notebooks/blob/master/notebooks/stats_tables/extreme_hex_area.ipynb).

### Edge lengths

todo
