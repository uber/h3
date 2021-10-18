---
id: README
title: Introduction
sidebar_label: Introduction
slug: /
---

H3 is a geospatial indexing system that partitions the world into hexagonal cells. H3 is open source under the Apache 2 license.

The H3 Core Library implements the H3 grid system. It includes functions for converting from latitude and longitude coordinates to the containing H3 cell, finding the center of H3 cells, finding the boundary geometry of H3 cells, finding neighbors of H3 cells, and more.

* The H3 Core Library is written in *C*. [Bindings for many other languages](/docs/community/bindings) are available.

## Highlights

* H3 is a hierarchical [geospatial index](/docs/highlights/indexing).
* H3 was developed to address the [challenges of Uber's data science needs](/docs/highlights/aggregation).
* H3 can be used to [join disparate data sets](/docs/highlights/joining).
* In addition to the benefits of the hexagonal grid shape, H3 includes features for [modeling flow](/docs/highlights/flowmodel).
* H3 is well suited to apply [ML to geospatial data](/docs/highlights/ml).

## Comparisons

* [S2](/docs/comparisons/s2), an open source, hierarchical, discrete, and global grid system using square cells.
* [Geohash](/docs/comparisons/geohash), a system for encoding locations using a string of characters, creating a hierarchical, square grid system (a quadtree).
* [Hexbin](/docs/comparisons/hexbin), the process of taking coordinates and binning them into hexagonal cells in analytics or mapping software.
* [Admin Boundaries](/docs/comparisons/admin), officially designated areas used for aggregating and analyzing data.
* [Placekey](/docs/comparisons/placekey), a system for encoding points of interest (POIs) which incorporates H3 in its POI identifier.
