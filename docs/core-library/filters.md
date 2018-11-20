Unix-style Filters for Address Conversion
---

The directory `src/apps/filters` contains unix-style stdin/stdout filters that perform conversions between integer **H3** indexes and other useful types. It currently contains the filters listed in the table below. See the header comments in each application source code file for more information.

Filters are experimental and are not part of the semantic version of the H3 library.

All latitude/longitude coordinates are in decimal degrees. See the <a href="#/documentation/core-library/h3-index-representations"> **H3** Index Representations</a> page for information on the integer `H3Index`.


| filter           | input     | outputs
| ---------------- | --------- | -------
| `geoToH3`        | lat/lon   | `H3Index`
| `h3ToGeo`        | `H3Index` | cell center point in lat/lon
| `h3ToGeoBoundary`| `H3Index` | cell boundary in lat/lon
| `h3ToComponents` | `H3Index` | components
| `kRing`          | `H3Index` | surrounding `H3Index`
| `hexRange`       | `H3Index` | surrounding `H3Index`, in order

Unix Command Line Examples
---

* find the index for coordinates at resolution 5

     `echo 40.689167 -74.044444 | geoToH3 5`

* output the cell center point for `H3Index` 845ad1bffffffff

     `echo 845ad1bffffffff | h3ToGeo`

* output the cell boundary for `H3Index` 845ad1bffffffff

     `echo 845ad1bffffffff | h3ToGeoBoundary`

* find the components for the `H3Index` 845ad1bffffffff

     `echo 845ad1bffffffff | h3ToComponents`

* output all indexes within distance 1 of the `H3Index` 845ad1bffffffff

     `echo 845ad1bffffffff | kRing 1`

* output all hexagon indexes within distance 2 of the `H3Index` 845ad1bffffffff

     `echo 845ad1bffffffff | hexRange 2`

Note that the filters `h3ToGeo` and `h3ToGeoBoundary` take optional arguments that allow them to generate `kml` output. See the header comments in the corresponding source code files for details.
