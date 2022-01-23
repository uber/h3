---
id: testing
title: Testing strategy
sidebar_label: Testing strategy
slug: /core-library/testing
---

The H3 developers try to ensure the robustness and reliability of the H3 library. Tools used to do this include unit tests with coverage reporting
and fuzzing.

The H3 library may despite these efforts behave unexpectedly; in these cases the developers
[welcome feedback and contributions](https://github.com/uber/h3/blob/master/CONTRIBUTING.md).

## Unit testing

Github Actions are used to run the H3 test suite for every commit. A variety of configurations, described below, are tested.

Coverage information is collected in Coveralls. Because of the self-contained nature of the H3 library, we seek
to have as close to 100% code coverage as possible.

| Operating system | Compiler    | Processor architecture | Special notes
| ---------------- | ----------- | ---------------------- | -------------
| Linux (Ubuntu)   | Clang       | x64                    | clang-format-9 is used to ensure all code is consistently formatted
| Linux            | Clang       | x64                    | An additional copy of the job runs with Valgrind
| Linux            | Clang       | x64                    | An additional copy of the job runs with coverage reporting, and excerising the `H3_PREFIX` mechanism.
| Linux            | gcc         | x64                    | 
| Mac OS           | Apple Clang | x64                    | 
| Windows          | MSVC        | x64                    | Static library
| Windows          | MSVC        | x86                    | Static library
| Windows          | MSVC        | x64                    | Dynamic library; testing is not run in this configuration
| Windows          | MSVC        | x86                    | Dynamic library; testing is not run in this configuration

## Benchmarks

H3 uses benchmarks to offer a comparison of the library's performance between revisions.

Benchmarks are automatically run on Linux x64 with Clang and GCC compilers for each commit in Github Actions.

## Fuzzers

H3 uses fuzzers to find novel inputs that crash or result in other undefined behavior.

On each commit, CI is triggered to run [oss-fuzz](https://github.com/google/oss-fuzz/tree/master/projects/h3) for H3.
