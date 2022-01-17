---
id: usage
title: Public API
sidebar_label: Public API
slug: /core-library/usage
---

The public API of the H3 Core Library is defined in the file [`h3api.h`](https://github.com/uber/h3/blob/master/src/h3lib/include/h3api.h.in).

## API Versioning

The functions defined in `h3api.h` adhere to [Semantic Versioning](https://semver.org/).

## Header preprocessing

The file `h3api.h.in` is preprocessed into the file `h3api.h` as part of H3's build process. The preprocessing inserts the correct values for the `H3_VERSION_MAJOR`, `H3_VERSION_MINOR`, and `H3_VERSION_PATCH` macros.

## API preconditions

The H3 API expects valid input. Behavior of the library may be undefined when given invalid input. Indexes should be validated with `h3IsValid` or `h3UnidirectionalEdgeIsValid` as appropriate.

## Function renaming

The [`H3_PREFIX`](./compilation-options#H3_PREFIX) exists to rename all functions in the H3 public API with a prefix chosen at compile time.
Internal functions and symbols are not renamed.
