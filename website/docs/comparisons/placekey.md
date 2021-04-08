---
id: placekey
title: Placekey
sidebar_label: Placekey
slug: /comparisons/placekey
---

[Placekey](https://www.placekey.io/) is a system for encoding points of interest (POIs), and incorporates H3 in its POI identifier.

For example, the Placekey for the Ferry Building in San Francisco, `zzw-22y@5vg-7gt-qzz`, contains a What Part (`zzw-22y`) encoding a specific POI, and a Where Part (`5vg-7gt-qzz`) encoding where that POI is. The Where Part is an H3 cell index at resolution 10, using an alternate string encoding. The example Where Part represents the H3 index `8a283082a677fff`. Placekey Where Parts can be losslessly converted to and from their equivalent H3 indexes.
