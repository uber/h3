/*
 * Copyright 2016-2023 Uber Technologies, Inc.
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
#include "h3Assert.h"
#include "h3api.h"
#include "mathExtensions.h"

/**
 * Normalizes radians to a value between 0.0 and two PI.
 *
 * @param rads The input radians value.
 * @return The normalized radians value.
 */
double _posAngleRads(double rads) {
    double tmp = ((rads < 0.0) ? rads + M_2PI : rads);
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
 * Normalize an input longitude according to the specified normalization
 * @param  lng           Input longitude
 * @param  normalization Longitude normalization strategy
 * @return               Normalized longitude
 */
double normalizeLng(const double lng,
                    const LongitudeNormalization normalization) {
    switch (normalization) {
        case NORMALIZE_EAST:
            return lng < 0 ? lng + (double)M_2PI : lng;
        case NORMALIZE_WEST:
            return lng > 0 ? lng - (double)M_2PI : lng;
        default:
            return lng;
    }
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
double H3_EXPORT(greatCircleDistanceRads)(const LatLng *a, const LatLng *b) {
    double sinLat = sin((b->lat - a->lat) * 0.5);
    double sinLng = sin((b->lng - a->lng) * 0.5);

    double A = sinLat * sinLat + cos(a->lat) * cos(b->lat) * sinLng * sinLng;

    return 2 * atan2(sqrt(A), sqrt(1 - A));
}

/**
 * The great circle distance in kilometers between two spherical coordinates.
 *
 * @param  a  the first lat/lng pair (in radians)
 * @param  b  the second lat/lng pair (in radians)
 *
 * @return    the great circle distance in kilometers between a and b
 */
double H3_EXPORT(greatCircleDistanceKm)(const LatLng *a, const LatLng *b) {
    return H3_EXPORT(greatCircleDistanceRads)(a, b) * EARTH_RADIUS_KM;
}

/**
 * The great circle distance in meters between two spherical coordinates.
 *
 * @param  a  the first lat/lng pair (in radians)
 * @param  b  the second lat/lng pair (in radians)
 *
 * @return    the great circle distance in meters between a and b
 */
double H3_EXPORT(greatCircleDistanceM)(const LatLng *a, const LatLng *b) {
    return H3_EXPORT(greatCircleDistanceKm)(a, b) * 1000;
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
            double invcosp2lat = 1.0 / cos(p2->lat);
            sinlng = sin(az) * sin(distance) * invcosp2lat;
            coslng = (cos(distance) - sin(p1->lat) * sin(p2->lat)) /
                     cos(p1->lat) * invcosp2lat;
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

H3Error H3_EXPORT(getHexagonAreaAvgKm2)(int res, double *out) {
    static const double areas[] = {
        4.357449416078383e+06, 6.097884417941332e+05, 8.680178039899720e+04,
        1.239343465508816e+04, 1.770347654491307e+03, 2.529038581819449e+02,
        3.612906216441245e+01, 5.161293359717191e+00, 7.373275975944177e-01,
        1.053325134272067e-01, 1.504750190766435e-02, 2.149643129451879e-03,
        3.070918756316060e-04, 4.387026794728296e-05, 6.267181135324313e-06,
        8.953115907605790e-07};
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    *out = areas[res];
    return E_SUCCESS;
}

H3Error H3_EXPORT(getHexagonAreaAvgM2)(int res, double *out) {
    static const double areas[] = {
        4.357449416078390e+12, 6.097884417941339e+11, 8.680178039899731e+10,
        1.239343465508818e+10, 1.770347654491309e+09, 2.529038581819452e+08,
        3.612906216441250e+07, 5.161293359717198e+06, 7.373275975944188e+05,
        1.053325134272069e+05, 1.504750190766437e+04, 2.149643129451882e+03,
        3.070918756316063e+02, 4.387026794728301e+01, 6.267181135324322e+00,
        8.953115907605802e-01};
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    *out = areas[res];
    return E_SUCCESS;
}

H3Error H3_EXPORT(getHexagonEdgeLengthAvgKm)(int res, double *out) {
    static const double lens[] = {
        1281.256011, 483.0568391, 182.5129565, 68.97922179,
        26.07175968, 9.854090990, 3.724532667, 1.406475763,
        0.531414010, 0.200786148, 0.075863783, 0.028663897,
        0.010830188, 0.004092010, 0.001546100, 0.000584169};
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    *out = lens[res];
    return E_SUCCESS;
}

H3Error H3_EXPORT(getHexagonEdgeLengthAvgM)(int res, double *out) {
    static const double lens[] = {
        1281256.011, 483056.8391, 182512.9565, 68979.22179,
        26071.75968, 9854.090990, 3724.532667, 1406.475763,
        531.4140101, 200.7861476, 75.86378287, 28.66389748,
        10.83018784, 4.092010473, 1.546099657, 0.584168630};
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    *out = lens[res];
    return E_SUCCESS;
}

H3Error H3_EXPORT(getNumCells)(int res, int64_t *out) {
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    *out = 2 + 120 * _ipow(7, res);
    return E_SUCCESS;
}

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
    double s = (a + b + c) * 0.5;

    a = (s - a) * 0.5;
    b = (s - b) * 0.5;
    c = (s - c) * 0.5;
    s = s * 0.5;

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
    return triangleEdgeLengthsToArea(H3_EXPORT(greatCircleDistanceRads)(a, b),
                                     H3_EXPORT(greatCircleDistanceRads)(b, c),
                                     H3_EXPORT(greatCircleDistanceRads)(c, a));
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
 * @param    out  cell area in radians^2
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(cellAreaRads2)(H3Index cell, double *out) {
    LatLng c;
    CellBoundary cb;
    H3Error err = H3_EXPORT(cellToLatLng)(cell, &c);
    if (err) {
        return err;
    }
    err = H3_EXPORT(cellToBoundary)(cell, &cb);
    if (NEVER(err)) {
        // Uncoverable because cellToLatLng will have returned an error already
        return err;
    }

    double area = 0.0;
    for (int i = 0; i < cb.numVerts; i++) {
        int j = (i + 1) % cb.numVerts;
        area += triangleArea(&cb.verts[i], &cb.verts[j], &c);
    }

    *out = area;
    return E_SUCCESS;
}

/**
 * Area of H3 cell in kilometers^2.
 *
 * @param   cell  H3 cell
 * @param    out  cell area in kilometers^2
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(cellAreaKm2)(H3Index cell, double *out) {
    H3Error err = H3_EXPORT(cellAreaRads2)(cell, out);
    if (!err) {
        *out = *out * EARTH_RADIUS_KM * EARTH_RADIUS_KM;
    }
    return err;
}

/**
 * Area of H3 cell in meters^2.
 *
 * @param   cell  H3 cell
 * @param    out  cell area in meters^2
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(cellAreaM2)(H3Index cell, double *out) {
    H3Error err = H3_EXPORT(cellAreaKm2)(cell, out);
    if (!err) {
        *out = *out * 1000 * 1000;
    }
    return err;
}

/**
 * Length of a directed edge in radians.
 *
 * @param   edge  H3 directed edge
 * @param    length  length in radians
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(edgeLengthRads)(H3Index edge, double *length) {
    CellBoundary cb;

    H3Error err = H3_EXPORT(directedEdgeToBoundary)(edge, &cb);
    if (err) {
        return err;
    }

    *length = 0.0;
    for (int i = 0; i < cb.numVerts - 1; i++) {
        *length +=
            H3_EXPORT(greatCircleDistanceRads)(&cb.verts[i], &cb.verts[i + 1]);
    }

    return E_SUCCESS;
}

/**
 * Length of a directed edge in kilometers.
 *
 * @param   edge  H3 directed edge
 * @param    length  length in kilometers
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(edgeLengthKm)(H3Index edge, double *length) {
    H3Error err = H3_EXPORT(edgeLengthRads)(edge, length);
    *length = *length * EARTH_RADIUS_KM;
    return err;
}

/**
 * Length of a directed edge in meters.
 *
 * @param   edge  H3 directed edge
 * @param    length  length in meters
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(edgeLengthM)(H3Index edge, double *length) {
    H3Error err = H3_EXPORT(edgeLengthKm)(edge, length);
    *length = *length * 1000;
    return err;
}
