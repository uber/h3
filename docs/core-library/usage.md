How to use H3
---

The public API of the **H3 Core Library** is defined in the file h3api.h. The functions defined in h3api.h adhere to [Semantic Versioning](http://semver.org/).

The **H3** API expects valid input. Behavior of the library may be undefined when given invalid input. Indexes should be validated with `h3IsValid` or `h3UnidirectionalEdgeIsValid` as appropriate.

You can find an example of using the **H3** library in `examples/index.c`.
