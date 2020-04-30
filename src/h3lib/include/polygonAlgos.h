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
/** @file
 * @brief Include file for poylgon algorithms. This includes the core
 *        logic for algorithms acting over loops of coordinates,
 *        allowing them to be reused for both Geofence and
 *        LinkegGeoLoop structures. This file is intended to be
 *        included inline in a file that defines the type-specific
 *        macros required for iteration.
 */

#include <float.h>
#include <math.h>
#include <stdbool.h>

#include "bbox.h"
#include "constants.h"
#include "geoCoord.h"
#include "h3api.h"
#include "linkedGeo.h"
#include "polygon.h"

#ifndef TYPE
#error "TYPE must be defined before including this header"
#endif

#ifndef IS_EMPTY
#error "IS_EMPTY must be defined before including this header"
#endif

#ifndef INIT_ITERATION
#error "INIT_ITERATION must be defined before including this header"
#endif

#ifndef ITERATE
#error "ITERATE must be defined before including this header"
#endif

#define LOOP_ALGO_XTJOIN(a, b) a##b
#define LOOP_ALGO_TJOIN(a, b) LOOP_ALGO_XTJOIN(a, b)
#define GENERIC_LOOP_ALGO(func) LOOP_ALGO_TJOIN(func, TYPE)

/** Macro: Normalize longitude, dealing with transmeridian arcs */
#define NORMALIZE_LON(lon, isTransmeridian) \
    (isTransmeridian && lon < 0 ? lon + (double)M_2PI : lon)

/**
 * pointInside is the core loop of the point-in-poly algorithm
 * @param loop  The loop to check
 * @param bbox  The bbox for the loop being tested
 * @param coord The coordinate to check
 * @return      Whether the point is contained
 */
bool GENERIC_LOOP_ALGO(pointInside)(const TYPE* loop, const BBox* bbox,
                                    const GeoCoord* coord) {
    // fail fast if we're outside the bounding box
    if (!bboxContains(bbox, coord)) {
        return false;
    }
    bool isTransmeridian = bboxIsTransmeridian(bbox);
    bool contains = false;

    double lat = coord->lat;
    double lng = NORMALIZE_LON(coord->lon, isTransmeridian);

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

        double aLng = NORMALIZE_LON(a.lon, isTransmeridian);
        double bLng = NORMALIZE_LON(b.lon, isTransmeridian);

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
            NORMALIZE_LON(aLng + (bLng - aLng) * ratio, isTransmeridian);

        // Intersection of the ray
        if (testLng > lng) {
            contains = !contains;
        }
    }

    return contains;
}

/**
 * Create a bounding box from a simple polygon loop.
 * Known limitations:
 * - Does not support polygons with two adjacent points > 180 degrees of
 *   longitude apart. These will be interpreted as crossing the antimeridian.
 * - Does not currently support polygons containing a pole.
 * @param loop     Loop of coordinates
 * @param bbox     Output bbox
 */
void GENERIC_LOOP_ALGO(bboxFrom)(const TYPE* loop, BBox* bbox) {
    // Early exit if there are no vertices
    if (IS_EMPTY(loop)) {
        *bbox = (BBox){0};
        return;
    }

    bbox->south = DBL_MAX;
    bbox->west = DBL_MAX;
    bbox->north = -DBL_MAX;
    bbox->east = -DBL_MAX;
    double minPosLon = DBL_MAX;
    double maxNegLon = -DBL_MAX;
    bool isTransmeridian = false;

    double lat;
    double lon;
    GeoCoord coord;
    GeoCoord next;

    INIT_ITERATION;

    while (true) {
        ITERATE(loop, coord, next);

        lat = coord.lat;
        lon = coord.lon;
        if (lat < bbox->south) bbox->south = lat;
        if (lon < bbox->west) bbox->west = lon;
        if (lat > bbox->north) bbox->north = lat;
        if (lon > bbox->east) bbox->east = lon;
        // Save the min positive and max negative longitude for
        // use in the transmeridian case
        if (lon > 0 && lon < minPosLon) minPosLon = lon;
        if (lon < 0 && lon > maxNegLon) maxNegLon = lon;
        // check for arcs > 180 degrees longitude, flagging as transmeridian
        if (fabs(lon - next.lon) > M_PI) {
            isTransmeridian = true;
        }
    }
    // Swap east and west if transmeridian
    if (isTransmeridian) {
        bbox->east = maxNegLon;
        bbox->west = minPosLon;
    }
}

/**
 * Whether the winding order of a given loop is clockwise, with normalization
 * for loops crossing the antimeridian.
 * @param loop              The loop to check
 * @param isTransmeridian   Whether the loop crosses the antimeridian
 * @return                  Whether the loop is clockwise
 */
static bool GENERIC_LOOP_ALGO(isClockwiseNormalized)(const TYPE* loop,
                                                     bool isTransmeridian) {
    double sum = 0;
    GeoCoord a;
    GeoCoord b;

    INIT_ITERATION;
    while (true) {
        ITERATE(loop, a, b);
        // If we identify a transmeridian arc (> 180 degrees longitude),
        // start over with the transmeridian flag set
        if (!isTransmeridian && fabs(a.lon - b.lon) > M_PI) {
            return GENERIC_LOOP_ALGO(isClockwiseNormalized)(loop, true);
        }
        sum += ((NORMALIZE_LON(b.lon, isTransmeridian) -
                 NORMALIZE_LON(a.lon, isTransmeridian)) *
                (b.lat + a.lat));
    }

    return sum > 0;
}

/**
 * Whether the winding order of a given loop is clockwise. In GeoJSON,
 * clockwise loops are always inner loops (holes).
 * @param loop  The loop to check
 * @return      Whether the loop is clockwise
 */
bool GENERIC_LOOP_ALGO(isClockwise)(const TYPE* loop) {
    return GENERIC_LOOP_ALGO(isClockwiseNormalized)(loop, false);
}
