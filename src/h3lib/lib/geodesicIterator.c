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

/** @file geodesicIterator.c
 * @brief Manages geodesic polygon iterator lifecycle and traversal helpers.
 */

#include "geodesicIterator.h"

#include "geodesicPolygonInternal.h"
#include "h3Index.h"

#define H3_CHECK(h3_call, iter_ptr)                    \
    do {                                               \
        H3Error _err = (h3_call);                      \
        if (_err != E_SUCCESS) {                       \
            iterErrorPolygonCompact((iter_ptr), _err); \
            return;                                    \
        }                                              \
    } while (0)

static GeodesicPolygon *_getOrCreateGeodesicPolygon(
    IterCellsPolygonCompact *iter) {
    if (iter->geodesicPoly == NULL) {
        if (iter->_polygon->geoloop.numVerts <= 0) {
            iterErrorPolygonCompact(iter, E_DOMAIN);
            return NULL;
        }
        if (iter->_polygon->numHoles > 0 && iter->_polygon->holes == NULL) {
            iterErrorPolygonCompact(iter, E_DOMAIN);
            return NULL;
        }
        for (int i = 0; i < iter->_polygon->numHoles; i++) {
            if (iter->_polygon->holes[i].numVerts <= 0) {
                iterErrorPolygonCompact(iter, E_DOMAIN);
                return NULL;
            }
        }
        GeodesicPolygon *poly = geodesicPolygonCreate(iter->_polygon);
        if (!poly) {
            iterErrorPolygonCompact(iter, E_MEMORY_ALLOC);
            return NULL;
        }
        iter->geodesicPoly = poly;
    }
    return iter->geodesicPoly;
}

void geodesicIteratorDestroyState(IterCellsPolygonCompact *iter) {
    if (!iter || !iter->geodesicPoly) {
        return;
    }
    geodesicPolygonDestroy(iter->geodesicPoly);
    iter->geodesicPoly = NULL;
}

void geodesicIteratorStep(IterCellsPolygonCompact *iter, H3Index cell) {
    GeodesicPolygon *poly = _getOrCreateGeodesicPolygon(iter);
    if (!poly) {
        return;
    }

    ContainmentMode mode = FLAG_GET_CONTAINMENT_MODE(iter->_flags);

    while (cell) {
        int cellRes = H3_GET_RESOLUTION(cell);

        SphereCap cap;
        H3_CHECK(cellToSphereCap(cell, &cap), iter);
        if (cellRes < iter->_res) {
            if (!geodesicPolygonCapIntersects(poly, &cap)) {
                cell = nextCell(cell);
                continue;
            }

            H3Index child;
            H3_CHECK(H3_EXPORT(cellToCenterChild)(cell, cellRes + 1, &child),
                     iter);
            cell = child;
            continue;
        }

        GeodesicCellBoundary boundary;
        H3_CHECK(cellToGeodesicBoundary(cell, &boundary), iter);

        const bool boundaryIntersection =
            geodesicPolygonBoundaryIntersects(poly, &boundary, &cap);

        if (!boundaryIntersection) {
            const bool pointInside =
                geodesicPolygonContainsPoint(poly, &boundary.verts[0]);
            if (pointInside) {
                iter->cell = cell;
                return;
            }
        }

        bool intersects = boundaryIntersection;

        if (!intersects) {
            H3Index polygonCell;
            Vec3d *firstVert = &poly->geoloop.edges[0].vert;
            H3_CHECK(vec3dToCell(firstVert, cellRes, &polygonCell), iter);
            if (polygonCell == cell) {
                intersects = true;
            }
        }

        if (intersects && mode != CONTAINMENT_FULL) {
            iter->cell = cell;
            return;
        }

        cell = nextCell(cell);
    }

    iterDestroyPolygonCompact(iter);
}
