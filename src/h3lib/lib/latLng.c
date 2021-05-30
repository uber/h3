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
/** @file latLng.c
 * @brief   Functions for working with lat/lng coordinates.
 */

#include "latLng.h"

#include <math.h>
#include <stdbool.h>

#include "constants.h"
#include "h3api.h"
#include "mathExtensions.h"

/**
 * Normalizes radians to a value between 0.0 and two PI.
 *
 * @param rads The input radians value.
 * @return The normalized radians value.
 */
double _posAngleRads(double rads) {
    double tmp = ((rads < 0.0L) ? rads + M_2PI : rads);
    if (rads >= M_2PI) tmp -= M_2PI;
    return tmp;
}

/**
 * Determines if the components of two spherical coordinates are within some
 * threshold distance of each other.
 *
 * @param p1 The first spherical coordinates.
 * @param p2 The second spherical coordinates.
 * @param threshold The threshold distance.
 * @return Whether or not the two coordinates are within the threshold distance
 *         of each other.
 */
bool geoAlmostEqualThreshold(const LatLng *p1, const LatLng *p2,
                             double threshold) {
    return fabs(p1->lat - p2->lat) < threshold &&
           fabs(p1->lng - p2->lng) < threshold;
}

/**
 * Determines if the components of two spherical coordinates are within our
 * standard epsilon distance of each other.
 *
 * @param p1 The first spherical coordinates.
 * @param p2 The second spherical coordinates.
 * @return Whether or not the two coordinates are within the epsilon distance
 *         of each other.
 */
bool geoAlmostEqual(const LatLng *p1, const LatLng *p2) {
    return geoAlmostEqualThreshold(p1, p2, EPSILON_RAD);
}

/**
 * Set the components of spherical coordinates in decimal degrees.
 *
 * @param p The spherical coordinates.
 * @param latDegs The desired latitude in decimal degrees.
 * @param lngDegs The desired longitude in decimal degrees.
 */
void setGeoDegs(LatLng *p, double latDegs, double lngDegs) {
    _setGeoRads(p, H3_EXPORT(degsToRads)(latDegs),
                H3_EXPORT(degsToRads)(lngDegs));
}

/**
 * Set the components of spherical coordinates in radians.
 *
 * @param p The spherical coordinates.
 * @param latRads The desired latitude in decimal radians.
 * @param lngRads The desired longitude in decimal radians.
 */
void _setGeoRads(LatLng *p, double latRads, double lngRads) {
    p->lat = latRads;
    p->lng = lngRads;
}

/**
 * Convert from decimal degrees to radians.
 *
 * @param degrees The decimal degrees.
 * @return The corresponding radians.
 */
double H3_EXPORT(degsToRads)(double degrees) { return degrees * M_PI_180; }

/**
 * Convert from radians to decimal degrees.
 *
 * @param radians The radians.
 * @return The corresponding decimal degrees.
 */
double H3_EXPORT(radsToDegs)(double radians) { return radians * M_180_PI; }

/**
 * constrainLat makes sure latitudes are in the proper bounds
 *
 * @param lat The original lat value
 * @return The corrected lat value
 */
double constrainLat(double lat) {
    while (lat > M_PI_2) {
        lat = lat - M_PI;
    }
    return lat;
}

/**
 * constrainLng makes sure longitudes are in the proper bounds
 *
 * @param lng The origin lng value
 * @return The corrected lng value
 */
double constrainLng(double lng) {
    while (lng > M_PI) {
        lng = lng - (2 * M_PI);
    }
    while (lng < -M_PI) {
        lng = lng + (2 * M_PI);
    }
    return lng;
}

/**
 * The great circle distance in radians between two spherical coordinates.
 *
 * This function uses the Haversine formula.
 * For math details, see:
 *     https://en.wikipedia.org/wiki/Haversine_formula
 *     https://www.movable-type.co.uk/scripts/latlong.html
 *
 * @param  a  the first lat/lng pair (in radians)
 * @param  b  the second lat/lng pair (in radians)
 *
 * @return    the great circle distance in radians between a and b
 */
double H3_EXPORT(distanceRads)(const LatLng *a, const LatLng *b) {
    double sinLat = sin((b->lat - a->lat) / 2.0);
    double sinLng = sin((b->lng - a->lng) / 2.0);

    double A = sinLat * sinLat + cos(a->lat) * cos(b->lat) * sinLng * sinLng;

    return 2 * atan2(sqrt(A), sqrt(1 - A));
}

/**
 * The great circle distance in kilometers between two spherical coordinates.
 */
