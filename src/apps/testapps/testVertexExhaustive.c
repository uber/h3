/*
 * Copyright 2020-2021 Uber Technologies, Inc.
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
/** @file
 * @brief tests H3 vertex functions, exhaustively checking all cells at res 0-4
 */

#include "test.h"
#include "utility.h"
#include "vertex.h"

static void directionForVertexNum_symmetry_assertions(H3Index h3) {
    int numVerts = H3_EXPORT(isPentagon)(h3) ? NUM_PENT_VERTS : NUM_HEX_VERTS;
    for (int i = 0; i < numVerts; i++) {
        Direction dir = directionForVertexNum(h3, i);
        int vertexNum = vertexNumForDirection(h3, dir);
        t_assert(
            vertexNum == i,
            "directionForVertexNum and vertexNumForDirection are symmetrical");
    }
}

static void cellToVertex_point_assertions(H3Index h3) {
    CellBoundary gb;
    H3_EXPORT(cellToBoundary)(h3, &gb);
    int numVerts = H3_EXPORT(isPentagon)(h3) ? NUM_PENT_VERTS : NUM_HEX_VERTS;

    // This test won't work if there are distortion vertexes in the boundary
    if (numVerts < gb.numVerts) return;

    LatLng coord;
    for (int i = 0; i < numVerts; i++) {
        H3Index vertex;
        t_assertSuccess(H3_EXPORT(cellToVertex)(h3, i, &vertex));
        t_assertSuccess(H3_EXPORT(vertexToLatLng)(vertex, &coord));
        int almostEqual =
            geoAlmostEqualThreshold(&gb.verts[i], &coord, 0.000001);
        t_assert(almostEqual, "Vertex coordinates match boundary vertex");
    }
}

static void cellToVertex_uniqueness_assertions(H3Index h3) {
    H3Index originVerts[NUM_HEX_VERTS] = {0};
    t_assertSuccess(H3_EXPORT(cellToVertexes)(h3, originVerts));

    for (int v1 = 0; v1 < NUM_HEX_VERTS - 1; v1++) {
        for (int v2 = v1 + 1; v2 < NUM_HEX_VERTS; v2++) {
            if (originVerts[v1] == originVerts[v2]) {
                t_assert(false, "vertex should be unique");
            }
        }
    }
}

static void cellToVertex_validity_assertions(H3Index h3) {
    H3Index verts[NUM_HEX_VERTS] = {0};
    t_assertSuccess(H3_EXPORT(cellToVertexes)(h3, verts));

    for (int i = 0; i < NUM_HEX_VERTS - 1; i++) {
        if (verts[i] != H3_NULL) {
            t_assert(H3_EXPORT(isValidVertex(verts[i])), "vertex is valid");
        }
    }
}

static void cellToVertex_neighbor_assertions(H3Index h3) {
    H3Index neighbors[7] = {0};
    H3Index originVerts[NUM_HEX_VERTS] = {0};
    H3Index neighborVerts[NUM_HEX_VERTS] = {0};

    t_assertSuccess(H3_EXPORT(gridDisk)(h3, 1, neighbors));
    t_assertSuccess(H3_EXPORT(cellToVertexes)(h3, originVerts));

    for (int i = 0; i < 7; i++) {
        H3Index neighbor = neighbors[i];
        if (neighbor == H3_NULL || neighbor == h3) continue;
        t_assertSuccess(H3_EXPORT(cellToVertexes)(neighbor, neighborVerts));

        // calculate the set intersection
        int intersection = 0;
        for (int v1 = 0; v1 < NUM_HEX_VERTS; v1++) {
            for (int v2 = 0; v2 < NUM_HEX_VERTS; v2++) {
                if (neighborVerts[v1] == originVerts[v2]) {
                    intersection++;
                }
            }
        }

        t_assert(intersection == 2,
                 "Neighbor shares 2 unique vertexes with origin");
    }
}

SUITE(Vertex) {
    TEST(directionForVertexNum_symmetry) {
        iterateAllIndexesAtRes(0, directionForVertexNum_symmetry_assertions);
        iterateAllIndexesAtRes(1, directionForVertexNum_symmetry_assertions);
        iterateAllIndexesAtRes(2, directionForVertexNum_symmetry_assertions);
        iterateAllIndexesAtRes(3, directionForVertexNum_symmetry_assertions);
        iterateAllIndexesAtRes(4, directionForVertexNum_symmetry_assertions);
    }

    TEST(cellToVertex_point) {
        iterateAllIndexesAtRes(0, cellToVertex_point_assertions);
        iterateAllIndexesAtRes(1, cellToVertex_point_assertions);
        iterateAllIndexesAtRes(2, cellToVertex_point_assertions);
        iterateAllIndexesAtRes(3, cellToVertex_point_assertions);
        iterateAllIndexesAtRes(4, cellToVertex_point_assertions);

        // Res 5: normal base cell
        iterateBaseCellIndexesAtRes(5, cellToVertex_point_assertions, 0);
        // Res 5: pentagon base cell
        iterateBaseCellIndexesAtRes(5, cellToVertex_point_assertions, 14);
        // Res 5: polar pentagon base cell
        iterateBaseCellIndexesAtRes(5, cellToVertex_point_assertions, 117);
    }

    TEST(cellToVertex_neighbors) {
        iterateAllIndexesAtRes(0, cellToVertex_neighbor_assertions);
        iterateAllIndexesAtRes(1, cellToVertex_neighbor_assertions);
        iterateAllIndexesAtRes(2, cellToVertex_neighbor_assertions);
        iterateAllIndexesAtRes(3, cellToVertex_neighbor_assertions);
        iterateAllIndexesAtRes(4, cellToVertex_neighbor_assertions);
    }

    TEST(cellToVertex_uniqueness) {
        iterateAllIndexesAtRes(0, cellToVertex_uniqueness_assertions);
        iterateAllIndexesAtRes(1, cellToVertex_uniqueness_assertions);
        iterateAllIndexesAtRes(2, cellToVertex_uniqueness_assertions);
        iterateAllIndexesAtRes(3, cellToVertex_uniqueness_assertions);
        iterateAllIndexesAtRes(4, cellToVertex_uniqueness_assertions);
    }

    TEST(cellToVertex_validity) {
        iterateAllIndexesAtRes(0, cellToVertex_validity_assertions);
        iterateAllIndexesAtRes(1, cellToVertex_validity_assertions);
        iterateAllIndexesAtRes(2, cellToVertex_validity_assertions);
        iterateAllIndexesAtRes(3, cellToVertex_validity_assertions);
        iterateAllIndexesAtRes(4, cellToVertex_validity_assertions);
    }
}
