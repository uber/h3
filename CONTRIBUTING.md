# Contributing to the H3 project

Pull requests and Github issues are welcome!

Planned improvements and changes are discussed in Github issues. Feel free to open a discussion proposing a new one.

## Pull requests

* Please include tests that show the bug is fixed or feature works as intended.
* Please include fuzzer tests for public API functions.
* Please add a description of your change to the Unreleased section of the [changelog](./CHANGELOG.md).
* Please open issues to discuss large features or changes which would break compatibility, before submitting pull requests.
* Please keep H3 compatible with major C compilers, such as GCC, Clang, and MSVC. We use clang-format-11 for source code formatting, if you have another version the CI job may error on formatting differences.
* Please keep code coverage of the core H3 library at 100%.

Before we can merge your changes, you must agree to the [Uber Contributor License Agreement](https://cla-assistant.io/uber/h3).

## Other ways to contribute

In addition to making pull requests and Github issues on the @uber/h3 repository, there are other ways you can contribute. Here are some suggestions:

* Writing and updating the [documentation website](./docs/).
* Writing [bindings](./dev-docs/creating_bindings.md) for your preferred language.
* Writing components so that your preferred GIS, mapping, or visualization system can display H3 indexes.
