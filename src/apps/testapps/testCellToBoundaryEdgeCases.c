/*
 * Copyright 2017-2021 Uber Technologies, Inc.
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

#include <float.h>
#include <stdlib.h>

#include "bbox.h"
#include "h3api.h"
#include "latLng.h"
#include "polygon.h"
#include "test.h"
#include "utility.h"

SUITE(cellToBoundaryEdgeCases) {
    TEST(doublePrecisionVertex) {
        // The carefully constructed case here:
        // - A res 1 pentagon cell with distortion vertexes that change
        //   when we use a double instead of a float in _v2dIntersect
        // - One of the previous (float-based) distortion vertexes
        // This is the only case yet found where a point indexed to the
        // cell is shown to be incorrectly outside of the geo boundary
        // when we use the float version. Presumably more could be found.
        H3Index cell = 0x81083ffffffffff;
        LatLng point = {.lat = H3_EXPORT(degsToRads)(61.890838431),
                        .lng = H3_EXPORT(degsToRads)(8.644221328)};

        CellBoundary boundary;
        t_assertSuccess(H3_EXPORT(cellToBoundary)(cell, &boundary));

        LatLng *verts = boundary.verts;
        GeoLoop geoloop = {.numVerts = boundary.numVerts, .verts = verts};

        BBox bbox;
        bboxFromGeoLoop(&geoloop, &bbox);

        H3Index cell2;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&point, 1, &cell2));
        // Check whether the point is physically inside the geo boundary
        if (cell2 == cell) {
            t_assert(pointInsideGeoLoop(&geoloop, &bbox, &point),
                     "Boundary contains input point");
        } else {
            t_assert(!pointInsideGeoLoop(&geoloop, &bbox, &point),
                     "Boundary does not contain input point");
        }
    }
}
