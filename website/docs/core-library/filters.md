---
id: filters
title: Unix-style Filters for H3
sidebar_label: Unix-style Filters for H3
slug: /core-library/filters
---

The directory `src/apps/filters` contains unix-style stdin/stdout filters that perform conversions between integer H3 indexes and other useful types. It currently contains the filters listed in the table below. See the header comments in each application source code file for more information.

Filters are experimental and are not part of the semantic version of the H3 library.

All latitude/longitude coordinates are in decimal degrees. See the [H3 Index Representations](/docs/core-library/h3indexing) page for information on the integer `H3Index`.


|      filter      |   input   |             outputs             |
|------------------|-----------|---------------------------------|
| `latLngToCell`   | lat/lng   | `H3Index`                       |
| `cellToLatLng`   | `H3Index` | cell center point in lat/lng    |
| `cellToBoundary` | `H3Index` | cell boundary in lat/lng        |
| `h3ToComponents` | `H3Index` | components                      |
| `gridDisk`       | `H3Index` | surrounding `H3Index`           |
| `gridDiskUnsafe` | `H3Index` | surrounding `H3Index`, in order |

Unix Command Line Examples
---

* find the index for coordinates at resolution 5

     `latLngToCell --resolution 5 --latitude 40.689167 --longitude -74.044444`

* output the cell center point for `H3Index` 845ad1bffffffff

     `cellToLatLng --index 845ad1bffffffff`

* output the cell boundary for `H3Index` 845ad1bffffffff

     `cellToBoundary --index 845ad1bffffffff`

* find the components for the `H3Index` 845ad1bffffffff

     `h3ToComponents --index 845ad1bffffffff`

* output all indexes within distance 1 of the `H3Index` 845ad1bffffffff

     `kRing -k 1 --origin 845ad1bffffffff`

* output all hexagon indexes within distance 2 of the `H3Index` 845ad1bffffffff

     `hexRange -k 2 --origin 845ad1bffffffff`

Note that the filters `cellToLatLng` and `cellToBoundary` take optional arguments that allow them to generate `kml` output. See the header comments in the corresponding source code files for details.
