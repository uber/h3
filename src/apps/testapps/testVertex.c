/*
 * Copyright 2020 Uber Technologies, Inc.
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
 *
 *  usage: `testVertex`
 */

#include "test.h"
#include "utility.h"
#include "vertex.h"

static void directionForVertexNum_symmetry_assertions(H3Index h3) {
    int numVerts = H3_EXPORT(h3IsPentagon)(h3) ? NUM_PENT_VERTS : NUM_HEX_VERTS;
    for (int i = 0; i < numVerts; i++) {
        Direction dir = directionForVertexNum(h3, i);
        int vertexNum = vertexNumForDirection(h3, dir);
        t_assert(
            vertexNum == i,
            "directionForVertexNum and vertexNumForDirection are symmetrical");
    }
}

static void getCellVertex_point_assertions(H3Index h3) {
    GeoBoundary gb;
    H3_EXPORT(h3ToGeoBoundary)(h3, &gb);
    int numVerts = H3_EXPORT(h3IsPentagon)(h3) ? NUM_PENT_VERTS : NUM_HEX_VERTS;

    // This test won't work if there are distortion vertexes in the boundary
    if (numVerts < gb.numVerts) return;

    GeoCoord coord;
    for (int i = 0; i < numVerts; i++) {
        H3Index vertex = getCellVertex(h3, i);
        vertexToPoint(vertex, &coord);
        int almostEqual =
            geoAlmostEqualThreshold(&gb.verts[i], &coord, 0.000001);
        if (!almostEqual) {
            h3Print(h3);
            printf(" vertex %d of %d\n", i, numVerts);
            // geoPrintln(&coord);
            // geoBoundaryPrintln(&gb);
        }
        // t_assert(almostEqual, "Vertex coordinates match boundary vertex");
    }
}

SUITE(Vertex) {
    TEST(vertexNumForDirection_hex) {
        H3Index origin = 0x823d6ffffffffff;
        int vertexNums[NUM_HEX_VERTS] = {0};
        for (Direction dir = K_AXES_DIGIT; dir < NUM_DIGITS; dir++) {
            int vertexNum = vertexNumForDirection(origin, dir);
            t_assert(vertexNum >= 0 && vertexNum < NUM_HEX_VERTS,
                     "vertex number appears valid");
            t_assert(!vertexNums[vertexNum], "vertex number appears only once");
            vertexNums[vertexNum] = 1;
        }
    }

    TEST(vertexNumForDirection_pent) {
        H3Index pentagon = 0x823007fffffffff;
        int vertexNums[NUM_PENT_VERTS] = {0};
        for (Direction dir = J_AXES_DIGIT; dir < NUM_DIGITS; dir++) {
            int vertexNum = vertexNumForDirection(pentagon, dir);
            t_assert(vertexNum >= 0 && vertexNum < NUM_PENT_VERTS,
                     "vertex number appears valid");
            t_assert(!vertexNums[vertexNum], "vertex number appears only once");
            vertexNums[vertexNum] = 1;
        }
    }

    TEST(vertexNumForDirection_badDirections) {
        H3Index origin = 0x823007fffffffff;

        t_assert(
            vertexNumForDirection(origin, CENTER_DIGIT) == INVALID_VERTEX_NUM,
            "center digit should return invalid vertex");
        t_assert(
            vertexNumForDirection(origin, INVALID_DIGIT) == INVALID_VERTEX_NUM,
            "invalid digit should return invalid vertex");

        H3Index pentagon = 0x823007fffffffff;
        t_assert(
            vertexNumForDirection(pentagon, K_AXES_DIGIT) == INVALID_VERTEX_NUM,
            "K direction on pentagon should return invalid vertex");
    }

    // TODO: Move to exhaustive suite
    TEST(directionForVertexNum_symmetry) {
        iterateAllIndexesAtRes(0, directionForVertexNum_symmetry_assertions);
        iterateAllIndexesAtRes(1, directionForVertexNum_symmetry_assertions);
        iterateAllIndexesAtRes(2, directionForVertexNum_symmetry_assertions);
    }

    // TODO: Move to exhaustive suite
    TEST(getCellVertex_point) {
        // 0 fails on rotation into pentagon
        printf("res 0\n");
        iterateAllIndexesAtRes(0, getCellVertex_point_assertions);

        // 1 succeeds
        printf("res 1\n");
        iterateAllIndexesAtRes(1, getCellVertex_point_assertions);

        printf("res 2\n");
        iterateAllIndexesAtRes(2, getCellVertex_point_assertions);

        // 3 succeeds
        printf("res 3\n");
        iterateAllIndexesAtRes(3, getCellVertex_point_assertions);

        printf("res 4\n");
        iterateAllIndexesAtRes(4, getCellVertex_point_assertions);

        printf("res 5\n");
        // Res 5: normal base cell
        iterateBaseCellIndexesAtRes(5, getCellVertex_point_assertions, 0);
        // Res 5: pentagon base cell
        iterateBaseCellIndexesAtRes(5, getCellVertex_point_assertions, 14);
        // Res 5: polar pentagon base cell
        iterateBaseCellIndexesAtRes(5, getCellVertex_point_assertions, 117);
    }
}