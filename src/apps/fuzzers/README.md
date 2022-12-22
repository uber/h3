# Fuzzer harnesses for H3

This directory contains helper programs for testing the H3 library using the
[American fuzzy lop](https://lcamtuf.coredump.cx/afl/)/
[AFL++](https://github.com/AFLplusplus/AFLplusplus) or
[libFuzzer](https://www.llvm.org/docs/LibFuzzer.html) fuzzers.
Fuzzing is a technique for discovering crashes and other edge cases in code
such as the H3 core library.

# Function coverage

The public API of H3 is covered in the following fuzzers:

| Function | File or status
| -------- | --------------
| latLngToCell | [fuzzerLatLngToCell](./fuzzerLatLngToCell.c)
| cellToLatLng |  [fuzzerCellToLatLng](./fuzzerCellToLatLng.c)
| cellToBoundary | [fuzzerCellToLatLng](./fuzzerCellToLatLng.c)
| gridDisk | [fuzzerGridDisk](./fuzzerGridDisk.c)
| gridDiskDistances | [fuzzerGridDisk](./fuzzerGridDisk.c)
| gridRingUnsafe | [fuzzerGridDisk](./fuzzerGridDisk.c)
| gridDisksUnsafe | [fuzzerGridDisk](./fuzzerGridDisk.c)
| polygonToCells | [fuzzerPoylgonToCells](./fuzzerPolygonToCells.c)
| h3SetToMultiPolygon | [fuzzerH3SetToLinkedGeo](./fuzzerH3SetToLinkedGeo.c)
| degsToRads | Trivial
| radsToDegs | Trivial
| distance | [fuzzerDistances](./fuzzerDistances.c)
| getHexagonAreaAvg | [fuzzerResolutions](./fuzzerResolutions.c)
| cellArea | [fuzzerCellArea](./fuzzerCellArea.c)
| getHexagonEdgeLengthAvg | [fuzzerResolutions](./fuzzerResolutions.c)
| edgeLength | [fuzzerEdgeLength](./fuzzerEdgeLength.c)
| getNumCells | [fuzzerResolutions](./fuzzerResolutions.c)
| getRes0Cells | Trivial
| getPentagons | [fuzzerResolutions](./fuzzerResolutions.c)
| getResolution | [fuzzerCellProperties](./fuzzerCellProperties.c)
| getBaseCellNumber | [fuzzerCellProperties](./fuzzerCellProperties.c)
| stringToH3 | [fuzzerIndexIO](./fuzzerIndexIO.c)
| h3ToString | [fuzzerIndexIO](./fuzzerIndexIO.c)
| isValidCell | [fuzzerCellProperties](./fuzzerCellProperties.c)
| cellToParent | [fuzzerHierarchy](./fuzzerHierarchy.c)
| cellToChildren | [fuzzerHierarchy](./fuzzerHierarchy.c)
| cellToCenterChild | [fuzzerHierarchy](./fuzzerHierarchy.c)
| compactCells | [fuzzerCompact](./fuzzerCompact.c)
| uncompactCells | [fuzzerCompact](./fuzzerCompact.c)
| isResClassIII | [fuzzerCellProperties](./fuzzerCellProperties.c)
| isPentagon | [fuzzerCellProperties](./fuzzerCellProperties.c)
| getIcosahedronFaces | [fuzzerCellProperties](./fuzzerCellProperties.c)
| areNeighborCells | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| cellsToDirectedEdge | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| isValidDirectedEdge | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| getDirectedEdgeOrigin | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| getDirectedEdgeDestination | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| directedEdgeToCells | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| originToDirectedEdges | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| directedEdgeToBoundary | [fuzzerDirectedEdge](./fuzzerDirectedEdge.c)
| cellToVertex | [fuzzerVertexes](./fuzzerVertexes.c)
| cellToVertexes | [fuzzerVertexes](./fuzzerVertexes.c)
| vertexToLatLng | [fuzzerVertexes](./fuzzerVertexes.c)
| isValidVertex | [fuzzerVertexes](./fuzzerVertexes.c)
| gridDistance | [fuzzerLocalIj](./fuzzerLocalIj.c)
| gridPathCells | [fuzzerLocalIj](./fuzzerLocalIj.c)
| cellToLocalIj | [fuzzerLocalIj](./fuzzerLocalIj.c)
| localIjToCell | [fuzzerLocalIj](./fuzzerLocalIj.c)
| cellToChildPos| [fuzzerCellToChildPos](./fuzzerCellToChildPos.c)
| childPosToCell| [fuzzerCellToChildPos](./fuzzerCellToChildPos.c)

## Internal function coverage

In addition to the public API, the following internal functions of H3 are covered in fuzzers:

| Function | File
| -------- | ----
| h3NeighborRotations | [fuzzerInternalAlgos](./fuzzerInternalAlgos.c)
| directionForNeighbor | [fuzzerInternalAlgos](./fuzzerInternalAlgos.c)
| h3SetToVertexGraph | [fuzzerInternalAlgos](./fuzzerInternalAlgos.c)
| _vertexGraphToLinkedGeo | [fuzzerInternalAlgos](./fuzzerInternalAlgos.c)
| _upAp7Checked | [fuzzerInternalCoordIjk](./fuzzerInternalCoordIjk.c)
| _upAp7rChecked | [fuzzerInternalCoordIjk](./fuzzerInternalCoordIjk.c)
| _ijkNormalizeCouldOverflow | [fuzzerInternalCoordIjk](./fuzzerInternalCoordIjk.c)
| _ijkNormalize | [fuzzerInternalCoordIjk](./fuzzerInternalCoordIjk.c)

# libFuzzer Usage

libFuzzer is one of the supported fuzzing drivers.

This is the fuzzer used in [oss-fuzz](https://github.com/google/oss-fuzz/tree/master/projects/h3).

## Build

You must build H3 with Clang and enable support for libFuzzer.

```
CC=clang cmake -DENABLE_LIBFUZZER=ON .
make fuzzers
```

## Run

To begin fuzzing, execute the fuzzer binary:

```
fuzzerLatLngToCell
```

For command line options including how to specify a test corpus, consult the [libFuzzer documentation](https://www.llvm.org/docs/LibFuzzer.html#options).

# AFL Usage

AFL/AFL++ are supported fuzzing drivers.

## Installation

```
apt install afl-clang
```

(There is also an afl-cov which looks interesting but isn't necessary.)

## Build

You must compile with the instrumented compiler:

```
CXX=afl-clang++ CC=afl-clang cmake .
make fuzzers
```

Generate a blank (zeroed) test case file. This will not be very a interesting test case but is usedful
for having files of the right size.

```
fuzzerLatLngToCell --generate bytes24
```

An individual fuzzer run is invoked as follows. The argument is a file containing the number of bytes needed.

```
fuzzerLatLngToCell bytes24
```

## Run

To begin running the fuzzer, run the following. The testcase directory (`testcase_dir`) should contain a file
with at least the right number of bytes that the fuzzer will read (this can be generated using the `--generate`
option above.)

```
afl-fuzz -i testcase_dir -o findings_dir -- fuzzerLatLngToCell @@
```
