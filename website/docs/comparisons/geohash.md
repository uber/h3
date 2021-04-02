---
id: geohash
title: Geohash
sidebar_label: Geohash
slug: /comparisons/geohash
---

[Geohash](https://en.wikipedia.org/wiki/Geohash) is a system for encoding locations using a string of characters, creating a hierarchical, square grid system (a quadtree).

## Area distrotion

Because Geohash encodes latitude and longitudes pairs, it is subject to significant differences in area at different latitudes. A degree of longitude near a pole represents a significantly smaller distance than a degree of longitude near the equator.

## Identifiers

Geohash uses strings for its cell indexes. Because they are strings, they can encode arbitrarily precise cells.

H3 cell indexes are designed to be 64 bit integers, which can be rendered and transmitted as strings if needed. The integer representation can be used when high performance is needed, as integer operations are usually more performant than string oeprations. Because indexes are fixed size, H3 has a maximum resolution is can encode.
