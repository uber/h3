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
 * @brief Conformance runner for the `inspection` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceInspection <record-file>`
 *
 *  Each line holds one 64-bit index, which may be a cell, a directed edge,
 *  a vertex or an arbitrary bit pattern, and the reference's answer from
 *  every inspection function. Everything here is discrete and compared
 *  exactly.
 */

#include <stdlib.h>
#include <string.h>

#include "conformance.h"
#include "constants.h"
#include "h3api.h"

static void checkInspection(const cJSON *record) {
    H3Index h = jsonIndex(record, "index");
    int res = jsonInt(record, "res");
    int baseCell = jsonInt(record, "baseCell");

    c_assert(H3_EXPORT(getResolution)(h) == res, "res");
    c_assert(H3_EXPORT(getBaseCellNumber)(h) == baseCell, "baseCell");
    c_assert(!!H3_EXPORT(isValidCell)(h) == jsonBool(record, "validCell"),
             "validCell");
    c_assert(!!H3_EXPORT(isValidIndex)(h) == jsonBool(record, "validIndex"),
             "validIndex");
    c_assert(!!H3_EXPORT(isResClassIII)(h) == jsonBool(record, "resClassIII"),
             "resClassIII");
    c_assert(!!H3_EXPORT(isPentagon)(h) == jsonBool(record, "pentagon"),
             "pentagon");

    // faces: getIcosahedronFaces as a set, with the -1 padding removed, or
    // its error.
    const cJSON *facesItem = jsonItem(record, "faces");
    int maxFaces;
    c_assert(!H3_EXPORT(maxFaceCount)(h, &maxFaces), "maxFaceCount");
    int *faces = calloc((size_t)maxFaces, sizeof *faces);
    c_assert(faces, "out of memory");
    H3Error faceErr = H3_EXPORT(getIcosahedronFaces)(h, faces);
    if (!expectResult(facesItem, faceErr, "faces")) {
        // Collect the expected set and compare as sets: both sides small.
        int gotCount = 0;
        for (int i = 0; i < maxFaces; i++) {
            if (faces[i] >= 0) {
                faces[gotCount++] = faces[i];
            }
        }
        int wantCount = cJSON_GetArraySize(facesItem);
        c_assert(gotCount == wantCount, "faces: count differs");
        const cJSON *item;
        cJSON_ArrayForEach(item, facesItem) {
            c_assert(cJSON_IsNumber(item), "faces: entry is not a number");
            bool found = false;
            for (int i = 0; i < gotCount && !found; i++) {
                found = faces[i] == item->valueint;
            }
            c_assert(found, "faces: expected face not returned");
        }
        // Duplicates in the output would be caught by the count check only
        // if the expected set is smaller; check them directly.
        for (int i = 0; i < gotCount; i++) {
            for (int j = i + 1; j < gotCount; j++) {
                c_assert(faces[i] != faces[j], "faces: duplicate face");
            }
        }
    }
    free(faces);

    // digits: getIndexDigit for every resolution 1 through 15.
    const cJSON *digitsItem = jsonArray(record, "digits");
    c_assert(cJSON_GetArraySize(digitsItem) == MAX_H3_RES,
             "digits: expected 15 entries");
    int digits[MAX_H3_RES];
    int r = 1;
    const cJSON *item;
    cJSON_ArrayForEach(item, digitsItem) {
        c_assert(cJSON_IsNumber(item), "digits: entry is not a number");
        int digit;
        c_assert(!H3_EXPORT(getIndexDigit)(h, r, &digit), "getIndexDigit");
        c_assert(digit == item->valueint, "digits");
        digits[r - 1] = digit;
        r++;
    }

    // construct: constructCell from the record's own fields.
    const cJSON *constructItem = jsonItem(record, "construct");
    H3Index constructed;
    H3Error constructErr =
        H3_EXPORT(constructCell)(res, baseCell, digits, &constructed);
    if (!expectResult(constructItem, constructErr, "construct")) {
        assertIndex(constructItem, constructed, "construct");
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkInspection);
}
