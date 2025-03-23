#ifndef POLYGON_TO_CELLS_H
#define POLYGON_TO_CELLS_H

#include "h3api.h"
#include "latLng.h"
#include "bbox.h"

// Constants for polygon processing
#define MAX_POLYGON_SEGMENTS 10
#define MAX_POLYGON_CELLS 10000
#define POLE_THRESHOLD 0.0001  // Radians from pole to trigger special handling

// Structure for polygon segment processing
typedef struct {
    LatLng* vertices;
    int numVertices;
    bool crossesPole;
    BBox bbox;
} PolygonSegment;

/**
 * Validates polygon coordinates and structure
 * @param polygon Input polygon to validate
 * @return E_SUCCESS if valid, error code otherwise
 */
H3Error validatePolygonCoordinates(const LatLngPoly* polygon);

/**
 * Normalizes polygon coordinates to standard ranges
 * @param polygon Polygon to normalize
 * @return E_SUCCESS if successful
 */
H3Error normalizePolygonCoordinates(LatLngPoly* polygon);

/**
 * Checks if a point is near a pole
 * @param point Point to check
 * @return true if near pole
 */
bool isNearPole(const LatLng* point);

/**
 * Splits a polygon into segments at pole crossings
 * @param polygon Input polygon
 * @param segments Output segments
 * @param numSegments Number of segments created
 * @return E_SUCCESS if successful
 */
H3Error splitPolygonAtPoles(const LatLngPoly* polygon, 
                           PolygonSegment* segments,
                           int* numSegments);

/**
 * Processes a single polygon segment
 * @param segment Segment to process
 * @param res H3 resolution
 * @param out Output cells
 * @param numCells Number of cells generated
 * @return E_SUCCESS if successful
 */
H3Error processPolygonSegment(const PolygonSegment* segment,
                             int res,
                             H3Index* out,
                             int* numCells);

#endif // POLYGON_TO_CELLS_H