#ifndef AREA_H
#define AREA_H

#include "h3api.h"

/** @defgroup geoLoopAreaRads2 geoLoopAreaRads2
 * Functions for geoLoopAreaRads2
 * @{
 */
/**
 * @brief Area in radians^2 enclosed by vertices of a GeoLoop.
 */
H3Error geoLoopAreaRads2(GeoLoop loop, double *out);
/** @} */

#endif