double H3_EXPORT(pointDistKm)(const LatLng *a, const LatLng *b) {
    return H3_EXPORT(distanceRads)(a, b) * EARTH_RADIUS_KM;
}

/**
 * The great circle distance in meters between two spherical coordinates.
 */
double H3_EXPORT(pointDistM)(const LatLng *a, const LatLng *b) {
    return H3_EXPORT(pointDistKm)(a, b) * 1000;
}

/**
 * Determines the azimuth to p2 from p1 in radians.
 *
 * @param p1 The first spherical coordinates.
 * @param p2 The second spherical coordinates.
 * @return The azimuth in radians from p1 to p2.
 */
double _geoAzimuthRads(const LatLng *p1, const LatLng *p2) {
    return atan2(cos(p2->lat) * sin(p2->lng - p1->lng),
                 cos(p1->lat) * sin(p2->lat) -
                     sin(p1->lat) * cos(p2->lat) * cos(p2->lng - p1->lng));
}

/**
 * Computes the point on the sphere a specified azimuth and distance from
 * another point.
 *
 * @param p1 The first spherical coordinates.
 * @param az The desired azimuth from p1.
 * @param distance The desired distance from p1, must be non-negative.
 * @param p2 The spherical coordinates at the desired azimuth and distance from
 * p1.
 */
void _geoAzDistanceRads(const LatLng *p1, double az, double distance,
                        LatLng *p2) {
    if (distance < EPSILON) {
        *p2 = *p1;
        return;
    }

    double sinlat, sinlng, coslng;

    az = _posAngleRads(az);

    // check for due north/south azimuth
    if (az < EPSILON || fabs(az - M_PI) < EPSILON) {
        if (az < EPSILON)  // due north
            p2->lat = p1->lat + distance;
        else  // due south
            p2->lat = p1->lat - distance;

        if (fabs(p2->lat - M_PI_2) < EPSILON)  // north pole
        {
            p2->lat = M_PI_2;
            p2->lng = 0.0;
        } else if (fabs(p2->lat + M_PI_2) < EPSILON)  // south pole
        {
            p2->lat = -M_PI_2;
            p2->lng = 0.0;
        } else
            p2->lng = constrainLng(p1->lng);
    } else  // not due north or south
    {
        sinlat = sin(p1->lat) * cos(distance) +
                 cos(p1->lat) * sin(distance) * cos(az);
        if (sinlat > 1.0) sinlat = 1.0;
        if (sinlat < -1.0) sinlat = -1.0;
        p2->lat = asin(sinlat);
        if (fabs(p2->lat - M_PI_2) < EPSILON)  // north pole
        {
            p2->lat = M_PI_2;
            p2->lng = 0.0;
        } else if (fabs(p2->lat + M_PI_2) < EPSILON)  // south pole
        {
            p2->lat = -M_PI_2;
            p2->lng = 0.0;
        } else {
            sinlng = sin(az) * sin(distance) / cos(p2->lat);
            coslng = (cos(distance) - sin(p1->lat) * sin(p2->lat)) /
                     cos(p1->lat) / cos(p2->lat);
            if (sinlng > 1.0) sinlng = 1.0;
            if (sinlng < -1.0) sinlng = -1.0;
            if (coslng > 1.0) coslng = 1.0;
            if (coslng < -1.0) coslng = -1.0;
            p2->lng = constrainLng(p1->lng + atan2(sinlng, coslng));
        }
    }
}

/*
 * The following functions provide meta information about the H3 hexagons at
 * each zoom level. Since there are only 16 total levels, these are current
 * handled with hardwired static values, but it may be worthwhile to put these
 * static values into another file that can be autogenerated by source code in
 * the future.
 */

double H3_EXPORT(getHexagonAreaAvgKm2)(int res) {
    static const double areas[] = {
        4250546.848, 607220.9782, 86745.85403, 12392.26486,
        1770.323552, 252.9033645, 36.1290521,  5.1612932,
        0.7373276,   0.1053325,   0.0150475,   0.0021496,
        0.0003071,   0.0000439,   0.0000063,   0.0000009};
    return areas[res];
}

double H3_EXPORT(getHexagonAreaAvgM2)(int res) {
    static const double areas[] = {
        4.25055E+12, 6.07221E+11, 86745854035, 12392264862,
        1770323552,  252903364.5, 36129052.1,  5161293.2,
        737327.6,    105332.5,    15047.5,     2149.6,
        307.1,       43.9,        6.3,         0.9};
    return areas[res];
}

