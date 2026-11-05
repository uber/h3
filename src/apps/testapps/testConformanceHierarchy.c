/*
 * Copyright 2026 Uber Technologies, Inc.
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
 * @brief Conformance runner for the `hierarchy` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceHierarchy <record-file>`
 *
 *  Each line holds one valid cell with its parents at every coarser
 *  resolution, its center child and children one resolution finer, and its
 *  position among the children of its resolution 0 ancestor.
 */

#include <stdlib.h>

#include "conformance.h"
#include "h3api.h"

static void checkHierarchy(const cJSON *record) {
    H3Index h = jsonIndex(record, "index");
    int res = H3_EXPORT(getResolution)(h);

    const cJSON *parents = jsonArray(record, "parents");
    c_assert(cJSON_GetArraySize(parents) == res,
             "parents: expected one per coarser resolution");
    int parentRes = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, parents) {
        H3Index parent;
        c_assert(!H3_EXPORT(cellToParent)(h, parentRes, &parent),
                 "cellToParent");
        assertIndex(item, parent, "parents");
        parentRes++;
    }

    const cJSON *centerChildItem = jsonItem(record, "centerChild");
    H3Index centerChild;
    H3Error err = H3_EXPORT(cellToCenterChild)(h, res + 1, &centerChild);
    if (!expectResult(centerChildItem, err, "centerChild")) {
        assertIndex(centerChildItem, centerChild, "centerChild");
    }

    const cJSON *childrenItem = jsonItem(record, "children");
    int64_t childCount = 0;
    err = H3_EXPORT(cellToChildrenSize)(h, res + 1, &childCount);
    if (!expectResult(childrenItem, err, "children")) {
        H3Index *children = calloc((size_t)childCount, sizeof *children);
        c_assert(children, "out of memory");
        c_assert(!H3_EXPORT(cellToChildren)(h, res + 1, children),
                 "cellToChildren");
        assertIndexSet(childrenItem, children, childCount, "children");
        free(children);
    }

    const cJSON *childPosItem = jsonItem(record, "childPos");
    int64_t childPos;
    err = H3_EXPORT(cellToChildPos)(h, 0, &childPos);
    if (!expectResult(childPosItem, err, "childPos")) {
        c_assert(cJSON_IsNumber(childPosItem), "childPos: not a number");
        c_assert(childPos == (int64_t)childPosItem->valuedouble, "childPos");

        // The round trip through the resolution 0 ancestor.
        H3Index ancestor = h;
        if (res > 0) {
            ancestor = parseIndex(cJSON_GetArrayItem(parents, 0)->valuestring);
        }
        H3Index roundTrip;
        c_assert(
            !H3_EXPORT(childPosToCell)(childPos, ancestor, res, &roundTrip),
            "childPosToCell");
        c_assert(roundTrip == h, "childPosToCell does not round trip");
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkHierarchy);
}
