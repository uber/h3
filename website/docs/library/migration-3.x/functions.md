---
id: functions
title: Function name changes
sidebar_label: Function name changes
slug: /library/migration-3.x/functions
---

The following function and structure names changed from 3.x to 4.0.0:

### General Function Names

|            3.x name.          |      4.0.0 name       |
|-------------------------------|-----------------------|
| *Does Not Exist (DNE)*        | `isValidIndex`        |
| `h3IsValid`                   | `isValidCell`         |
| `h3UnidirectionalEdgeIsValid` | `isValidDirectedEdge` |
| `h3IsPentagon`                | `isPentagon`          |
| `h3IsResClassIII`             | `isResClassIII`       |
| `h3IndexesAreNeighbors`       | `areNeighborCells`    |
| `h3ToParent`                  | `cellToParent`        |
| `h3ToCenterChild`             | `cellToCenterChild`   |
| `h3ToChildren`                | `cellToChildren`      |
| `numHexagons`                 | `getNumCells`         |
| `getRes0Indexes`              | `getRes0Cells`        |
| `getPentagonIndexes`          | `getPentagons`        |
| `h3GetBaseCell`               | `getBaseCellNumber`   |
| `h3GetResolution`             | `getResolution`       |
| *DNE*                         | `getMode`             |
| `h3GetFaces`                  | `getIcosahedronFaces` |
| `geoToH3`                     | `latLngToCell`        |
| `h3ToGeo`                     | `cellToLatLng`        |
| `compact`                     | `compactCells`        |
| `uncompact`                   | `uncompactCells`      |
| `polyfill`                    | `polygonToCells`      |

**Note**: `getResolution` and `getBaseCellNumber` should work for both cells and edges.


### H3 Grid Functions

Many of these functions will have three forms:
- `<func_name>`
- `<func_name>Unsafe`
- `<func_name>Safe`

The `Unsafe` version is fast, but may fail if it encounters a pentagon.
It should return a failure code in this case.

The `Safe` version is slower, but will work in all cases.

The version without either suffix is intended to be the one typically
used.
This version will first attempt the `Unsafe` version, and if
it detects failure, will fall back to the `Safe` version.
Encountering pentagons is rare in most use-cases, so this version
should usually be equivalent to the fast version, but with a guarantee
that it will not fail.

Initially, we **will not** expose the `Safe` versions to users in the API.
We may expose them in the future if a need becomes clear.


#### Distance

|   3.x name.  |       4.0.0 name        |
|--------------|-------------------------|
| `h3Distance` | `gridDistance`          |
| `h3Line`     | `gridPathCells`         |
| *DNE*        | `gridPathEdges`         |
| *DNE*        | `gridPathDirectedEdges` |


#### Filled-In Disk With Distances

|       3.x name.     |        4.0.0 name         |                 Calls                 |
|---------------------|---------------------------|---------------------------------------|
| `hexRangeDistances` | `gridDiskDistancesUnsafe` | NONE                                  |
| `_kRingInternal`    | `gridDiskDistancesSafe`   | NONE                                  |
| `kRingDistances`    | `gridDiskDistances`       | `hexRangeDistances`, `_kRingInternal` |


#### Filled-In Disk Without Distances

|   3.x name.  |    4.0.0 name     |        Calls        |
|--------------|-------------------|---------------------|
| `hexRange`   | `gridDiskUnsafe`  | `hexRangeDistances` |
| *DNE*        | `gridDiskSafe`    |                     |
| `kRing`      | `gridDisk`        | `kRingDistances`    |
| `hexRanges`  | `gridDisksUnsafe` | N x `hexRange`      |

#### Hollow Ring

|   3.x name.  |   4.0.0 name     |              Calls               |
|--------------|------------------|----------------------------------|
| `hexRing`    | `gridRingUnsafe` | NONE                             |
| *DNE*        | `gridRingSafe`   | `gridDiskDistancesSafe`          |
| *DNE*        | `gridRing`       | `gridRingUnsafe`, `gridRingSafe` |

