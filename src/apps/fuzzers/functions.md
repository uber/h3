This file is an index to where all the functions in the H3 public API are fuzzed.

| Function | File or status
| -------- | --------------
| latLngToCell | [fuzzerLatLngToCell](./fuzzerLatLngToCell.c)
| cellToLatLng |  [fuzzerCellToLatLng](./fuzzerCellToLatLng.c)
| cellToBoundary | [fuzzerCellToLatLng](./fuzzerCellToLatLng.c)
| gridDisk | [fuzzerGridDisk](./fuzzerGridDisk.c)
| gridDiskDistances | [fuzzerGridDisk](./fuzzerGridDisk.c)
| gridRingUnsafe | [fuzzerGridDisk](./fuzzerGridDisk.c)
| polygonToCells | 
| h3SetToMultiPolygon | [fuzzerH3SetToLinkedGeo](./fuzzerH3SetToLinkedGeo.c)
| degsToRads | Trivial
| radsToDegs | Trivial
| distance | [fuzzerDistances](./fuzzerDistances.c)
| getHexagonAreaAvg | Trivial
| cellArea | [fuzzerCellArea](./fuzzerCellArea.c)
| getHexagonEdgeLengthAvg | Trivial
| exactEdgeLength | [fuzzerExactEdgeLength](./fuzzerExactEdgeLength.c)
| getNumCells | Trivial
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
| compactCells | 
| uncompactCells | 
| isResClassIII | [fuzzerCellProperties](./fuzzerCellProperties.c)
| isPentagon | [fuzzerCellProperties](./fuzzerCellProperties.c)
| getIcosahedronFaces | [fuzzerCellProperties](./fuzzerCellProperties.c)
| areNeighborCells | 
| cellsToDirectedEdge | 
| isValidDirectedEdge | 
| getDirectedEdgeOrigin | 
| getDirectedEdgeDestination | 
| directedEdgeToCells | 
| originToDirectedEdges | 
| directedEdgeToBoundary | 
| cellToVertex | [fuzzerVertexes](./fuzzerVertexes.c)
| cellToVertexes | [fuzzerVertexes](./fuzzerVertexes.c)
| vertexToLatLng | [fuzzerVertexes](./fuzzerVertexes.c)
| isValidVertex | [fuzzerVertexes](./fuzzerVertexes.c)
| gridDistance | 
| gridPathCells | 
| experimentalH3ToLocalIj | 
| experimentalLocalIjToH3 | 