double H3_EXPORT(getHexagonEdgeLengthAvgKm)(int res) {
    static const double lens[] = {
        1107.712591, 418.6760055, 158.2446558, 59.81085794,
        22.6063794,  8.544408276, 3.229482772, 1.220629759,
        0.461354684, 0.174375668, 0.065907807, 0.024910561,
        0.009415526, 0.003559893, 0.001348575, 0.000509713};
    return lens[res];
}

double H3_EXPORT(getHexagonEdgeLengthAvgM)(int res) {
    static const double lens[] = {
        1107712.591, 418676.0055, 158244.6558, 59810.85794,
        22606.3794,  8544.408276, 3229.482772, 1220.629759,
        461.3546837, 174.3756681, 65.90780749, 24.9105614,
        9.415526211, 3.559893033, 1.348574562, 0.509713273};
    return lens[res];
}

int64_t H3_EXPORT(getNumCells)(int res) { return 2 + 120 * _ipow(7, res); }

/**
 * Surface area in radians^2 of spherical triangle on unit sphere.
 *
 * For the math, see:
 * https://en.wikipedia.org/wiki/Spherical_trigonometry#Area_and_spherical_excess
 *
 * @param   a  length of triangle side A in radians
 * @param   b  length of triangle side B in radians
 * @param   c  length of triangle side C in radians
 *
 * @return     area in radians^2 of triangle on unit sphere
 */
double triangleEdgeLengthsToArea(double a, double b, double c) {
    double s = (a + b + c) / 2;

    a = (s - a) / 2;
    b = (s - b) / 2;
    c = (s - c) / 2;
    s = s / 2;

    return 4 * atan(sqrt(tan(s) * tan(a) * tan(b) * tan(c)));
}

/**
 * Compute area in radians^2 of a spherical triangle, given its vertices.
 *
 * @param   a  vertex lat/lng in radians
 * @param   b  vertex lat/lng in radians
 * @param   c  vertex lat/lng in radians
 *
 * @return     area of triangle on unit sphere, in radians^2
 */
double triangleArea(const LatLng *a, const LatLng *b, const LatLng *c) {
    return triangleEdgeLengthsToArea(H3_EXPORT(distanceRads)(a, b),
                                     H3_EXPORT(distanceRads)(b, c),
                                     H3_EXPORT(distanceRads)(c, a));
}

/**
 * Area of H3 cell in radians^2.
 *
 * The area is calculated by breaking the cell into spherical triangles and
 * summing up their areas. Note that some H3 cells (hexagons and pentagons)
 * are irregular, and have more than 6 or 5 sides.
 *
 * todo: optimize the computation by re-using the edges shared between triangles
 *
 * @param   cell  H3 cell
 *
 * @return        cell area in radians^2
 */
double H3_EXPORT(cellAreaRads2)(H3Index cell) {
    LatLng c;
    CellBoundary cb;
    H3_EXPORT(cellToLatLng)(cell, &c);
    H3_EXPORT(cellToBoundary)(cell, &cb);

    double area = 0.0;
    for (int i = 0; i < cb.numVerts; i++) {
        int j = (i + 1) % cb.numVerts;
        area += triangleArea(&cb.verts[i], &cb.verts[j], &c);
    }

    return area;
}

/**
 * Area of H3 cell in kilometers^2.
 */
double H3_EXPORT(cellAreaKm2)(H3Index h) {
    return H3_EXPORT(cellAreaRads2)(h) * EARTH_RADIUS_KM * EARTH_RADIUS_KM;
}

/**
 * Area of H3 cell in meters^2.
 */
double H3_EXPORT(cellAreaM2)(H3Index h) {
    return H3_EXPORT(cellAreaKm2)(h) * 1000 * 1000;
}

/**
 * Length of a directed edge in radians.
 *
 * @param   edge  H3 directed edge
 *
 * @return        length in radians
 */
double H3_EXPORT(exactEdgeLengthRads)(H3Index edge) {
    CellBoundary cb;

    H3_EXPORT(directedEdgeToBoundary)(edge, &cb);

    double length = 0.0;
    for (int i = 0; i < cb.numVerts - 1; i++) {
        length += H3_EXPORT(distanceRads)(&cb.verts[i], &cb.verts[i + 1]);
    }

    return length;
}

/**
 * Length of a directed edge in kilometers.
 */
double H3_EXPORT(exactEdgeLengthKm)(H3Index edge) {
    return H3_EXPORT(exactEdgeLengthRads)(edge) * EARTH_RADIUS_KM;
}

/**
 * Length of a directed edge in meters.
 */
double H3_EXPORT(exactEdgeLengthM)(H3Index edge) {
    return H3_EXPORT(exactEdgeLengthKm)(edge) * 1000;
}
