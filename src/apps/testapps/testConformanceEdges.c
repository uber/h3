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
 * @brief Conformance runner for the `edges` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceEdges <record-file>`
 *
 *  Each line holds one valid cell with its directed edges, their
 *  destinations, and `cellsToDirectedEdge` to a target drawn from the
 *  cell's 1-disk, which is the cell itself about one time in seven.
 */

#include "conformance.h"
#include "h3api.h"

#define NUM_EDGES 6

static void checkEdges(const cJSON *record) {
    H3Index h = jsonIndex(record, "index");
    H3Index target = jsonIndex(record, "target");

    H3Index edges[NUM_EDGES] = {0};
    c_assert(!H3_EXPORT(originToDirectedEdges)(h, edges),
             "originToDirectedEdges");
    assertIndexSet(jsonArray(record, "edges"), edges, NUM_EDGES, "edges");
    int64_t count = normalizeIndexSet(edges, NUM_EDGES);

    // destinations follow the edges in ascending order.
    H3Index destinations[NUM_EDGES];
    for (int64_t i = 0; i < count; i++) {
        c_assert(H3_EXPORT(isValidDirectedEdge)(edges[i]),
                 "edges: not a valid directed edge");
        H3Index origin;
        c_assert(!H3_EXPORT(getDirectedEdgeOrigin)(edges[i], &origin),
                 "getDirectedEdgeOrigin");
        c_assert(origin == h, "edges: origin is not the cell");
        c_assert(
            !H3_EXPORT(getDirectedEdgeDestination)(edges[i], &destinations[i]),
            "getDirectedEdgeDestination");
    }
    assertIndexSequence(jsonArray(record, "destinations"), destinations, count,
                        "destinations");

    const cJSON *edgeItem = jsonItem(record, "edge");
    H3Index edge;
    H3Error err = H3_EXPORT(cellsToDirectedEdge)(h, target, &edge);
    if (!expectResult(edgeItem, err, "edge")) {
        assertIndex(edgeItem, edge, "edge");
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkEdges);
}
