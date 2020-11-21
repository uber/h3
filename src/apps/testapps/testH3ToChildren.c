/*
 * Copyright 2017-2019 Uber Technologies, Inc.
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

#include <stdlib.h>

#include "h3Index.h"
#include "test.h"

#define PADDED_COUNT 10

static void verifyCountAndUniqueness(H3Index* children, int paddedCount,
                                     int expectedCount) {
    int numFound = 0;
    for (int i = 0; i < paddedCount; i++) {
        H3Index currIndex = children[i];
        if (currIndex == 0) {
            continue;
        }
        numFound++;
        // verify uniqueness
        int indexSeen = 0;
        for (int j = i + 1; j < paddedCount; j++) {
            if (children[j] == currIndex) {
                indexSeen++;
            }
        }
        t_assert(indexSeen == 0, "index was seen only once");
    }
    t_assert(numFound == expectedCount, "got expected number of children");
}

static void subset(H3Index* set1, int len1, H3Index* set2, int len2) {
    int isthere;

    for (int i = 0; i < len1; i++) {
        if (set1[i] == 0) continue;

        isthere = 0;
        for (int j = 0; j < len2; j++) {
            if (set1[i] == set2[j]) isthere++;
        }
        t_assert(isthere == 1, "children must match");
    }
}

static void sets_equal(H3Index* set1, int len1, H3Index* set2, int len2) {
    subset(set1, len1, set2, len2);
    subset(set2, len2, set1, len1);
}

SUITE(h3ToChildren) {
    GeoCoord sf = {0.659966917655, 2 * 3.14159 - 2.1364398519396};
    H3Index sfHex8 = H3_EXPORT(geoToH3)(&sf, 8);

    TEST(oneResStep) {
        const int expectedCount = 7;
        const int paddedCount = 10;

        H3Index sfHex9s[PADDED_COUNT] = {0};
        H3_EXPORT(h3ToChildren)(sfHex8, 9, sfHex9s);

        GeoCoord center;
        H3_EXPORT(h3ToGeo)(sfHex8, &center);
        H3Index sfHex9_0 = H3_EXPORT(geoToH3)(&center, 9);

        int numFound = 0;

        // Find the center
        for (int i = 0; i < paddedCount; i++) {
            if (sfHex9_0 == sfHex9s[i]) {
                numFound++;
            }
        }
        t_assert(numFound == 1, "found the center hex");

        // Get the neighbor hexagons by averaging the center point and outer
        // points then query for those independently

        GeoBoundary outside;
        H3_EXPORT(h3ToGeoBoundary)(sfHex8, &outside);
        for (int i = 0; i < outside.numVerts; i++) {
            GeoCoord avg = {0};
            avg.lat = (outside.verts[i].lat + center.lat) / 2;
            avg.lon = (outside.verts[i].lon + center.lon) / 2;
            H3Index avgHex9 = H3_EXPORT(geoToH3)(&avg, 9);
            for (int j = 0; j < expectedCount; j++) {
                if (avgHex9 == sfHex9s[j]) {
                    numFound++;
                }
            }
        }

        t_assert(numFound == expectedCount, "found all expected children");
    }

    TEST(multipleResSteps) {
        // Lots of children. Will just confirm number and uniqueness
        const int expectedCount = 49;
        const int paddedCount = 60;

        H3Index* children = calloc(paddedCount, sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(sfHex8, 10, children);

        verifyCountAndUniqueness(children, paddedCount, expectedCount);
        free(children);
    }

    TEST(sameRes) {
        const int expectedCount = 1;
        const int paddedCount = 7;

        H3Index* children = calloc(paddedCount, sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(sfHex8, 8, children);

        verifyCountAndUniqueness(children, paddedCount, expectedCount);
        free(children);
    }

    TEST(childResTooCoarse) {
        const int expectedCount = 0;
        const int paddedCount = 7;

        H3Index* children = calloc(paddedCount, sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(sfHex8, 7, children);

        verifyCountAndUniqueness(children, paddedCount, expectedCount);
        free(children);
    }

    TEST(childResTooFine) {
        const int expectedCount = 0;
        const int paddedCount = 7;
        H3Index sfHexMax = H3_EXPORT(geoToH3)(&sf, MAX_H3_RES);

        H3Index* children = calloc(paddedCount, sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(sfHexMax, MAX_H3_RES + 1, children);

        verifyCountAndUniqueness(children, paddedCount, expectedCount);
        free(children);
    }

    TEST(pentagonChildren) {
        H3Index pentagon = 0x81083ffffffffff;  // res 1 pentagon
        const int childRes = 3;

        const int expectedCount = (5 * 7) + 6;
        const int paddedCount =
            H3_EXPORT(maxH3ToChildrenSize)(pentagon, childRes);

        H3Index expectedCells[] = {
            0x830800fffffffff, 0x830802fffffffff, 0x830803fffffffff,
            0x830804fffffffff, 0x830805fffffffff, 0x830806fffffffff,
            0x830810fffffffff, 0x830811fffffffff, 0x830812fffffffff,
            0x830813fffffffff, 0x830814fffffffff, 0x830815fffffffff,
            0x830816fffffffff, 0x830818fffffffff, 0x830819fffffffff,
            0x83081afffffffff, 0x83081bfffffffff, 0x83081cfffffffff,
            0x83081dfffffffff, 0x83081efffffffff, 0x830820fffffffff,
            0x830821fffffffff, 0x830822fffffffff, 0x830823fffffffff,
            0x830824fffffffff, 0x830825fffffffff, 0x830826fffffffff,
            0x830828fffffffff, 0x830829fffffffff, 0x83082afffffffff,
            0x83082bfffffffff, 0x83082cfffffffff, 0x83082dfffffffff,
            0x83082efffffffff, 0x830830fffffffff, 0x830831fffffffff,
            0x830832fffffffff, 0x830833fffffffff, 0x830834fffffffff,
            0x830835fffffffff, 0x830836fffffffff};

        H3Index* children = calloc(paddedCount, sizeof(H3Index));
        H3_EXPORT(h3ToChildren)(pentagon, childRes, children);

        sets_equal(children, paddedCount, expectedCells, expectedCount);

        // verifyCountAndUniqueness(children, paddedCount, expectedCount);

        // for (int i = 0; i < expectedCount - 1; i++) {
        //     t_assert(children[i] == expectedCells[i], "children must match");
        // }

        free(children);
    }
}
