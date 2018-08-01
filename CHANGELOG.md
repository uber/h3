# Change Log
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).

The public API of this library consists of the functions declared in file
[h3api.h](./src/h3lib/include/h3api.h).

## [Unreleased]

## [3.1.0] - 2018-08-02
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
