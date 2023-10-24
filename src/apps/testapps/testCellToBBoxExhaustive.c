/*
 * Copyright 2023 Uber Technologies, Inc.
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
 * @brief tests H3 function `cellToBBox`
 *
 *  usage: `testCellToBBoxExhaustive`
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bbox.h"
#include "h3Index.h"
#include "h3api.h"
#include "polyfill.h"
#include "polygon.h"
#include "test.h"
#include "utility.h"

static void cellBBox_assertions(H3Index h3) {
    BBox bbox;
    t_assertSuccess(cellToBBox(h3, &bbox, false));

    CellBoundary verts;
    t_assertSuccess(H3_EXPORT(cellToBoundary)(h3, &verts));
    for (int j = 0; j < verts.numVerts; j++) {
        if (!bboxContains(&bbox, &verts.verts[j])) {
            printf("cell: ");
            h3Println(h3);
            bboxPrintln(&bbox);
            geoPrintln(&verts.verts[j]);
        }
        t_assert(bboxContains(&bbox, &verts.verts[j]),
                 "BBox contains cell vertex");
    }
}

static void childBBox_assertions(H3Index h3) {
    int parentRes = H3_GET_RESOLUTION(h3);

    BBox bbox;
    t_assertSuccess(cellToBBox(h3, &bbox, true));

    for (int resolutionOffset = 0; resolutionOffset < 5; resolutionOffset++) {
        // Test whether all verts of all children are inside the bbox
        int childRes = parentRes + resolutionOffset;
        int64_t numChildren;
        t_assertSuccess(
            H3_EXPORT(cellToChildrenSize)(h3, childRes, &numChildren));

        H3Index *children = calloc(numChildren, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(cellToChildren)(h3, childRes, children));

        for (int64_t i = 0; i < numChildren; i++) {
            CellBoundary childVerts;
            t_assertSuccess(
                H3_EXPORT(cellToBoundary)(children[i], &childVerts));
            for (int j = 0; j < childVerts.numVerts; j++) {
                if (!bboxContains(&bbox, &childVerts.verts[j])) {
                    printf("parent: ");
                    h3Println(h3);
                    bboxPrintln(&bbox);
                    h3Println(children[i]);
                    geoPrintln(&childVerts.verts[j]);
                }
                t_assert(bboxContains(&bbox, &childVerts.verts[j]),
                         "BBox contains child vertex");
            }
        }

        free(children);
    }
}

SUITE(cellToBBox) {
    TEST(cellBBox_correctness) {
        iterateAllIndexesAtRes(0, cellBBox_assertions);
        iterateAllIndexesAtRes(1, cellBBox_assertions);
        iterateAllIndexesAtRes(2, cellBBox_assertions);
    }
    TEST(childBBox_correctness) {
        iterateAllIndexesAtRes(0, childBBox_assertions);
        iterateAllIndexesAtRes(1, childBBox_assertions);
        iterateAllIndexesAtRes(2, childBBox_assertions);
    }
}
