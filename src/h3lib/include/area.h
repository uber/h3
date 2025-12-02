#ifndef AREA_H
#define AREA_H

#include "h3api.h"

GeoMultiPolygon createGlobeMultiPolygon();
H3Error geoLoopAreaRads2(GeoLoop loop, double *out);
H3Error geoPolygonAreaRads2(GeoPolygon poly, double *out);
H3Error geoMultiPolygonAreaRads2(GeoMultiPolygon mpoly, double *out);

#endif
