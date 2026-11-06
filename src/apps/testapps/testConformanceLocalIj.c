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
 * @brief Conformance runner for the `localij` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceLocalIj <record-file>`
 *
 *  Each line holds an origin and a target cell, `cellToLocalIj` of the pair
 *  and `localIjToCell` of the result, or the error both report.
 */

#include <string.h>

#include "conformance.h"
#include "h3api.h"

static void checkLocalIj(const cJSON *record) {
    H3Index origin = jsonIndex(record, "index");
    H3Index target = jsonIndex(record, "target");
    const cJSON *ijItem = jsonItem(record, "ij");
    const cJSON *cellItem = jsonItem(record, "cell");

    CoordIJ ij;
    H3Error err = H3_EXPORT(cellToLocalIj)(origin, target, 0, &ij);
    if (expectResult(ijItem, err, "ij")) {
        // The cell key repeats the error.
        c_assert(isErrorObject(cellItem), "cell: expected the ij error");
        c_assert(
            !strcmp(jsonString(cellItem, "err"), jsonString(ijItem, "err")),
            "cell: error differs from the ij error");
        return;
    }
    c_assert(cJSON_IsObject(ijItem), "ij: not an object");
    c_assert(ij.i == jsonInt(ijItem, "i"), "ij.i");
    c_assert(ij.j == jsonInt(ijItem, "j"), "ij.j");

    CoordIJ fromRecord = {jsonInt(ijItem, "i"), jsonInt(ijItem, "j")};
    H3Index cell;
    err = H3_EXPORT(localIjToCell)(origin, &fromRecord, 0, &cell);
    if (!expectResult(cellItem, err, "cell")) {
        assertIndex(cellItem, cell, "cell");
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkLocalIj);
}
