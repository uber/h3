/*
 * Copyright 2018-2021 Uber Technologies, Inc.
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
/** @file polygon.c
 * @brief Polygon (GeoLoop) algorithms
 */

#include "polygon.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>

#include "bbox.h"
#include "constants.h"
#include "h3api.h"
#include "latLng.h"
#include "linkedGeo.h"

// Define macros used in polygon algos for GeoLoop
#define TYPE GeoLoop
#define INIT_ITERATION INIT_ITERATION_GEOFENCE
#define ITERATE ITERATE_GEOFENCE
#define IS_EMPTY IS_EMPTY_GEOFENCE

#include "polygonAlgos.h"

#undef TYPE
#undef INIT_ITERATION
#undef ITERATE
#undef IS_EMPTY

/**
 * Create a bounding box from a GeoPolygon
 * @param polygon Input GeoPolygon
 * @param bboxes  Output bboxes, one for the outer loop and one for each hole
 */
void bboxesFromGeoPolygon(const GeoPolygon *polygon, BBox *bboxes) {
    bboxFromGeoLoop(&polygon->geoloop, &bboxes[0]);
    for (int i = 0; i < polygon->numHoles; i++) {
        bboxFromGeoLoop(&polygon->holes[i], &bboxes[i + 1]);
    }
}

/**
 * pointInsidePolygon takes a given GeoPolygon data structure and
 * checks if it contains a given geo coordinate.
 *
 * @param geoPolygon The geoloop and holes defining the relevant area
 * @param bboxes     The bboxes for the main geoloop and each of its holes
 * @param coord      The coordinate to check
 * @return           Whether the point is contained
 */
bool pointInsidePolygon(const GeoPolygon *geoPolygon, const BBox *bboxes,
                        const LatLng *coord) {
    // Start with contains state of primary geoloop
    bool contains =
        pointInsideGeoLoop(&(geoPolygon->geoloop), &bboxes[0], coord);

    // If the point is contained in the primary geoloop, but there are holes in
    // the geoloop iterate through all holes and return false if the point is
    // contained in any hole
    if (contains && geoPolygon->numHoles > 0) {
        for (int i = 0; i < geoPolygon->numHoles; i++) {
            if (pointInsideGeoLoop(&(geoPolygon->holes[i]), &bboxes[i + 1],
                                   coord)) {
                return false;
            }
        }
    }

    return contains;
}

/**
 * Whether a cell boundary is completely contained by a polygon.
 * @param  geoPolygon The polygon to test
 * @param  bboxes     The bboxes for the main geoloop and each of its holes
 * @param  boundary   The cell boundary to test
 * @return            Whether the cell boundary is contained
 */
bool cellBoundaryInsidePolygon(const GeoPolygon *geoPolygon, const BBox *bboxes,
                               const CellBoundary *boundary,
                               const BBox *boundaryBBox) {
    // First test a single point. Note that this fails fast if point is outside
    // bounding box.
    if (!pointInsidePolygon(geoPolygon, &bboxes[0], &boundary->verts[0])) {
        return false;
    }

    // If a point is contained, check for any line intersections
    if (cellBoundaryCrossesGeoLoop(&(geoPolygon->geoloop), &bboxes[0], boundary,
                                   boundaryBBox)) {
        return false;
    }
    // Check for line intersections with any hole
    for (int i = 0; i < geoPolygon->numHoles; i++) {
        if (cellBoundaryCrossesGeoLoop(&(geoPolygon->holes[i]), &bboxes[i + 1],
                                       boundary, boundaryBBox)) {
            return false;
        }
    }
    return true;
}

/**
 * Whether a cell boundary crosses a geo loop. Crossing in this case means
 * whether any line segments intersect; it does not include containment.
 * @param  geoloop  Geo loop to test
 * @param  boundary Cell boundary to test
 * @return          Whether any line segments in the boundary intersect any line
 * segments in the geo loop
 */
bool cellBoundaryCrossesGeoLoop(const GeoLoop *geoloop, const BBox *loopBBox,
                                const CellBoundary *boundary,
                                const BBox *boundaryBBox) {
    if (!bboxIntersects(loopBBox, boundaryBBox)) {
        return false;
    }
    bool isTransmeridianLoop = bboxIsTransmeridian(loopBBox);
    bool isTransmeridianBoundary = bboxIsTransmeridian(boundaryBBox);

    LatLng loop1;
    LatLng loop2;
    LatLng boundary1;
    LatLng boundary2;

    for (int i = 0; i < geoloop->numVerts; i++) {
        loop1 = geoloop->verts[i];
        loop1.lng = NORMALIZE_LNG(loop1.lng, isTransmeridianLoop);
        loop2 = geoloop->verts[(i + 1) % geoloop->numVerts];
        loop2.lng = NORMALIZE_LNG(loop2.lng, isTransmeridianLoop);
        for (int j = 0; j < boundary->numVerts; j++) {
            boundary1 = boundary->verts[j];
            boundary1.lng =
                NORMALIZE_LNG(boundary1.lng, isTransmeridianBoundary);
            boundary2 = boundary->verts[(j + 1) % boundary->numVerts];
            boundary2.lng =
                NORMALIZE_LNG(boundary2.lng, isTransmeridianBoundary);
            if (lineIntersectsLine(&loop1, &loop2, &boundary1, &boundary2)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Whether two lines intersect. This is a purely Cartesian implementation
 * and does not consider anti-meridian wrapping, poles, etc. Based on
 * http://www.jeffreythompson.org/collision-detection/line-line.php
 * @param  a1 Start of line A
 * @param  a2 End of line A
 * @param  b1 Start of line B
 * @param  b2 End of line B
 * @return    Whether the lines intersect
 */
bool lineIntersectsLine(const LatLng *a1, const LatLng *a2, const LatLng *b1,
                        const LatLng *b2) {
    double test;
    test = ((b2->lat - b1->lat) * (a1->lng - b1->lng) -
            (b2->lng - b1->lng) * (a1->lat - b1->lat)) /
           ((b2->lng - b1->lng) * (a2->lat - a1->lat) -
            (b2->lat - b1->lat) * (a2->lng - a1->lng));
    if (test < 0 || test > 1) return false;

    test = ((a2->lat - a1->lat) * (a1->lng - b1->lng) -
            (a2->lng - a1->lng) * (a1->lat - b1->lat)) /
           ((b2->lng - b1->lng) * (a2->lat - a1->lat) -
            (b2->lat - b1->lat) * (a2->lng - a1->lng));
    return (test >= 0 && test <= 1);
}
