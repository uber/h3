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
#include <stdbool.h>
#include "bbox.h"
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
bool loopContainsPoint(const Geofence* geofence, const BBox* bbox,
                       const GeoCoord* coord) {
    // fail fast if we're outside the bounding box
    if (!bboxContains(bbox, coord)) {
        return false;
    }
    bool isTransmeridian = bboxIsTransmeridian(bbox);
    bool contains = false;

    double lat = coord->lat;
    double lng = _normalizeLng(coord->lon, isTransmeridian);

    for (int i = 0; i < geofence->numVerts; i++) {
        GeoCoord a = geofence->verts[i];
        GeoCoord b;
        if (i + 1 == geofence->numVerts) {
            b = geofence->verts[0];
        } else {
            b = geofence->verts[i + 1];
        }

        // Ray casting algo requires the second point to always be higher
        // than the first, so swap if needed
        if (a.lat > b.lat) {
            a = b;
            b = geofence->verts[i];
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
        };
    }

    return contains;
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
        loopContainsPoint(&(geoPolygon->geofence), &bboxes[0], coord);

    // If the point is contained in the primary geofence, but there are holes in
    // the geofence iterate through all holes and return false if the point is
    // contained in any hole
    if (contains && geoPolygon->numHoles > 0) {
        for (int i = 0; i < geoPolygon->numHoles; i++) {
            if (loopContainsPoint(&(geoPolygon->holes[i]), &bboxes[i + 1],
                                  coord)) {
                return false;
            }
        }
    }

    return contains;
}