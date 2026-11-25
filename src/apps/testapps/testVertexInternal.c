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

#include "algos.h"
#include "test.h"
#include "utility.h"
#include "vertex.h"

SUITE(VertexInternal) {
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

    TEST(directionForNeighbor_invalid) {
        t_assert(directionForNeighbor(0, 0) == INVALID_DIGIT, "not neighbors");

        H3Index origin = ~0;

        t_assert(directionForNeighbor(origin, origin) == INVALID_DIGIT,
                 "not neighbors (all ones)");
    }

    TEST(vertexRotations_allPentagonBaseCells) {
        // vertexRotations (static, reached here via vertexNumForDirection
        // and directionForVertexNum) looks up each pentagon base cell in the
        // pentagonDirectionFaces table. Exercise every one of the
        // NUM_PENTAGONS base cells - including both polar pentagons (which
        // take the extra CCW rotation branch) and the rest (which can hit
        // the deleted-subsequence rotation branches) - to document that the
        // lookup always finds a match across the full input domain, for
        // every valid (non-K) direction.
        H3Index pentagons[NUM_PENTAGONS] = {0};
        t_assertSuccess(H3_EXPORT(getPentagons)(0, pentagons));

        for (int p = 0; p < NUM_PENTAGONS; p++) {
            H3Index pentagon = pentagons[p];
            t_assert(H3_EXPORT(isPentagon)(pentagon), "index is a pentagon");

            int vertexNums[NUM_PENT_VERTS] = {0};
            for (Direction dir = J_AXES_DIGIT; dir < NUM_DIGITS; dir++) {
                int vertexNum = vertexNumForDirection(pentagon, dir);
                t_assert(vertexNum >= 0 && vertexNum < NUM_PENT_VERTS,
                         "vertex number appears valid for every pentagon "
                         "base cell");

                Direction roundTrip =
                    directionForVertexNum(pentagon, vertexNum);
                t_assert(roundTrip == dir,
                         "direction round-trips through vertex number for "
                         "every pentagon base cell");

                vertexNums[vertexNum] = 1;
            }
            for (int v = 0; v < NUM_PENT_VERTS; v++) {
                t_assert(vertexNums[v],
                         "every pentagon vertex number is reachable");
            }
        }
    }
}
