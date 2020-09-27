/*
 * Copyright 2016-2020 Uber Technologies, Inc.
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
 * Whether two bounding boxes are strictly equal
 * @param  b1 Bounding box 1
 * @param  b2 Bounding box 2
 * @return    Whether the boxes are equal
 */
bool bboxEquals(const BBox* b1, const BBox* b2) {
    return b1->north == b2->north && b1->south == b2->south &&
           b1->east == b2->east && b1->west == b2->west;
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
    return H3_EXPORT(pointDistKm)(&h3Center, h3Boundary.verts);
}

/**
 * bboxHexEstimate returns an estimated number of hexagons that fit
 *                 within the cartesian-projected bounding box
 *
 * @param bbox the bounding box to estimate the hexagon fill level
 * @param res the resolution of the H3 hexagons to fill the bounding box
 * @return the estimated number of hexagons to fill the bounding box
 */
int bboxHexEstimate(const BBox* bbox, int res) {
    // Get the area of the pentagon as the maximally-distorted area possible
    H3Index pentagons[12] = {0};
    H3_EXPORT(getPentagonIndexes)(res, pentagons);
    double pentagonRadiusKm = _hexRadiusKm(pentagons[0]);
    // Area of a regular hexagon is 3/2*sqrt(3) * r * r
    // The pentagon has the most distortion (smallest edges) and shares its
    // edges with hexagons, so the most-distorted hexagons have this area,
    // shrunk by 20% off chance that the bounding box perfectly bounds a
    // pentagon.
    double pentagonAreaKm2 =
        0.8 * (2.59807621135 * pentagonRadiusKm * pentagonRadiusKm);

    // Then get the area of the bounding box of the geofence in question
    GeoCoord p1, p2;
    p1.lat = bbox->north;
    p1.lon = bbox->east;
    p2.lat = bbox->south;
    p2.lon = bbox->west;
    double d = H3_EXPORT(pointDistKm)(&p1, &p2);
    // Derived constant based on: https://math.stackexchange.com/a/1921940
    // Clamped to 3 as higher values tend to rapidly drag the estimate to zero.
    double a = d * d / fmin(3.0, fabs((p1.lon - p2.lon) / (p1.lat - p2.lat)));

    // Divide the two to get an estimate of the number of hexagons needed
    int estimate = (int)ceil(a / pentagonAreaKm2);
    if (estimate == 0) estimate = 1;
    return estimate;
}

/**
 * lineHexEstimate returns an estimated number of hexagons that trace
 *                 the cartesian-projected line
 *
 *  @param origin the origin coordinates
 *  @param destination the destination coordinates
 *  @param res the resolution of the H3 hexagons to trace the line
 *  @return the estimated number of hexagons required to trace the line
 */
int lineHexEstimate(const GeoCoord* origin, const GeoCoord* destination,
                    int res) {
    // Get the area of the pentagon as the maximally-distorted area possible
    H3Index pentagons[12] = {0};
    H3_EXPORT(getPentagonIndexes)(res, pentagons);
    double pentagonRadiusKm = _hexRadiusKm(pentagons[0]);

    double dist = H3_EXPORT(pointDistKm)(origin, destination);
    int estimate = (int)ceil(dist / (2 * pentagonRadiusKm));
    if (estimate == 0) estimate = 1;
    return estimate;
}