#### Local IJ

|         3.x name          |   4.0.0 name    |
|---------------------------|-----------------|
| `experimentalLocalIjToH3` | `localIjToCell` |
| `experimentalH3ToLocalIj` | `cellToLocalIj` |

### H3 Edge Types

Instead of `UnidirectionalEdge`, use the term `DirectedEdge`.

For a future undirected edge mode, use the term `Edge`.

|                    3.x name.                  |         4.0.0 name           |
|-----------------------------------------------|------------------------------|
| `h3UnidirectionalEdgeIsValid`                 | `isValidDirectedEdge`        |
| `getH3UnidirectionalEdge`                     | `cellsToDirectedEdge`        |
| `getH3IndexesFromUnidirectionalEdge`          | `directedEdgeToCells`        |
| `getH3UnidirectionalEdgesFromHexagon`         | `originToDirectedEdges`      |
| *DNE*                                         | `destinationToDirectedEdges` |
| `getH3UnidirectionalEdgeBoundary`             | `directedEdgeToBoundary`     |
| `getOriginH3IndexFromUnidirectionalEdge`      | `getDirectedEdgeOrigin`      |
| `getDestinationH3IndexFromUnidirectionalEdge` | `getDirectedEdgeDestination` |


### Area/Length Functions

|    3.x name.          |         4.0.0 name          |
|-----------------------|-----------------------------|
| `hexAreaKm2`          | `getHexagonAreaAvgKm2`      |
| `hexAreaM2`           | `getHexagonAreaAvgM2`       |
| `edgeLengthKm`        | `getHexagonEdgeLengthAvgKm` |
| `edgeLengthM`         | `getHexagonEdgeLengthAvgM`  |
| *DNE*                 | `getPentagonAreaAvg*`       |
| *DNE*                 | `getPentagonEdgeLengthAvg*` |
| *DNE*                 | `cellAreaKm2`               |
| *DNE*                 | `cellAreaM2`                |
| `pointDistKm`         | `greatCircleDistanceKm`     |
| `pointDistM`          | `greatCircleDistanceM`      |
| `pointDistRads`       | `greatCircleDistanceRads`   |
| `exactEdgeLengthRads` | `edgeLengthRads`            |
| `exactEdgeLengthKm`   | `edgeLengthKm`              |
| `exactEdgeLengthM`    | `edgeLengthM`               |

**Note**: `cellAreaKm2` and `cellAreaM2` would return the actual area of
the passed-in cell.

## Polygons

### Data Structures

- rename `GeoBoundary` to `CellBoundary` to indicate it is space-limited to describing the geometry of cells

|      3.x name     |    4.0.0 name     |               Notes               |
|-------------------|-------------------|-----------------------------------|
| `GeoBoundary`     | `CellBoundary`    | <= 10 stack-allocated `LatLng`s   |
| `GeoCoord`        | `LatLng`          |                                   |
| `Geofence`        | `GeoLoop`         | heap-allocated `LatLng`s          |
| `GeoPolygon`      | `GeoPolygon`      |                                   |
| `GeoMultiPolygon` | `GeoMultiPolygon` | currently not used                |


### Functions

|              3.x name             |       4.0.0 name            |         Notes              |
|-----------------------------------|-----------------------------|----------------------------|
| `h3ToGeoBoundary`                 | `cellToBoundary`            | returns `CellBoundary`     |
| *DNE*                             | `cellToLoop`                | returns `GeoLoop`          |
| *DNE*                             | `loopToBoundary`            |                            |
| *DNE*                             | `boundaryToLoop`            |                            |
| `getH3UnidirectionalEdgeBoundary` | `directedEdgeToBoundary`    | returns `CellBoundary`     |
| `h3SetToLinkedGeo`                | `cellsToLinkedMultiPolygon` | returns `LinkedGeoPolygon` |
| `h3SetToMultiPolygon`             | `cellsToMultiPolygon`       | bindings only              |
