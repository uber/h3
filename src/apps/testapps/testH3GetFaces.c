/*
 * Copyright 2019 Uber Technologies, Inc.
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
 * @brief tests the h3GetFaces function
 */

#include <stdio.h>
#include <stdlib.h>

#include "baseCells.h"
#include "h3Index.h"
#include "h3api.h"
#include "test.h"
#include "utility.h"

static int countFaces(H3Index h3, int expectedMax) {
    int sz = H3_EXPORT(maxFaceCount)(h3);
    t_assert(sz == expectedMax, "got expected max face count");
    int *faces = calloc(sz, sizeof(int));

    H3_EXPORT(h3GetFaces)(h3, faces);

    int validCount = 0;
    for (int i = 0; i < sz; i++) {
        if (faces[i] >= 0 && faces[i] <= 19) validCount++;
    }

    free(faces);
    return validCount;
}

static void assertSingleHexFace(H3Index h3) {
    int validCount = countFaces(h3, 2);
    t_assert(validCount == 1, "got a single valid face");
}

static void assertMultipleHexFaces(H3Index h3) {
    int validCount = countFaces(h3, 2);
    t_assert(validCount == 2, "got multiple valid faces for a hexagon");
}

static void assertPentagonFaces(H3Index h3) {
    t_assert(H3_EXPORT(h3IsPentagon)(h3), "got a pentagon");
    int validCount = countFaces(h3, 5);
    t_assert(validCount == 5, "got 5 valid faces for a pentagon");
}

SUITE(h3GetFaces) {
    TEST(singleFaceHexes) {
        // base cell 16 is at the center of an icosahedron face,
        // so all children should have the same face
        iterateBaseCellIndexesAtRes(2, assertSingleHexFace, 16);
        iterateBaseCellIndexesAtRes(3, assertSingleHexFace, 16);
    }

    TEST(hexagonWithEdgeVertices) {
        // Class II pentagon neighbor - one face, two adjacent vertices on edge
        H3Index h3 = 0x821c37fffffffff;
        assertSingleHexFace(h3);
    }

    TEST(hexagonWithDistortion) {
        // Class III pentagon neighbor, distortion across faces
        H3Index h3 = 0x831c06fffffffff;
        assertMultipleHexFaces(h3);
    }

    TEST(hexagonCrossingFaces) {
        // Class II hex with two vertices on edge
        H3Index h3 = 0x821ce7fffffffff;
        assertMultipleHexFaces(h3);
    }

    TEST(classIIIPentagon) {
        H3Index pentagon;
        setH3Index(&pentagon, 1, 4, 0);
        assertPentagonFaces(pentagon);
    }

    TEST(classIIPentagon) {
        H3Index pentagon;
        setH3Index(&pentagon, 2, 4, 0);
        assertPentagonFaces(pentagon);
    }

    TEST(res15Pentagon) {
        H3Index pentagon;
        setH3Index(&pentagon, 15, 4, 0);
        assertPentagonFaces(pentagon);
    }

    TEST(baseCellHexagons) {
        int singleCount = 0;
        int multipleCount = 0;
        for (int i = 0; i < NUM_BASE_CELLS; i++) {
            if (!_isBaseCellPentagon(i)) {
                // Make the base cell index
                H3Index baseCell;
                setH3Index(&baseCell, 0, i, 0);
                int validCount = countFaces(baseCell, 2);
                t_assert(validCount > 0, "got at least one face");
                if (validCount == 1)
                    singleCount++;
                else
                    multipleCount++;
            }
        }
        t_assert(singleCount == 4 * 20,
                 "got single face for 4 aligned hex base cells per face");
        t_assert(multipleCount == 1.5 * 20,
                 "got multiple faces for non-aligned hex base cells");
    }

    TEST(baseCellPentagons) {
        for (int i = 0; i < NUM_BASE_CELLS; i++) {
            if (_isBaseCellPentagon(i)) {
                // Make the base cell index
                H3Index baseCell;
                setH3Index(&baseCell, 0, i, 0);
                assertPentagonFaces(baseCell);
            }
        }
    }
}
