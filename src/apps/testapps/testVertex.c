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

    TEST(isValidVertex_hex) {
        H3Index origin = 0x823d6ffffffffff;
        H3Index vert;

        for (int i = 0; i < NUM_HEX_VERTS; i++) {
            vert = H3_EXPORT(cellToVertex)(origin, i);
            t_assert(H3_EXPORT(isValidVertex)(vert), "vertex is valid");
        }
    }

    TEST(isValidVertex_badVerts) {
        H3Index origin = 0x823d6ffffffffff;
        t_assert(H3_EXPORT(isValidVertex)(origin) == 0, "cell is not valid");

        H3Index fakeEdge = origin;
        H3_SET_MODE(fakeEdge, H3_UNIEDGE_MODE);
        t_assert(H3_EXPORT(isValidVertex)(fakeEdge) == 0,
                 "edge mode is not valid");

        H3Index vert = H3_EXPORT(cellToVertex)(origin, 0);
        H3_SET_RESERVED_BITS(vert, 6);
        t_assert(H3_EXPORT(isValidVertex)(vert) == 0,
                 "invalid vertexNum is not valid");

        H3Index pentagon = 0x823007fffffffff;
        H3Index vert2 = H3_EXPORT(cellToVertex)(pentagon, 0);
        H3_SET_RESERVED_BITS(vert2, 5);
        t_assert(H3_EXPORT(isValidVertex)(vert2) == 0,
                 "invalid pentagon vertexNum is not valid");
    }
}
