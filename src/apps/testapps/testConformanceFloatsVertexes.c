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
 * @brief Conformance runner for `floats/vertexes.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceFloatsVertexes <record-file>`
 *
 *  Each line holds one vertex and `vertexToLatLng` of it, compared under
 *  the angular tolerance.
 */

#include "conformance.h"
#include "h3api.h"

static void checkFloatVertex(const cJSON *record) {
    H3Index vertex = jsonIndex(record, "index");
    LatLng point;
    c_assert(!H3_EXPORT(vertexToLatLng)(vertex, &point), "vertexToLatLng");
    assertPoint(jsonItem(record, "latLng"), &point, "latLng");
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkFloatVertex);
}
