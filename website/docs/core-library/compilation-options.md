---
id: compilation-options
title: Compilation options
sidebar_label: Compilation options
slug: /core-library/compilation-options
---

A number of options in CMake can be set when compiling the H3 core library. These are relevant for building
different ways of using and testing the library and do not effect the underlying algorithms.

When configuring with CMake, an option may be specified using `-D` on the command line, like so for setting
`CMAKE_BUILD_TYPE` to `Release`:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

## BUILD_ALLOC_TESTS

Whether to build the parts of the [test suite](./testing) that exercise the [H3_ALLOC_PREFIX](./custom-alloc) feature.

## BUILD_BENCHMARKS

Whether to build the [benchmark suite](./testing#benchmarks).

## BUILD_FILTERS

Whether to build the [H3 command line filter](./filters) executables.

## BUILD_GENERATORS

Whether to build generator executables used in the development of H3. This is not required for
building the library as the output of these generators is checked in.

## BUILD_TESTING

Whether to build the [test suite](./testing).

## CMAKE_BUILD_TYPE

Should be set to `Release` for production builds, and `Debug` in development.

## ENABLE_COVERAGE

Whether to compile `Debug` builds with coverage instrumentation (compatible with GCC, Clang, and lcov).

## ENABLE_DOCS

Whether to build the Doxygen documentation. This is not the same as the [H3 website](https://github.com/uber/h3/tree/master/website),
but is rather the documentation for the internal C library functions.

## ENABLE_FORMAT

Whether to enable using clang-format-9 to format source files before building. This should be enabled
before submitting patches for H3 as continuous integration will fail if the formatting does not match.

## ENABLE_LINTING

## H3_ALLOC_PREFIX

Used for directing the library to use a [different set of functions for memory management](./custom-alloc).

## H3_PREFIX

Used for [renaming the public API](./usage#function-renaming).

## PRINT_TEST_FILES

If enabled, CMake will print which CTest test case corresponds to which input file.

## WARNINGS_AS_ERRORS

Whether to treat compiler warnings as errors. While a useful tool for ensuring software quality, this should not be enabled
for production builds as compiler warnings can change unexpectedly between versions.

## WRAP_VALGRIND

Whether to wrap invocations of the test suite with `valgrind` (if available).

