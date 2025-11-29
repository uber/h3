#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adder.h"
#include "alloc.h"
#include "constants.h"
#include "h3api.h"

// TOOD: demonstrate the area alg works for global polygons.

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

H3Error H3_EXPORT(geoLoopArea)(GeoLoop loop, double *out) {
    Adder adder = {0.0, 0.0};

    for (int i = 0; i < loop.numVerts; i++) {
        int j = (i + 1) % loop.numVerts;
        kadd(&adder, cagnoli(loop.verts[i], loop.verts[j]));
    }

    if (adder.sum < 0.0) {
        kadd(&adder, 4.0 * M_PI);
    }

    *out = adder.sum;
    return E_SUCCESS;
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
    CellBoundary cb;
    H3Error err = H3_EXPORT(cellToBoundary)(cell, &cb);
    if (err) {
        return err;
    }

    GeoLoop loop = {.verts = cb.verts, .numVerts = cb.numVerts};
    err = H3_EXPORT(geoLoopArea)(loop, out);
    if (err) {
        return err;
    }

    return E_SUCCESS;
}

void H3_EXPORT(destroyGeoLoop)(GeoLoop *loop) {
    H3_MEMORY(free)(loop->verts);
    loop->verts = NULL;
}
