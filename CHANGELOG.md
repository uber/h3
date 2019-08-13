# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

The public API of this library consists of the functions declared in file
[h3api.h.in](./src/h3lib/include/h3api.h.in).

## [Unreleased]

## [3.6.0] - 2019-08-12
### Added
- `h3ToCenterChild` function to find center child at given resolution (#267)
- `getPentagonIndexes` (and `pentagonIndexCount`) function to find pentagons at given resolution (#267)
### Fixed
- Fix bounds check for local IJ coordinates (#271)

## [3.5.0] - 2019-07-22
### Added
- CMake options for excluding filter applications or benchmarks from the build. (#247)
- `h3GetFaces` function to find icosahedron faces for an index, and helper function `maxFaceCount` (#253)
### Changed
- Argument parsing for all filter applications is more flexible. (#238)
### Fixed
- Fix printing program name in `h3ToHier` error messages. (#254)

## [3.4.4] - 2019-05-30
### Changed
- Local coordinate spaces cannot cross more than one icosahedron edge. (#234)
- All dynamic internal memory allocations happen on the heap instead of the stack. (#235)
- Argument parsing for `h3ToGeo`, `geoToH3`, and `h3ToGeoBoundary` is more flexible. (#227)

## [3.4.3] - 2019-05-02
### Added
- `localIjToH3` filter application (#222)
- An option to print distances in the `kRing` filter application (#222)
### Changed
- Arguments parsing for `kRing` filter application is more flexible. (#224)
### Fixed
- `benchmarkPolyfill` allocates its memory on the heap (#198)
- Fixed constraints of vertex longitudes (#213)
- Zero only input to `uncompact` does not produce an error (#223)

## [3.4.2] - 2019-02-21
### Changed
- `binding-functions` build target generates an ASCII file on Windows (#193)

## [3.4.1] - 2019-02-15
### Fixed
- `binding-functions` build target fixed when running the build out of source (#188)

## [3.4.0] - 2019-01-23
### Added
- `getRes0Indexes` function for getting all base cells, and helper function `res0IndexCount` (#174)
- Include defined constants for current library version (#173)

## [3.3.0] - 2018-12-25
### Added
- `h3Line` and `h3LineSize` functions for getting the line of indexes between some start and end (inclusive) (#165)
### Changed
- Indexes in deleted pentagon subsequences are not considered valid.

## [3.2.0] - 2018-10-04
### Added
- `experimentalH3ToLocalIj` function for getting local coordinates for an index. (#102)
- `experimentalLocalIjToH3` function for getting an index from local coordinates. (#102)
- Benchmarks for the kRing method for k's of size 10, 20, 30, and 40. (#142, #144)
### Changed
- Internal `h3ToIjk` function renamed to `h3ToLocalIjk`. (#102)
- `h3ToIjk` filter application replaced with `h3ToLocalIj`. (#102)
### Fixed
- Added `#include <stdio.h>` to `benchmark.h` (#142)

## [3.1.1] - 2018-08-29
### Fixed
- Fixed bounding box bug for polygons crossing the antimeridian (#130)
- Normalize output of h3SetToMultiPolygon to align with the GeoJSON spec, ensuring that each polygon has only one outer loop, followed by holes (#131)
### Changed
- Longitude outputs are now guaranteed to be in the range [-Pi, Pi]. (#93)
- Implemented closed form formula for maxKringSize. Source: https://oeis.org/A003215 (#138)
- Improved test and benchmark harnesses (#126, #128)

## [3.1.0] - 2018-08-03
### Added
- `h3Distance` function for determining the grid distance between H3 indexes (#83)
- Internal `h3ToIjk` function for getting IJK+ coordinates from an index (#83)
- Internal `ijkDistance` function for determining the grid distance between IJK+ coordinates (#83)
- `h3ToIjk` filter application for experimenting with `h3ToIjk` (#83)
### Fixed
- Don't require a C++ compiler (#107)
### Changed
- Most dynamic internal memory allocations happen on the heap instead of the stack (#100)

## [3.0.8] - 2018-07-18
### Added
- Added Direction enum, replacing int and defined constants (#77)
### Fixed
- Ensured unused memory is cleared for pentagon children. (#84)
- Fixed compiler warnings in `h3ToGeoHier` and `h3ToGeoBoundaryHier`. (#90)
- Fixed a segfault in `h3SetToLinkedGeo` (#94)
### Changed
- Warnings are not errors by default. (#90)

## [3.0.7] - 2018-06-08
### Added
- Generator for the faceCenterPoint table (#67)
- Generator for number of indexes per resolution (#69)
- Added CMake `WARNINGS_AS_ERRORS` option, default on, for Clang and GCC (#70)
- Added CMake `TEST_WRAPPER` setting when `WRAP_VALGRIND` is on (#66)
### Changed
- Moved Vec3d structure to `vec3d.h` (#67)
- Added input validation when getting the origin or destination of an edge (#74)

## [3.0.6] - 2018-06-01
### Changed
- Changed signature of internal function h3NeighborRotations.
- Do not require support for `hexRange`, `hexRanges`, or `hexRangeDistances` in the bindings. These functions may be deprecated in the public API in future releases; consumers should use `kRing` and `kRingDistances` instead.
- Performance improvement in core indexing logic for `geoToH3` (thanks @wewei!)

## [3.0.5] - 2018-04-27
### Fixed
- Fixed duplicate vertex in h3ToGeoBoundary for certain class III hexagons (#46)
### Added
- Release guide

## [3.0.4] - 2018-04-16
### Added
- Assertions when allocating memory on the heap.
### Changed
- Most internal memory allocations happen on the stack instead of the heap.

## [3.0.3] - 2018-03-19
### Added
- CMake config installation to ease use of h3 from CMake projects.
- Add CMake toolchain file to set build options upfront.
- Add CMake `C_CLANG_TIDY` property to integrate clang-tidy checks during build.
- YouCompleteMe completion support.
### Changed
- Improved resilience to out of range inputs.
- Minimum CMake version to 3.1.
- Set `CMAKE_C_STANDARD` to `11` instead of using compiler flag `-std=c1x`.
- Use `CMAKE_POSITION_INDEPENDENT_CODE` instead of using compiler flag `-fPIC`.
- Rename library target from `h3.1` to `h3` with `SONAME` property of 1.
### Removed
- CMake `H3_COMPILER` option in favor of built-in `CMAKE_C_COMPILER`.
- CMake `BUILD_STATIC` option in favor of built-in `BUILD_SHARED_LIBS`.
- CMake `ENABLE_TESTS` option in favor of built-in `BUILD_TESTING`.
- File globbing in CMake in favor of listing source files.

## [3.0.2] - 2018-01-24
### Removed
- Removed the `H3IndexFat` internal structure.
### Changed
- Simplified build process on Windows.
### Fixed
- Fixed an issue where tests would fail on ARM systems.

## [3.0.1] - 2018-01-18
### Added
- Added a `make install` target.
### Changed
- Improved compatability with building on Windows.
- Fixed various cases where the test suite could crash or not compile.

## [3.0.0] - 2018-01-08
### Added
- First public release.
