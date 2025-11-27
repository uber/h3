#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adder.h"
#include "alloc.h"
#include "h3api.h"


void H3_EXPORT(destroyGeoLoop)(GeoLoop* loop) {
    H3_MEMORY(free)(loop->verts);
    loop->verts = NULL;
}

static double cagnoli(LatLng x, LatLng y) {
    x.lat = x.lat / 2.0 + M_PI / 4.0;
    y.lat = y.lat / 2.0 + M_PI / 4.0;

    double sa = sin(x.lat) * sin(y.lat);
    double ca = cos(x.lat) * cos(y.lat);

    double d = y.lng - x.lng;
    double sd = sin(d);
    double cd = cos(d);

    return -2.0 * atan2(sa * sd, sa * cd + ca);
}

static double vertArea(LatLng *verts, int numVerts) {
    Kahan k = {0.0, 0.0};

    for (int i = 0; i < numVerts; i++) {
        int j = (i + 1) % numVerts;
        kadd(&k, cagnoli(verts[i], verts[j]));
    }

    if (k.sum < 0.0) {
        kadd(&k, 4.0 * M_PI);
    }
    return k.sum;
}

H3Error H3_EXPORT(geoLoopArea)(GeoLoop loop, double *out) {
    *out = vertArea(loop.verts, loop.numVerts);
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
    *out = vertArea(cb.verts, cb.numVerts);

    return E_SUCCESS;
}
