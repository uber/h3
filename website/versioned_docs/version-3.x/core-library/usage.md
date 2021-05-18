---
id: usage
title: Public API
sidebar_label: Public API
slug: /core-library/usage
---

## API Versioning

The public API of the H3 Core Library is defined in the file `h3api.h`. The functions defined in h3api.h adhere to [Semantic Versioning](http://semver.org/).

## API Preconditions

The H3 API expects valid input. Behavior of the library may be undefined when given invalid input. Indexes should be validated with `h3IsValid` or `h3UnidirectionalEdgeIsValid` as appropriate.
