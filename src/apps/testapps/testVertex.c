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

    TEST(directionForVertexNum_hex) {
        H3Index origin = 0x823d6ffffffffff;
        bool seenDirs[NUM_DIGITS] = {false};
        for (int vertexNum = 0; vertexNum < NUM_HEX_VERTS; vertexNum++) {
            Direction dir = directionForVertexNum(origin, vertexNum);
            t_assert(dir > 0 && dir < INVALID_DIGIT, "direction appears valid");
            t_assert(!seenDirs[dir], "direction appears only once");
            seenDirs[dir] = true;
        }
    }

    TEST(directionForVertexNum_badVerts) {
        H3Index origin = 0x823d6ffffffffff;

        t_assert(directionForVertexNum(origin, -1) == INVALID_DIGIT,
                 "negative vertex should return invalid direction");
        t_assert(directionForVertexNum(origin, 6) == INVALID_DIGIT,
                 "invalid vertex should return invalid direction");

        H3Index pentagon = 0x823007fffffffff;
        t_assert(directionForVertexNum(pentagon, 5) == INVALID_DIGIT,
                 "invalid pent vertex should return invalid direction");
    }

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
}
