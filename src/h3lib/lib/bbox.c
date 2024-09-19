/*
 * Copyright 2016-2021 Uber Technologies, Inc.
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
#include "h3Index.h"
#include "latLng.h"

/**
 * Width of the bounding box, in rads
 * @param  bbox Bounding box to inspect
 * @return      width, in rads
 */
double bboxWidthRads(const BBox *bbox) {
    return bboxIsTransmeridian(bbox) ? bbox->east - bbox->west + M_2PI
                                     : bbox->east - bbox->west;
}

/**
 * Height of the bounding box
 * @param  bbox Bounding box to inspect
 * @return      height, in rads
 */
double bboxHeightRads(const BBox *bbox) { return bbox->north - bbox->south; }

/**
 * Whether the given bounding box crosses the antimeridian
 * @param  bbox Bounding box to inspect
 * @return      is transmeridian
 */
bool bboxIsTransmeridian(const BBox *bbox) { return bbox->east < bbox->west; }

/**
 * Get the center of a bounding box
 * @param bbox   Input bounding box
 * @param center Output center coordinate
 */
void bboxCenter(const BBox *bbox, LatLng *center) {
    center->lat = (bbox->north + bbox->south) * 0.5;
    // If the bbox crosses the antimeridian, shift east 360 degrees
    double east = bboxIsTransmeridian(bbox) ? bbox->east + M_2PI : bbox->east;
    center->lng = constrainLng((east + bbox->west) * 0.5);
}

/**
 * Whether the bounding box contains a given point
 * @param  bbox  Bounding box
 * @param  point Point to test
 * @return       Whether the point is contained
 */
bool bboxContains(const BBox *bbox, const LatLng *point) {
    return point->lat >= bbox->south && point->lat <= bbox->north &&
           (bboxIsTransmeridian(bbox) ?
                                      // transmeridian case
                (point->lng >= bbox->west || point->lng <= bbox->east)
                                      :
                                      // standard case
                (point->lng >= bbox->west && point->lng <= bbox->east));
}

/**
 * Whether two bounding boxes overlap
 * @param  a First bounding box
 * @param  b Second bounding box
 * @return   Whether the bounding boxes overlap
 */
bool bboxOverlapsBBox(const BBox *a, const BBox *b) {
    // Check whether latitude coords overlap
    if (a->north < b->south || a->south > b->north) {
        return false;
    }

    // Check whether longitude coords overlap, accounting for transmeridian
    // bboxes
    LongitudeNormalization aNormalization;
    LongitudeNormalization bNormalization;
    bboxNormalization(a, b, &aNormalization, &bNormalization);

    if (normalizeLng(a->east, aNormalization) <
            normalizeLng(b->west, bNormalization) ||
        normalizeLng(a->west, aNormalization) >
            normalizeLng(b->east, bNormalization)) {
        return false;
    }

    return true;
}

/**
 * Whether one bounding box contains another
 * @param  a First bounding box
 * @param  b Second bounding box
 * @return   Whether a contains b
 */
bool bboxContainsBBox(const BBox *a, const BBox *b) {
    // Check whether latitude coords are contained
    if (a->north < b->north || a->south > b->south) {
        return false;
    }
    // Check whether longitude coords are contained
    // Account for transmeridian bboxes
    LongitudeNormalization aNormalization;
    LongitudeNormalization bNormalization;
    bboxNormalization(a, b, &aNormalization, &bNormalization);
    return normalizeLng(a->west, aNormalization) <=
               normalizeLng(b->west, bNormalization) &&
           normalizeLng(a->east, aNormalization) >=
               normalizeLng(b->east, bNormalization);
}

/**
 * Whether two bounding boxes are strictly equal
 * @param  b1 Bounding box 1
 * @param  b2 Bounding box 2
 * @return    Whether the boxes are equal
 */
bool bboxEquals(const BBox *b1, const BBox *b2) {
    return b1->north == b2->north && b1->south == b2->south &&
           b1->east == b2->east && b1->west == b2->west;
}

CellBoundary bboxToCellBoundary(const BBox *bbox) {
    // Convert bbox to cell boundary, CCW vertex order
    CellBoundary bboxBoundary = {.numVerts = 4,
                                 .verts = {{bbox->north, bbox->east},
                                           {bbox->north, bbox->west},
                                           {bbox->south, bbox->west},
                                           {bbox->south, bbox->east}}};
    return bboxBoundary;
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
    LatLng h3Center;
    CellBoundary h3Boundary;
    H3_EXPORT(cellToLatLng)(h3Index, &h3Center);
    H3_EXPORT(cellToBoundary)(h3Index, &h3Boundary);
    return H3_EXPORT(greatCircleDistanceKm)(&h3Center, h3Boundary.verts);
}

/**
 * bboxHexEstimate returns an estimated number of hexagons that fit
 *                 within the cartesian-projected bounding box
 *
 * @param bbox the bounding box to estimate the hexagon fill level
 * @param res the resolution of the H3 hexagons to fill the bounding box
 * @param out the estimated number of hexagons to fill the bounding box
 * @return E_SUCCESS (0) on success, or another value otherwise.
 */
