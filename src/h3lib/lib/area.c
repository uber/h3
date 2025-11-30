/*
 * Copyright 2025 Uber Technologies, Inc.
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
/** @file area.c
 * @brief   Algorithms for computing areas of regions on a sphere (GeoLoop,
 * cells, polygons, multipolygons, etc.)
 */

#include <math.h>

#include "adder.h"
#include "constants.h"
#include "h3api.h"

// Cagnoli contribution for edge arc x to y, following d3-geo’s
// area implementation:
// https://github.com/d3/d3-geo/blob/8c53a90ae70c94bace73ecb02f2c792c649c86ba/src/area.js#L51-L70
static inline double cagnoli(LatLng x, LatLng y) {
    x.lat = x.lat / 2.0 + M_PI / 4.0;
    y.lat = y.lat / 2.0 + M_PI / 4.0;

    double sa = sin(x.lat) * sin(y.lat);
    double ca = cos(x.lat) * cos(y.lat);

    double d = y.lng - x.lng;
    double sd = sin(d);
    double cd = cos(d);

    return -2.0 * atan2(sa * sd, sa * cd + ca);
}

/**
 * Area in radians^2 enclosed by vertices in GeoLoop.
 *
 * The GeoLoop should represent a simple curve with no self-intersections.
 * Vertices should be ordered according to the "right hand rule".
 * That is, if you are looking from outer space at a spherical polygon loop
 * on the surface of the earth whose interior is contained within a hemisphere,
 * then the vertices should be ordered counter-clockwise. The interior of the
 * loop is to the left of a person walking along the boundary of the polygon
 * in the counter-clockwise direction.
 *
 * Note that GeoLoops do not need to repeat the first vertex at the end of the
 * array to close the loop; this is done automatically.
 *
 * The edge arcs between adjacent vertices are assumed to be the shortest
 * geodesic path between them; that is, all arcs are interpreted to be less
 * than 180 degrees or pi radians.
 * Avoid arcs that are exactly pi (i.e, two antipodal vertices).
 * "Large" polygon loops (e.g., that cannot be contained in a hemisphere) can
 * still be constructed by using intermediate vertices with arcs less than
 * 180 degrees, and the loop area will still be computed correctly.
 *
 * The area of the entire globe is 4*pi radians^2. If, for example, you have a
 * small GeoLoop with area `a << 4*pi` and then reverse the order of the
 * vertices, you produce GeoLoop with area `4*pi - a`, since, by the right hand
 * rule, the new loop's interior is the majority of the globe,
 * or "everything except the original polygon".
 * Note that the area enclosed by the loop is determined by the vertex order;
 * this function does **not** return `min(a, 4*pi - a)`.
 *
 * @param   loop  GeoLoop of boundary vertices in counter-clockwise order
 * @param    out  loop area in radians^2, in interval [0, 4*pi]
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(geoLoopAreaRads2)(GeoLoop loop, double *out) {
    // Use `Adder` to improve numerical accuracy of the sum of many Cagnoli
    // terms
    Adder adder = {0.0, 0.0};

    for (int i = 0; i < loop.numVerts; i++) {
        int j = (i + 1) % loop.numVerts;
        kadd(&adder, cagnoli(loop.verts[i], loop.verts[j]));
    }

    // The Cagnoli sum above yields a signed area, with the sign switching
    // with the orientation of the vertices. Since we want our area to always be
    // positive, we normalize into [0, 4*pi] by adding 4*pi when the signed
    // area is negative.
    if (adder.sum < 0.0) {
        kadd(&adder, 4.0 * M_PI);
    }

    *out = adder.sum;
    return E_SUCCESS;
}

/**
 * Area of H3 cell in radians^2.
 *
 * Uses `geoLoopAreaRads2` to compute cell area.
 *
 * @param   cell  H3 cell
 * @param    out  cell area in radians^2
 * @return        E_SUCCESS on success, or an error code otherwise
 */
H3Error H3_EXPORT(cellAreaRads2)(H3Index cell, double *out) {
    CellBoundary cb;
    H3Error err = H3_EXPORT(cellToBoundary)(cell, &cb);
    if (err) {
        return err;
    }

    GeoLoop loop = {.verts = cb.verts, .numVerts = cb.numVerts};
    err = H3_EXPORT(geoLoopAreaRads2)(loop, out);
    if (err) {
        return err;
    }

    return E_SUCCESS;
}
