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
 * @brief Conformance runner for `floats/edges.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceFloatsEdges <record-file>`
 *
 *  Each line holds one directed edge with its lengths and boundary,
 *  compared under the manifest tolerances.
 */

#include "conformance.h"
#include "h3api.h"

static void checkFloatEdge(const cJSON *record) {
    H3Index edge = jsonIndex(record, "index");

    double length;
    H3Error err = H3_EXPORT(edgeLengthRads)(edge, &length);
    assertMeasure(record, "lengthRads", err, length, UNIT_RADS);
    err = H3_EXPORT(edgeLengthKm)(edge, &length);
    assertMeasure(record, "lengthKm", err, length, UNIT_KM);
    err = H3_EXPORT(edgeLengthM)(edge, &length);
    assertMeasure(record, "lengthM", err, length, UNIT_M);

    CellBoundary boundary;
    c_assert(!H3_EXPORT(directedEdgeToBoundary)(edge, &boundary),
             "directedEdgeToBoundary");
    assertPointSequence(jsonItem(record, "boundary"), boundary.verts,
                        boundary.numVerts, "boundary");
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkFloatEdge);
}
