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
 * @brief tests H3 vertex functions.
 */

#include "test.h"
#include "utility.h"
#include "vertex.h"

// Fixtures
static LatLng sfGeo = {0.659966917655, -2.1364398519396};

SUITE(Vertex) {
    TEST(cellToVertex_badVerts) {
        H3Index origin = 0x823d6ffffffffff;

        H3Index vert;
        t_assert(H3_EXPORT(cellToVertex)(origin, -1, &vert) == E_DOMAIN,
                 "negative vertex should return null index");
        t_assert(H3_EXPORT(cellToVertex)(origin, 6, &vert) == E_DOMAIN,
                 "invalid vertex should return null index");

        H3Index pentagon = 0x823007fffffffff;
        t_assert(H3_EXPORT(cellToVertex)(pentagon, 5, &vert) == E_DOMAIN,
                 "invalid pent vertex should return null index");
    }

    TEST(cellToVertex_invalid) {
        H3Index invalid = 0xFFFFFFFFFFFFFFFF;
        H3Index vert;
        t_assert(H3_EXPORT(cellToVertex)(invalid, 3, &vert) == E_FAILED,
                 "Invalid cell returns error");
    }

    TEST(cellToVertex_invalid2) {
        H3Index index = 0x685b2396e900fff9;
        H3Index vert;
        t_assert(H3_EXPORT(cellToVertex)(index, 2, &vert) == E_CELL_INVALID,
                 "Invalid cell returns error");
    }

    TEST(cellToVertex_invalid3) {
        H3Index index = 0x20ff20202020ff35;
        H3Index vert;
        t_assert(H3_EXPORT(cellToVertex)(index, 0, &vert) == E_CELL_INVALID,
                 "Invalid cell returns error");
    }

    TEST(isValidVertex_hex) {
        H3Index origin = 0x823d6ffffffffff;
        H3Index vert = 0x2222597fffffffff;

        t_assert(H3_EXPORT(isValidVertex)(vert), "known vertex is valid");

        for (int i = 0; i < NUM_HEX_VERTS; i++) {
            t_assertSuccess(H3_EXPORT(cellToVertex)(origin, i, &vert));
            t_assert(H3_EXPORT(isValidVertex)(vert), "vertex is valid");
        }
    }

    TEST(isValidVertex_invalidOwner) {
        H3Index origin = 0x823d6ffffffffff;
        int vertexNum = 0;
        H3Index vert;
        t_assertSuccess(H3_EXPORT(cellToVertex)(origin, vertexNum, &vert));

        // Set a bit for an unused digit to something else.
        vert ^= 1;

        t_assert(H3_EXPORT(isValidVertex)(vert) == 0,
                 "vertex with invalid owner is not valid");
    }

    TEST(isValidVertex_wrongOwner) {
        H3Index origin = 0x823d6ffffffffff;
        int vertexNum = 0;
        H3Index vert;
        t_assertSuccess(H3_EXPORT(cellToVertex)(origin, vertexNum, &vert));

        // Assert that origin does not own the vertex
        H3Index owner = vert;
        H3_SET_MODE(owner, H3_CELL_MODE);
        H3_SET_RESERVED_BITS(owner, 0);

        t_assert(origin != owner, "origin does not own the canonical vertex");

        H3Index nonCanonicalVertex = origin;
        H3_SET_MODE(nonCanonicalVertex, H3_VERTEX_MODE);
        H3_SET_RESERVED_BITS(nonCanonicalVertex, vertexNum);

        t_assert(H3_EXPORT(isValidVertex)(nonCanonicalVertex) == 0,
                 "vertex with incorrect owner is not valid");
    }

    TEST(isValidVertex_badVerts) {
        H3Index origin = 0x823d6ffffffffff;
        t_assert(H3_EXPORT(isValidVertex)(origin) == 0, "cell is not valid");

        H3Index fakeEdge = origin;
        H3_SET_MODE(fakeEdge, H3_DIRECTEDEDGE_MODE);
        t_assert(H3_EXPORT(isValidVertex)(fakeEdge) == 0,
                 "edge mode is not valid");

        H3Index vert;
        t_assertSuccess(H3_EXPORT(cellToVertex)(origin, 0, &vert));
        H3_SET_RESERVED_BITS(vert, 6);
        t_assert(H3_EXPORT(isValidVertex)(vert) == 0,
                 "invalid vertexNum is not valid");

        H3Index pentagon = 0x823007fffffffff;
        H3Index vert2;
        t_assertSuccess(H3_EXPORT(cellToVertex)(pentagon, 0, &vert2));
        H3_SET_RESERVED_BITS(vert2, 5);
        t_assert(H3_EXPORT(isValidVertex)(vert2) == 0,
                 "invalid pentagon vertexNum is not valid");
    }

    TEST(vertexToLatLng_invalid) {
        H3Index invalid = 0xFFFFFFFFFFFFFFFF;
        LatLng latLng;
        t_assert(H3_EXPORT(vertexToLatLng)(invalid, &latLng) != E_SUCCESS,
                 "Invalid vertex returns error");
    }

    TEST(cellToVertexes_invalid) {
        H3Index invalid = 0xFFFFFFFFFFFFFFFF;
        H3Index verts[6] = {0};
        t_assert(H3_EXPORT(cellToVertexes)(invalid, verts) == E_FAILED,
                 "cellToVertexes fails for invalid cell");
    }

    TEST(edgeToVertexes) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index ring[7] = {0};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 1, ring));

        H3Index edge;
        t_assertSuccess(H3_EXPORT(cellsToDirectedEdge)(sf, ring[0], &edge));

        H3Index v1, v2;
        t_assertSuccess(H3_EXPORT(edgeToVertexes)(edge, &v1, &v2));

        t_assert(H3_EXPORT(isValidVertex)(v1), "start vertex is valid");
        t_assert(H3_EXPORT(isValidVertex)(v2), "end vertex is valid");
        t_assert(v1 != v2, "start and end vertexes differ");

        // Both vertexes should belong to the origin cell
        H3Index cellVerts[6];
        t_assertSuccess(H3_EXPORT(cellToVertexes)(sf, cellVerts));
        int foundV1 = 0, foundV2 = 0;
        for (int i = 0; i < 6; i++) {
            if (cellVerts[i] == v1) foundV1 = 1;
            if (cellVerts[i] == v2) foundV2 = 1;
        }
        t_assert(foundV1, "start vertex belongs to the origin cell");
        t_assert(foundV2, "end vertex belongs to the origin cell");

        // The vertexes should match the ends of the edge boundary
        CellBoundary boundary;
        t_assertSuccess(H3_EXPORT(directedEdgeToBoundary)(edge, &boundary));
        LatLng v1Coord, v2Coord;
        t_assertSuccess(H3_EXPORT(vertexToLatLng)(v1, &v1Coord));
        t_assertSuccess(H3_EXPORT(vertexToLatLng)(v2, &v2Coord));
        t_assert(geoAlmostEqual(&v1Coord, &boundary.verts[0]),
                 "start vertex matches first boundary point");
        t_assert(
            geoAlmostEqual(&v2Coord, &boundary.verts[boundary.numVerts - 1]),
            "end vertex matches last boundary point");
    }

    TEST(edgeToVertexes_pentagon) {
        H3Index pentagon = 0x823007fffffffff;
        H3Index edges[6] = {0};
        t_assertSuccess(H3_EXPORT(originToDirectedEdges)(pentagon, edges));

        for (int i = 0; i < 6; i++) {
            if (edges[i] == H3_NULL) continue;

            H3Index v1, v2;
            t_assertSuccess(H3_EXPORT(edgeToVertexes)(edges[i], &v1, &v2));

            t_assert(H3_EXPORT(isValidVertex)(v1),
                     "pentagon start vertex is valid");
            t_assert(H3_EXPORT(isValidVertex)(v2),
                     "pentagon end vertex is valid");
            t_assert(v1 != v2, "pentagon start and end vertexes differ");
        }
    }

    TEST(edgeToVertexes_invalid) {
        H3Index v1, v2;

        // H3_NULL is not a valid edge
        t_assert(
            H3_EXPORT(edgeToVertexes)(H3_NULL, &v1, &v2) == E_DIR_EDGE_INVALID,
            "edgeToVertexes fails on H3_NULL");

        // A cell is not a valid edge
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        t_assert(H3_EXPORT(edgeToVertexes)(sf, &v1, &v2) == E_DIR_EDGE_INVALID,
                 "edgeToVertexes fails on a cell");

        // An edge with an invalid direction is not valid
        H3Index invalidEdge = sf;
        H3_SET_MODE(invalidEdge, H3_DIRECTEDEDGE_MODE);
        H3_SET_RESERVED_BITS(invalidEdge, 0);
        t_assert(H3_EXPORT(edgeToVertexes)(invalidEdge, &v1, &v2) ==
                     E_DIR_EDGE_INVALID,
                 "edgeToVertexes fails on an edge with an invalid direction");

        // The K axis direction is invalid for a pentagon's edges
        H3Index pentagon = 0x823007fffffffff;
        H3Index pentagonKEdge = pentagon;
        H3_SET_MODE(pentagonKEdge, H3_DIRECTEDEDGE_MODE);
        H3_SET_RESERVED_BITS(pentagonKEdge, K_AXES_DIGIT);
        t_assert(H3_EXPORT(edgeToVertexes)(pentagonKEdge, &v1, &v2) ==
                     E_DIR_EDGE_INVALID,
                 "edgeToVertexes fails on a pentagon's K edge");
    }
}
