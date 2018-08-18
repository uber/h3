/*
 * Copyright 2018 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file polygon.c
 * @brief Polygon algorithms
 */

#include "polygon.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include "bbox.h"
#include "constants.h"
#include "geoCoord.h"
#include "h3api.h"

/**
 * Normalize a longitude value, converting it into a comparable value in
 * transmeridian cases.
 * @param  lng Longitude to normalize
 * @param  isTransmeridian Whether this longitude is part of a transmeridian
 *                         polygon
 * @return Normalized longitude
 */
double _normalizeLng(double lng, bool isTransmeridian) {
    return isTransmeridian && lng < 0 ? lng + M_2PI : lng;
}

/**
 * loopContainsPoint is the core loop of the point-in-poly
 * algorithm, working on a Geofence struct
 *
 * @param geofence The geofence to check
 * @param bbox The bbox for the loop being tested
 * @param coord The coordinate to check if contained by the geofence
 * @return true or false
 */
bool _loopContainsPoint(const IterableGeoLoop* loop, const BBox* bbox,
                        const GeoCoord* coord) {
    // fail fast if we're outside the bounding box
    if (!bboxContains(bbox, coord)) {
        return false;
    }
    bool isTransmeridian = bboxIsTransmeridian(bbox);
    bool contains = false;

    double lat = coord->lat;
    double lng = _normalizeLng(coord->lon, isTransmeridian);

    GeoCoord a;
    GeoCoord b;

    INIT_ITERATION;

    while (true) {
        ITERATE(loop, a, b);

        // Ray casting algo requires the second point to always be higher
        // than the first, so swap if needed
        if (a.lat > b.lat) {
            GeoCoord tmp = a;
            a = b;
            b = tmp;
        }

        // If we're totally above or below the latitude ranges, the test
        // ray cannot intersect the line segment, so let's move on
        if (lat < a.lat || lat > b.lat) {
            continue;
        }

        double aLng = _normalizeLng(a.lon, isTransmeridian);
        double bLng = _normalizeLng(b.lon, isTransmeridian);

        // Rays are cast in the longitudinal direction, in case a point
        // exactly matches, to decide tiebreakers, bias westerly
        if (aLng == lng || bLng == lng) {
            lng -= DBL_EPSILON;
        }

        // For the latitude of the point, compute the longitude of the
        // point that lies on the line segment defined by a and b
        // This is done by computing the percent above a the lat is,
        // and traversing the same percent in the longitudinal direction
        // of a to b
        double ratio = (lat - a.lat) / (b.lat - a.lat);
        double testLng =
            _normalizeLng(aLng + (bLng - aLng) * ratio, isTransmeridian);

        // Intersection of the ray
        if (testLng > lng) {
            contains = !contains;
        }
    }

    return contains;
}

bool geofenceContainsPoint(const Geofence* geofence, const BBox* bbox,
                           const GeoCoord* coord) {
    IterableGeoLoop loop;
    loop.type = TYPE_GEOFENCE;
    loop.geofence = geofence;
    return _loopContainsPoint(&loop, bbox, coord);
}

/**
 * polygonContainsPoint takes a given GeoPolygon data structure and
 * checks if it contains a given geo coordinate.
 *
 * @param geoPolygon The geofence and holes defining the relevant area
 * @param bboxes The bboxes for the main geofence and each of its holes
 * @param coord The coordinate to check if contained by the geoJson-like
 * struct
 * @return true or false
 */
bool polygonContainsPoint(const GeoPolygon* geoPolygon, const BBox* bboxes,
                          const GeoCoord* coord) {
    // Start with contains state of primary geofence
    bool contains =
        geofenceContainsPoint(&(geoPolygon->geofence), &bboxes[0], coord);

    // If the point is contained in the primary geofence, but there are holes in
    // the geofence iterate through all holes and return false if the point is
    // contained in any hole
    if (contains && geoPolygon->numHoles > 0) {
        for (int i = 0; i < geoPolygon->numHoles; i++) {
            if (geofenceContainsPoint(&(geoPolygon->holes[i]), &bboxes[i + 1],
                                      coord)) {
                return false;
            }
        }
    }

    return contains;
}

/**
 * Create a bounding box from a simple polygon defined as an array of vertices.
 * Known limitations:
 * - Does not support polygons with two adjacent points > 180 degrees of
 *   longitude apart. These will be interpreted as crossing the antimeridian.
 * - Does not currently support polygons containing a pole.
 * @param verts    Array of vertices
 * @param numVerts Number of vertices
 * @param bbox     Output bbox
 */
void bboxFromVertices(const GeoCoord* verts, int numVerts, BBox* bbox) {
    // Early exit if there are no vertices
    if (numVerts == 0) {
        bbox->north = 0;
        bbox->south = 0;
        bbox->east = 0;
        bbox->west = 0;
        return;
    }
    double lat;
    double lon;

    bbox->south = DBL_MAX;
    bbox->west = DBL_MAX;
    bbox->north = -1.0 * DBL_MAX;
    bbox->east = -1.0 * DBL_MAX;
    bool isTransmeridian = false;

    for (int i = 0; i < numVerts; i++) {
        lat = verts[i].lat;
        lon = verts[i].lon;
        if (lat < bbox->south) bbox->south = lat;
        if (lon < bbox->west) bbox->west = lon;
        if (lat > bbox->north) bbox->north = lat;
        if (lon > bbox->east) bbox->east = lon;
        // check for arcs > 180 degrees longitude, flagging as transmeridian
        if (fabs(lon - verts[(i + 1) % numVerts].lon) > M_PI) {
            isTransmeridian = true;
        }
    }
    // Swap east and west if transmeridian
    if (isTransmeridian) {
        double tmp = bbox->east;
        bbox->east = bbox->west;
        bbox->west = tmp;
    }
}

/**
 * Create a bounding box from a Geofence
 * @param geofence Input Geofence
 * @param bbox     Output bbox
 */
void bboxFromGeofence(const Geofence* geofence, BBox* bbox) {
    bboxFromVertices(geofence->verts, geofence->numVerts, bbox);
}

/**
 * Create a bounding box from a GeoPolygon
 * @param polygon Input GeoPolygon
 * @param bboxes  Output bboxes, one for the outer loop and one for each hole
 */
void bboxesFromGeoPolygon(const GeoPolygon* polygon, BBox* bboxes) {
    bboxFromGeofence(&polygon->geofence, &bboxes[0]);
    for (int i = 0; i < polygon->numHoles; i++) {
        bboxFromGeofence(&polygon->holes[i], &bboxes[i + 1]);
    }
}
