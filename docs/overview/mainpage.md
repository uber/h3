H3 Core
---

The **H3** geospatial indexing system is a multi-precision hexagonal tiling of the sphere indexed with hierarchical linear indexes. The **H3 Core Library** provides functions for converting between latitude/longitude coordinates and **H3** geospatial indexes. Specifically, the major library entry point functions (defined in `h3api.h`) provide the functionality:

* given a latitude/longitude point, find the index of the containing **H3** cell at a particular resolution
* given an **H3** index, find the latitude/longitude cell center
* given an **H3** index, determine the cell boundary in latitude/longitude coordinates
* and more.

The **H3 Core Library** is written entirely in *C*. <a href="#/documentation/community/bindings">Bindings for other languages</a> are available.
