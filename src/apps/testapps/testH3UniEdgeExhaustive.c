/*
 * Copyright 2020 Uber Technologies, Inc.
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
 * @brief tests H3 unidirectional edge functions using tests over a large number
 *        of indexes.
 *
 *  usage: `testH3UniEdgeExhaustive`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "baseCells.h"
#include "constants.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

static void h3UniEdge_correctness_assertions(H3Index h3) {
    H3Index edges[6] = {H3_NULL};
    int isPentagon = H3_EXPORT(h3IsPentagon)(h3);
    H3_EXPORT(getH3UnidirectionalEdgesFromHexagon)(h3, edges);
    H3Index destination;

    for (int i = 0; i < 6; i++) {
        if (isPentagon && i == 0) {
            t_assert(edges[i] == H3_NULL, "last pentagon edge is empty");
            continue;
        }
        t_assert(H3_EXPORT(h3UnidirectionalEdgeIsValid)(edges[i]) == 1,
                 "edge is an edge");
        t_assert(
            H3_EXPORT(getOriginH3IndexFromUnidirectionalEdge)(edges[i]) == h3,
            "origin matches input origin");

        destination =
            H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(edges[i]);
        t_assert(H3_EXPORT(h3IndexesAreNeighbors)(h3, destination),
                 "destination is a neighbor");
    }
}

static void h3UniEdge_boundary_assertions(H3Index h3) {
    H3Index edges[6] = {H3_NULL};
    H3_EXPORT(getH3UnidirectionalEdgesFromHexagon)(h3, edges);
    H3Index destination;
    H3Index revEdge;
    GeoBoundary edgeBoundary;
    GeoBoundary revEdgeBoundary;

    for (int i = 0; i < 6; i++) {
        if (edges[i] == H3_NULL) continue;
        H3_EXPORT(getH3UnidirectionalEdgeBoundary)(edges[i], &edgeBoundary);
        destination =
            H3_EXPORT(getDestinationH3IndexFromUnidirectionalEdge)(edges[i]);
        revEdge = H3_EXPORT(getH3UnidirectionalEdge)(destination, h3);
        H3_EXPORT(getH3UnidirectionalEdgeBoundary)(revEdge, &revEdgeBoundary);

        t_assert(edgeBoundary.numVerts == revEdgeBoundary.numVerts,
                 "numVerts is equal for edge and reverse");

        for (int j = 0; j < edgeBoundary.numVerts; j++) {
            int almostEqual = geoAlmostEqualThreshold(
                &edgeBoundary.verts[j],
                &revEdgeBoundary.verts[revEdgeBoundary.numVerts - 1 - j],
                0.000001);
            t_assert(almostEqual, "Got expected vertex");
        }
    }
}

SUITE(h3UniEdge) {
    TEST(h3UniEdge_correctness) {
        iterateAllIndexesAtRes(0, h3UniEdge_correctness_assertions);
        iterateAllIndexesAtRes(1, h3UniEdge_correctness_assertions);
        iterateAllIndexesAtRes(2, h3UniEdge_correctness_assertions);
        iterateAllIndexesAtRes(3, h3UniEdge_correctness_assertions);
        iterateAllIndexesAtRes(4, h3UniEdge_correctness_assertions);
    }

    TEST(h3UniEdge_boundary) {
        iterateAllIndexesAtRes(0, h3UniEdge_boundary_assertions);
        iterateAllIndexesAtRes(1, h3UniEdge_boundary_assertions);
        iterateAllIndexesAtRes(2, h3UniEdge_boundary_assertions);
        iterateAllIndexesAtRes(3, h3UniEdge_boundary_assertions);
        iterateAllIndexesAtRes(4, h3UniEdge_boundary_assertions);
        // Res 5: normal base cell
        iterateBaseCellIndexesAtRes(5, h3UniEdge_boundary_assertions, 0);
        // Res 5: pentagon base cell
        iterateBaseCellIndexesAtRes(5, h3UniEdge_boundary_assertions, 14);
        // Res 5: polar pentagon base cell
        iterateBaseCellIndexesAtRes(5, h3UniEdge_boundary_assertions, 117);
        // Res 6: Test one pentagon just to check for new edge cases
        iterateBaseCellIndexesAtRes(6, h3UniEdge_boundary_assertions, 14);
    }
}
