H3 Core
---

The __H3__ geospatial indexing system is a multi-precision hexagonal tiling of the sphere indexed with hierarchical linear indexes. The __H3 Core Library__ provides functions for converting between latitude/longitude coordinates and __H3__ geospatial indexes. Specifically, the major library entry point functions (defined in `h3api.h`) provide the functionality:

* given a latitude/longitude point, find the index of the containing __H3__ cell at a particular resolution
* given an __H3__ index, find the latitude/longitude cell center
* given an __H3__ index, determine the cell boundary in latitude/longitude coordinates
* and more.

The __H3 Core Library__ is written entirely in _C_. Bindings for several languages are coming.
