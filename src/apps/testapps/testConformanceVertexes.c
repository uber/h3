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
 * @brief Conformance runner for the `vertexes` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceVertexes <record-file>`
 *
 *  Each line holds one valid cell with its vertexes as a set and
 *  `cellToVertex` for vertex numbers 0 to 5, which fails with `E_DOMAIN`
 *  for number 5 of a pentagon.
 */

#include "conformance.h"
#include "h3api.h"

#define NUM_VERTEX_NUMBERS 6

static void checkVertexes(const cJSON *record) {
    H3Index h = jsonIndex(record, "index");

    H3Index vertexes[NUM_VERTEX_NUMBERS] = {0};
    c_assert(!H3_EXPORT(cellToVertexes)(h, vertexes), "cellToVertexes");
    for (int i = 0; i < NUM_VERTEX_NUMBERS; i++) {
        c_assert(vertexes[i] == 0 || H3_EXPORT(isValidVertex)(vertexes[i]),
                 "vertexes: not a valid vertex");
    }
    assertIndexSet(jsonArray(record, "vertexes"), vertexes, NUM_VERTEX_NUMBERS,
                   "vertexes");

    const cJSON *byNumber = jsonArray(record, "byNumber");
    c_assert(cJSON_GetArraySize(byNumber) == NUM_VERTEX_NUMBERS,
             "byNumber: expected six entries");
    int number = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, byNumber) {
        H3Index vertex;
        H3Error err = H3_EXPORT(cellToVertex)(h, number, &vertex);
        if (!expectResult(item, err, "byNumber")) {
            assertIndex(item, vertex, "byNumber");
        }
        number++;
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkVertexes);
}
