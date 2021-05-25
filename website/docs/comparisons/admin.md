---
id: admin
title: Admin Boundaries
sidebar_label: Admin Boundaries
slug: /comparisons/admin
---

Administrative boundaries, such as ZIP Codes and Census Blocks in the United States, can be used for aggregating and analyzing data. These boundaries have a number of drawbacks for aggregating data. These are primarily related to not having a comparable spatial unit of analysis, being unable to spatially relate data, and being unrelated to the data being analyzed.

## ZIP Codes

The article [Stop Using Zip Codes for Geospatial Analysis](https://towardsdatascience.com/stop-using-zip-codes-for-geospatial-analysis-ceacb6e80c38) summarizes a number of problems with using ZIP Codes. In short, ZIP Codes do not represent areas themselves but rather mail delivery routes. They also vary greatly in spatial size when rendered as ZIP Code Tabulation Areas and [change for unrelated reasons](https://fas.org/sgp/crs/misc/RL33488.pdf).

## Use case specific partitioning

When using manually drawn partitions, there is usually no spatial unit of analysis which can be compared. Edges of partitions may exhibit boundary effects due to not taking into account neighboring partitions.

Manually drawn partitions can better incorporate human knowledge, but can require updating as that knowledge changes. It can take a significant amount of time and effort to define and update partitions manually.

The varying size of partitions means the center of a partition may be unrelated to the center of the data points.

## ZIP Codes vs H3 comparison

<iframe width="100%" height="500px" src="https://studio.unfolded.ai/public/72504bc0-184e-4ba0-b10b-72fdf61e2c33/embed" frameborder="0" allowfullscreen></iframe>

ZIP Codes on the left, H3 on the right. Data: [New York City 2015 Street Tree Census](https://data.cityofnewyork.us/Environment/2015-Street-Tree-Census-Tree-Data/uvpi-gqnh)

