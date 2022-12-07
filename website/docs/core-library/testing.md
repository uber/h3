---
id: testing
title: Testing strategy
sidebar_label: Testing strategy
slug: /core-library/testing
---

The H3 developers try to ensure the robustness and reliability of the H3 library. Tools used to do this include defensive code, unit tests with coverage reporting, and fuzzing.

The H3 library may despite these efforts behave unexpectedly; in these cases the developers
[welcome feedback and contributions](https://github.com/uber/h3/blob/master/CONTRIBUTING.md).

## Unit testing

Github Actions are used to run the H3 test suite for every commit. A variety of configurations, described below, are tested.

Coverage information is collected in Coveralls. Because of the self-contained nature of the H3 library, we seek to have as close to 100% code coverage as possible.

| Operating system | Compiler    | Build type     | Processor architecture | Special notes
| ---------------- | ----------- | -------------- | ---------------------- | -------------
| Linux (Ubuntu)   | Clang       | Debug, Release | x64                    | clang-format-11 is used to ensure all code is consistently formatted
| Linux            | Clang       | Debug          | x64                    | An additional copy of the job runs with [Valgrind](https://valgrind.org/)
| Linux            | Clang       | Debug          | x64                    | An additional copy of the job runs with coverage reporting, and excerising the `H3_PREFIX` mechanism.
| Linux            | gcc         | Debug, Release | x64                    |
| Mac OS           | Apple Clang | Debug, Release | x64                    |
| Windows          | MSVC        | Debug, Release | x64                    | Static library
| Windows          | MSVC        | Debug, Release | x86                    | Static library
| Windows          | MSVC        | Debug, Release | x64                    | Dynamic library; testing is not run in this configuration
| Windows          | MSVC        | Debug, Release | x86                    | Dynamic library; testing is not run in this configuration

## Defensive code

H3 uses preprocessor macros borrowed from [SQLite's testing methodology](https://www.sqlite.org/testing.html) to include defensive code in the library. Defensive code is code that handles error conditions for which there are no known test cases to demonstrate it. The lack of known test cases means that without the macros, the defensive cases could inappropriately reduce coverage metrics, disincentivizing including them. The macros behave differently, depending on the build configuration:

* Under release builds of the library (`CMAKE_BUILD_TYPE=Release`), the defensive code is included without modification. These branches are intended to be very simple (usually only `return`ing an error code and possibly `free`ing some resources) and to be visually inspectable.

* Under debug builds of the library (`CMAKE_BUILD_TYPE=Debug`), the defensive code is included and `assert` calls are included if the defensive code is invoked. Any unit test or fuzzer which can demonstrate the defensive code is actually reached will trigger a test failure and the developers can be alerted to cover the defensive code in unit tests.

* Under coverage builds of the library (`CMAKE_BUILD_TYPE=Debug ENABLE_COVERAGE=ON`), the defensive code is not included by replacing its condition with a constant value. The compiler removes the defensive code and it is not counted in coverage metrics. This is intended only for determining test coverage of the library.

## Benchmarks

H3 uses benchmarks to offer a comparison of the library's performance between revisions.

Benchmarks are automatically run on Linux x64 with Clang and GCC compilers for each commit in Github Actions.

## Fuzzers

H3 uses [fuzzers](https://github.com/uber/h3/tree/master/src/apps/fuzzers) to find novel inputs that crash or result in other undefined behavior.

On each commit, CI is triggered to run [OSS-Fuzz](https://github.com/google/oss-fuzz/tree/master/projects/h3) for H3. OSS-Fuzz regularly runs fuzzers against the latest development version of H3 and reports newly discovered issues to the H3 core maintainers.
