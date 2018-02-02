# H3: A Hexagonal Hierarchical Geospatial Indexing System

[![Build Status](https://travis-ci.org/uber/h3.svg?branch=master)](https://travis-ci.org/uber/h3)
[![Coverage Status](https://coveralls.io/repos/github/uber/h3/badge.svg?branch=master)](https://coveralls.io/github/uber/h3?branch=master)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

H3 is a geospatial indexing system using a hexagonal grid that can be (approximately) subdivided into finer and finer hexagonal grids, combining the benefits of a hexagonal grid with [S2](https://code.google.com/archive/p/s2-geometry-library/)'s hierarchical subdivisions.

Documentation is available at [https://uber.github.io/h3/]. Developer documentation in Markdown format is available under the [dev-docs](./dev-docs/) directory.

## Installing

For now, you can only install H3 by building from source. However, bindings for several languages are coming. If you're using a language with bindings, it's recommended you follow their instructions instead.

### Building from source

Still here? To build the H3 C library, you'll need a C compiler (tested with `gcc` and `clang`), [CMake](https://cmake.org/), and [Make](https://www.gnu.org/software/make/). If you intend to contribute to H3, you must have [clang-format](https://clang.llvm.org/docs/ClangFormat.html) installed and we recommend installing [ccmake](https://cmake.org/cmake/help/v3.0/manual/ccmake.1.html) and [LCOV](http://ltp.sourceforge.net/coverage/lcov.php) to configure the `cmake` arguments to build and run the tests and generate the code coverage report. We also recommend using `gcc` for the code coverage as some versions of `clang` generate annotations that aren't compatible with `lcov`. [Doxygen](http://www.stack.nl/~dimitri/doxygen/) is needed to build the API documentation.

#### Install build-time dependencies

* Debian/Ubuntu

```
# Installing the bare build requirements
sudo apt install cmake make gcc libtool
# Installing useful tools for development
sudo apt install clang-format cmake-curses-gui lcov doxygen
```

* macOS (using brew)

First make sure you [have the developer tools installed](http://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/) and then

```
# Installing the bare build requirements
brew install cmake
# Installing useful tools for development
brew install clang-format lcov doxygen
```

* Windows (Visual Studio)

You will need to install CMake and Visual Studio, including the Visual C++ compiler. For building on Windows, please follow the [Windows build instructions](dev-docs/build_windows.md).

#### Compilation

From the repository you would then compile like so:

```
cmake .
make
```

You can install system-wide with:

```
sudo make install
```

#### Testing

After making the project, you can test with `make test`, and if `lcov` is installed you can `make coverage` to generate a code coverage report.

#### Documentation

You can build developer documentation with `make docs` if Doxygen was installed when CMake was run. Index of the documentation will be `dev-docs/_build/html/index.html`.

After making the project, you can build KML files to visualize the hexagon grid with `make kml`. The files will be placed in `KML`.

To build the documentation website, see the [website/](./website/) directory.

## Usage

### From the command line

To get the H3 index for some location:

```
./bin/geoToH3 10
40.689167 -74.044444
```

10 is the H3 resolution, between 0 (coarsest) and 15 (finest). The coordinates entered are the latitude and longitude, in degrees, you want the index for (these coordinates are the Statue of Liberty).  You should get an H3 index as output, like `8a2a1072b59ffff`.

You can then take this index and get some information about it, for example:

```
./bin/h3ToGeoBoundary
8a2a1072b59ffff
```

This will produce the vertices of the hexagon at this location:

```
8a2a1072b59ffff
{
   40.690058601 285.955848238
   40.689907695 285.954938208
   40.689270936 285.954658582
   40.688785091 285.955288969
   40.688935993 285.956198979
   40.689572744 285.956478623
}
```

You can get the center coordinate of the hexagon like so:

```
./bin/h3ToGeo
8a2a1072b59ffff
```

This will produce some coordinate:

```
40.6894218437 285.9555686001
```

### From C

The above features of H3 can also be used from C. For example, you can compile and run [examples/index.c](./examples/index.c) like so:

```
cc -lh3.1 examples/index.c -o example
./example
```

You should get output like:

```
The index is: 8a2a1072b59ffff
Boundary vertex #0: 40.690059, 285.955848
Boundary vertex #1: 40.689908, 285.954938
Boundary vertex #2: 40.689271, 285.954659
Boundary vertex #3: 40.688785, 285.955289
Boundary vertex #4: 40.688936, 285.956199
Boundary vertex #5: 40.689573, 285.956479
Center coordinates: 40.689422, 285.955569
```

## Contributing

Pull requests and Github issues are welcome. Please include tests that show the bug is fixed or feature works as intended. Please add a description of your change to the Unreleased section of the [changelog](./CHANGELOG.md). Please open issues to discuss large features or changes which would break compatibility, before submitting pull requests.

Before we can merge your changes, you must agree to the [Uber Contributor License Agreement](http://t.uber.com/cla).

## Legal and Licensing

H3 is licensed under the [Apache 2.0 License](./LICENSE).

DGGRID
Copyright (c) 2015 Southern Oregon University
