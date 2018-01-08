# H3 Core

- - -

\section Introduction

The __H3__ geospatial indexing system is a multi-precision hexagonal tiling of the sphere indexed with hierarchical linear indexes. The __H3 Core Library__ provides functions for converting between latitude/longitude coordinates and __H3__ geospatial indexes. Specifically, the major library entry point functions (defined in `h3api.h`) provide the functionality:

* given a latitude/longitude point, find the index of the containing __H3__ cell at a particular resolution
* given an __H3__ index, find the latitude/longitude cell center
* given an __H3__ index, determine the cell boundary in latitude/longitude coordinates
* and more.

The __H3 Core Library__ is written entirely in _C_.

The pages below contain high level developer documentation for the __H3 Core library__. See the comments in the code itself for more detailed information on the functioning of specific algorithms.

* <a href="./md_doxyfiles_overview.html">Overview of the __H3__ Geospatial Indexing System</a>
* <a href="./md_doxyfiles_usecases.html">Why __H3__: use cases and comparisons</a>
* <a href="./md_doxyfiles_usage.html">How to use __H3__</a>
* <a href="./md_doxyfiles_restable.html">Table of Average Cell Areas for H3 Resolutions</a>
* <a href="./md_doxyfiles_coordsystems.html">Coordinate Systems used by the __H3 Core Library__</a>
* <a href="./md_doxyfiles_h3indexing.html"> __H3__ Index Representations</a>
* High level descriptions of the 3 primary core library functions:

   * <a href="./md_doxyfiles_geo_to_h3desc.html"> latitude/longitude to containing __H3__ cell index</a> (function ::geoToH3)
   * <a href="./md_doxyfiles_h3_to_geo_desc.html"> __H3__ cell index to latitude/longitude center point</a> (function ::h3ToGeo)
   * <a href="./md_doxyfiles_h3_to_geo_boundary_desc.html"> __H3__ cell index to latitude/longitude cell boundary</a> (function ::h3ToGeoBoundary)

* <a href="./md_doxyfiles_filters.html">Unix-style Filters for Address Conversion</a>

