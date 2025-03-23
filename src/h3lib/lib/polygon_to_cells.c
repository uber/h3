#include "polygon_to_cells.h"
#include "h3api.h"
#include "latLng.h"
#include "bbox.h"
#include "constants.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

H3Error validatePolygonCoordinates(const LatLngPoly* polygon) {
    if (!polygon || polygon->numVerts < 3) {
        return E_FAILED;
    }

    for (int i = 0; i < polygon->numVerts; i++) {
        // Check latitude range (-90 to 90 degrees)
        if (fabs(polygon->verts[i].lat) > M_PI_2) {
            return E_FAILED;
        }
        // Check longitude range (-180 to 180 degrees)
        if (fabs(polygon->verts[i].lng) > M_PI) {
            return E_FAILED;
        }
    }

    return E_SUCCESS;
}

H3Error normalizePolygonCoordinates(LatLngPoly* polygon) {
    if (!polygon) return E_FAILED;

    for (int i = 0; i < polygon->numVerts; i++) {
        // Normalize latitude to [-90, 90]
        polygon->verts[i].lat = fmax(-M_PI_2, fmin(M_PI_2, polygon->verts[i].lat));
        
        // Normalize longitude to [-180, 180]
        while (polygon->verts[i].lng > M_PI) {
            polygon->verts[i].lng -= 2 * M_PI;
        }
        while (polygon->verts[i].lng < -M_PI) {
            polygon->verts[i].lng += 2 * M_PI;
        }
    }

    return E_SUCCESS;
}

bool isNearPole(const LatLng* point) {
    return fabs(fabs(point->lat) - M_PI_2) < POLE_THRESHOLD;
}

H3Error splitPolygonAtPoles(const LatLngPoly* polygon, 
                           PolygonSegment* segments,
                           int* numSegments) {
    *numSegments = 0;
    if (!polygon || !segments) return E_FAILED;

    LatLng* currentVertices = malloc(polygon->numVerts * sizeof(LatLng));
    int currentCount = 0;
    bool inSegment = false;

    for (int i = 0; i < polygon->numVerts; i++) {
        if (isNearPole(&polygon->verts[i])) {
            if (inSegment && currentCount > 2) {
                // Complete current segment
                segments[*numSegments].vertices = malloc(currentCount * sizeof(LatLng));
                memcpy(segments[*numSegments].vertices, currentVertices, 
                       currentCount * sizeof(LatLng));
                segments[*numSegments].numVertices = currentCount;
                segments[*numSegments].crossesPole = true;
                (*numSegments)++;
            }
            currentCount = 0;
            inSegment = false;
        }

        currentVertices[currentCount++] = polygon->verts[i];
        inSegment = true;

        if (currentCount == polygon->numVerts || i == polygon->numVerts - 1) {
            if (currentCount > 2) {
                segments[*numSegments].vertices = malloc(currentCount * sizeof(LatLng));
                memcpy(segments[*numSegments].vertices, currentVertices, 
                       currentCount * sizeof(LatLng));
                segments[*numSegments].numVertices = currentCount;
                segments[*numSegments].crossesPole = false;
                (*numSegments)++;
            }
        }
    }

    free(currentVertices);
    return E_SUCCESS;
}

H3Error processPolygonSegment(const PolygonSegment* segment,
                             int res,
                             H3Index* out,
                             int* numCells) {
    if (!segment || !out || !numCells) return E_FAILED;

    // Calculate bounding box for the segment
    BBox bbox;
    bbox.north = -M_PI_2;
    bbox.south = M_PI_2;
    bbox.east = -M_PI;
    bbox.west = M_PI;

    for (int i = 0; i < segment->numVertices; i++) {
        bbox.north = fmax(bbox.north, segment->vertices[i].lat);
        bbox.south = fmin(bbox.south, segment->vertices[i].lat);
        bbox.east = fmax(bbox.east, segment->vertices[i].lng);
        bbox.west = fmin(bbox.west, segment->vertices[i].lng);
    }

    // Special handling for segments that cross poles
    if (segment->crossesPole) {
        if (bbox.north > M_PI_2 - POLE_THRESHOLD) {
            bbox.north = M_PI_2;
        }
        if (bbox.south < -M_PI_2 + POLE_THRESHOLD) {
            bbox.south = -M_PI_2;
        }
    }

    // Get the resolution-specific cell size
    double cellSize = H3_GET_RESOLUTION_SIZE(res);
    
    // Generate cells within the bounding box
    *numCells = 0;
    for (double lat = bbox.south; lat <= bbox.north; lat += cellSize) {
        for (double lng = bbox.west; lng <= bbox.east; lng += cellSize) {
            LatLng center = {lat, lng};
            H3Index cell = latLngToCell(&center, res);
            
            // Check if cell intersects with the polygon segment
            if (cellIntersectsPolygon(cell, segment)) {
                out[(*numCells)++] = cell;
                
                if (*numCells >= MAX_POLYGON_CELLS) {
                    return E_FAILED; // Too many cells
                }
            }
        }
    }

    return E_SUCCESS;
}

// Modified main polygonToCells function
H3Error polygonToCells(const LatLngPoly* polygon, int res, H3Index* out) {
    H3Error err = validatePolygonCoordinates(polygon);
    if (err) return err;

    // Create a mutable copy for normalization
    LatLngPoly* normalizedPoly = malloc(sizeof(LatLngPoly));
    memcpy(normalizedPoly, polygon, sizeof(LatLngPoly));
    
    err = normalizePolygonCoordinates(normalizedPoly);
    if (err) {
        free(normalizedPoly);
        return err;
    }

    // Check for pole proximity
    bool hasPoleProximity = false;
    for (int i = 0; i < normalizedPoly->numVerts; i++) {
        if (isNearPole(&normalizedPoly->verts[i])) {
            hasPoleProximity = true;
            break;
        }
    }

    if (hasPoleProximity) {
        // Handle pole-crossing case
        PolygonSegment segments[MAX_POLYGON_SEGMENTS];
        int numSegments;
        
        err = splitPolygonAtPoles(normalizedPoly, segments, &numSegments);
        if (err) {
            free(normalizedPoly);
            return err;
        }

        // Process each segment
        int totalCells = 0;
        for (int i = 0; i < numSegments; i++) {
            int segmentCells = 0;
            err = processPolygonSegment(&segments[i], res, 
                                      &out[totalCells], &segmentCells);
            
            if (err) {
                // Clean up
                for (int j = 0; j <= i; j++) {
                    free(segments[j].vertices);
                }
                free(normalizedPoly);
                return err;
            }
            
            totalCells += segmentCells;
            free(segments[i].vertices);
        }
    } else {
        // Standard processing for non-pole-crossing polygons
        PolygonSegment segment = {
            .vertices = normalizedPoly->verts,
            .numVertices = normalizedPoly->numVerts,
            .crossesPole = false
        };
        
        int numCells = 0;
        err = processPolygonSegment(&segment, res, out, &numCells);
        if (err) {
            free(normalizedPoly);
            return err;
        }
    }

    free(normalizedPoly);
    return E_SUCCESS;
}