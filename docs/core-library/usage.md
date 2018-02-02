How to use H3
---

The public API of the __H3 Core Library__ is defined in the file h3api.h. The functions defined in h3api.h adhere to [Semantic Versioning](http://semver.org/).

The __H3__ API expects valid input. Behavior of the library may be undefined when given invalid input. Indexes should be validated with `h3IsValid` or `h3UnidirectionalEdgeIsValid` as appropriate.

You can find an example of using the __H3__ library in `examples/index.c`.
