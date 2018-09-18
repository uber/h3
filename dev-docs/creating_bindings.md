# Creating bindings for H3

H3 is a C library, in part to make it simpler to create bindings for different programming languages. Each language usually has its own way to bind to C functions, but this document can serve as a starting point and list of tips.

There may already be [H3 bindings](https://uber.github.io/h3/#/documentation/community/bindings) available for your language of choice.

## Function naming

The `make binding-functions` target produces a file `binding-functions` containing a list of function in the H3 public API, one per line. You can use this as part of your build process to check how much of the H3 public API your bindings expose. This list does not include memory management functions that are needed to allocate arrays to be passed to the H3 API.

Keeping similar names and purposes for functions can make it easier for your users to use the H3 [API Reference](https://uber.github.io/h3/#/documentation/api-reference/).

## Community

When ready, make a [pull request](https://github.com/uber/h3/edit/master/docs/community/bindings.md) to add your binding to the H3 documentation website.

## Documentation

To be included in the H3 [API reference](https://uber.github.io/h3/#/documentation/api-reference/), your binding should:

* Be reasonably up to date with uber/h3.
* Include bindings for the relevant functions in the output of `make binding-functions`. For example, `stringToH3` may not be necessary if your bindings only supports string H3 indexes.
* Use the major and minor version of the version of H3 bound. For example, when binding H3 version 3.2.1, a valid binding version would be 3.2.0, but 4.2.1 would not be valid.