H3Error bboxHexEstimate(const BBox *bbox, int res, int64_t *out) {
    // Get the area of the pentagon as the maximally-distorted area possible
    H3Index pentagons[12] = {0};
    H3Error pentagonsErr = H3_EXPORT(getPentagons)(res, pentagons);
    if (pentagonsErr) {
        return pentagonsErr;
    }
    double pentagonRadiusKm = _hexRadiusKm(pentagons[0]);
    // Area of a regular hexagon is 3/2*sqrt(3) * r * r
    // The pentagon has the most distortion (smallest edges) and shares its
    // edges with hexagons, so the most-distorted hexagons have this area,
    // shrunk by 20% off chance that the bounding box perfectly bounds a
    // pentagon.
    double pentagonAreaKm2 =
        0.8 * (2.59807621135 * pentagonRadiusKm * pentagonRadiusKm);

    // Then get the area of the bounding box of the geoloop in question
    LatLng p1, p2;
    p1.lat = bbox->north;
    p1.lng = bbox->east;
    p2.lat = bbox->south;
    p2.lng = bbox->west;
    double d = H3_EXPORT(greatCircleDistanceKm)(&p1, &p2);
    double lngDiff = fabs(p1.lng - p2.lng);
    double latDiff = fabs(p1.lat - p2.lat);
    if (lngDiff == 0 || latDiff == 0) {
        return E_FAILED;
    }
    double length = fmax(lngDiff, latDiff);
    double width = fmin(lngDiff, latDiff);
    double ratio = length / width;
    // Derived constant based on: https://math.stackexchange.com/a/1921940
    // Clamped to 3 as higher values tend to rapidly drag the estimate to zero.
    double a = d * d / fmin(3.0, ratio);

    // Divide the two to get an estimate of the number of hexagons needed
    double estimateDouble = ceil(a / pentagonAreaKm2);
    if (!isfinite(estimateDouble)) {
        return E_FAILED;
    }
    int64_t estimate = (int64_t)estimateDouble;
    if (estimate == 0) estimate = 1;
    *out = estimate;
    return E_SUCCESS;
}

/**
 * lineHexEstimate returns an estimated number of hexagons that trace
 *                 the cartesian-projected line
 *
 * @param origin the origin coordinates
 * @param destination the destination coordinates
 * @param res the resolution of the H3 hexagons to trace the line
 * @param out Out parameter for the estimated number of hexagons required to
 * trace the line
 * @return E_SUCCESS (0) on success or another value otherwise.
 */
H3Error lineHexEstimate(const LatLng *origin, const LatLng *destination,
                        int res, int64_t *out) {
    // Get the area of the pentagon as the maximally-distorted area possible
    H3Index pentagons[12] = {0};
    H3Error pentagonsErr = H3_EXPORT(getPentagons)(res, pentagons);
    if (pentagonsErr) {
        return pentagonsErr;
    }
    double pentagonRadiusKm = _hexRadiusKm(pentagons[0]);

    double dist = H3_EXPORT(greatCircleDistanceKm)(origin, destination);
    double distCeil = ceil(dist / (2 * pentagonRadiusKm));
    if (!isfinite(distCeil)) {
        return E_FAILED;
    }
    int64_t estimate = (int64_t)distCeil;
    if (estimate == 0) estimate = 1;
    *out = estimate;
    return E_SUCCESS;
}

/**
 * Scale a given bounding box by some factor. Scales both width and height
 * by the factor, rather than scaling area, which will scale at scale^2.
 * Note that this function is meant to handle bounding boxes and scales,
 * within a reasonable domain, and does not guarantee reasonable results for
 * extreme values.
 * @param bbox Bounding box to scale, in-place
 * @param scale Scale factor
 */
void scaleBBox(BBox *bbox, double scale) {
    double width = bboxWidthRads(bbox);
    double height = bboxHeightRads(bbox);
    double widthBuffer = (width * scale - width) * 0.5;
    double heightBuffer = (height * scale - height) * 0.5;
    // Scale north and south, clamping to latitude domain
    bbox->north += heightBuffer;
    if (bbox->north > M_PI_2) bbox->north = M_PI_2;
    bbox->south -= heightBuffer;
    if (bbox->south < -M_PI_2) bbox->south = -M_PI_2;
    // Scale east and west, clamping to longitude domain
    bbox->east += widthBuffer;
    if (bbox->east > M_PI) bbox->east -= M_2PI;
    if (bbox->east < -M_PI) bbox->east += M_2PI;
    bbox->west -= widthBuffer;
    if (bbox->west > M_PI) bbox->west -= M_2PI;
    if (bbox->west < -M_PI) bbox->west += M_2PI;
}

/**
 * Determine the longitude normalization scheme for two bounding boxes, either
 * or both of which might cross the antimeridian. The goal is to transform
 * latitudes in one or both boxes so that they are in the same frame of
 * reference and can be operated on with standard Cartesian functions.
 * @param a              First bounding box
 * @param b              Second bounding box
 * @param aNormalization Output: Normalization for longitudes in the first box
 * @param bNormalization Output: Normalization for longitudes in the second box
 */
void bboxNormalization(const BBox *a, const BBox *b,
                       LongitudeNormalization *aNormalization,
                       LongitudeNormalization *bNormalization) {
    bool aIsTransmeridian = bboxIsTransmeridian(a);
    bool bIsTransmeridian = bboxIsTransmeridian(b);
    bool aToBTrendsEast = a->west - b->east < b->west - a->east;
    // If neither is transmeridian, no normalization.
    // If both are transmeridian, normalize east by convention.
    // If one is transmeridian and one is not, normalize toward the other.
    *aNormalization = !aIsTransmeridian  ? NORMALIZE_NONE
                      : bIsTransmeridian ? NORMALIZE_EAST
                      : aToBTrendsEast   ? NORMALIZE_EAST
                                         : NORMALIZE_WEST;
    *bNormalization = !bIsTransmeridian  ? NORMALIZE_NONE
                      : aIsTransmeridian ? NORMALIZE_EAST
                      : aToBTrendsEast   ? NORMALIZE_WEST
                                         : NORMALIZE_EAST;
}
