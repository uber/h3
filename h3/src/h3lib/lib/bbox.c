/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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
/** @file bbox.c
 * @brief   Geographic bounding box functions
 */

#include "bbox.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include "constants.h"
#include "geoCoord.h"
#include "h3Index.h"

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

/**
 * Whether the given bounding box crosses the antimeridian
 * @param  bbox Bounding box to inspect
 * @return      is transmeridian
 */
bool bboxIsTransmeridian(const BBox* bbox) { return bbox->east < bbox->west; }

/**
 * Get the center of a bounding box
 * @param bbox   Input bounding box
 * @param center Output center coordinate
 */
void bboxCenter(const BBox* bbox, GeoCoord* center) {
    center->lat = (bbox->north + bbox->south) / 2.0;
    // If the bbox crosses the antimeridian, shift east 360 degrees
    double east = bboxIsTransmeridian(bbox) ? bbox->east + M_2PI : bbox->east;
    center->lon = constrainLng((east + bbox->west) / 2.0);
}

/**
 * Whether the bounding box contains a given point
 * @param  bbox  Bounding box
 * @param  point Point to test
 * @return       Whether the point is contained
 */
bool bboxContains(const BBox* bbox, const GeoCoord* point) {
    return point->lat >= bbox->south && point->lat <= bbox->north &&
           (bboxIsTransmeridian(bbox) ?
                                      // transmeridian case
                (point->lon >= bbox->west || point->lon <= bbox->east)
                                      :
                                      // standard case
                (point->lon >= bbox->west && point->lon <= bbox->east));
}

/**
 * _hexRadiusKm returns the radius of a given hexagon in Km
 *
 * @param h3Index the index of the hexagon
 * @return the radius of the hexagon in Km
 */
double _hexRadiusKm(H3Index h3Index) {
    // There is probably a cheaper way to determine the radius of a
    // hexagon, but this way is conceptually simple
    GeoCoord h3Center;
    GeoBoundary h3Boundary;
    H3_EXPORT(h3ToGeo)(h3Index, &h3Center);
    H3_EXPORT(h3ToGeoBoundary)(h3Index, &h3Boundary);
    return _geoDistKm(&h3Center, h3Boundary.verts);
}

/**
 * Get the radius of the bbox in hexagons - i.e. the radius of a k-ring centered
 * on the bbox center and covering the entire bbox.
 * @param  bbox Bounding box to measure
 * @param  res  Resolution of hexagons to use in measurement
 * @return      Radius in hexagons
 */
int bboxHexRadius(const BBox* bbox, int res) {
    // Determine the center of the bounding box
    GeoCoord center;
    bboxCenter(bbox, &center);

    // Use a vertex on the side closest to the equator, to ensure the longest
    // radius in cases with significant distortion. East/west is arbitrary.
    double lat =
        fabs(bbox->north) > fabs(bbox->south) ? bbox->south : bbox->north;
    GeoCoord vertex = {lat, bbox->east};

    // Determine the length of the bounding box "radius" to then use
    // as a circle on the earth that the k-rings must be greater than
    double bboxRadiusKm = _geoDistKm(&center, &vertex);

    // Determine the radius of the center hexagon
    double centerHexRadiusKm = _hexRadiusKm(H3_EXPORT(geoToH3)(&center, res));

    // The closest point along a hexagon drawn through the center points
    // of a k-ring aggregation is exactly 1.5 radii of the hexagon. For
    // any orientation of the GeoJSON encased in a circle defined by the
    // bounding box radius and center, it is guaranteed to fit in this k-ring
    // Rounded *up* to guarantee containment
    return (int)ceil(bboxRadiusKm / (1.5 * centerHexRadiusKm));
}
