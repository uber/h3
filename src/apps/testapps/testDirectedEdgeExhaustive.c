/*
 * Copyright 2020-2021 Uber Technologies, Inc.
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
 * @brief tests H3 directed edge functions using tests over a large number
 *        of indexes.
 *
 *  usage: `testDirectedEdgeExhaustive`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "baseCells.h"
#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

static void directedEdge_correctness_assertions(H3Index h3) {
    H3Index edges[6] = {H3_NULL};
    int pentagon = H3_EXPORT(isPentagon)(h3);
    t_assertSuccess(H3_EXPORT(originToDirectedEdges)(h3, edges));

    for (int i = 0; i < 6; i++) {
        if (pentagon && i == 0) {
            t_assert(edges[i] == H3_NULL, "last pentagon edge is empty");
            continue;
        }
        t_assert(H3_EXPORT(isValidDirectedEdge)(edges[i]) == 1,
                 "edge is an edge");
        H3Index origin;
        t_assertSuccess(H3_EXPORT(getDirectedEdgeOrigin)(edges[i], &origin));
        t_assert(origin == h3, "origin matches input origin");

        H3Index destination;
        t_assertSuccess(
            H3_EXPORT(getDirectedEdgeDestination)(edges[i], &destination));
        int isNeighbor;
        t_assertSuccess(
            H3_EXPORT(areNeighborCells)(h3, destination, &isNeighbor));
        t_assert(isNeighbor, "destination is a neighbor");
    }
}

static void directedEdge_boundary_assertions(H3Index h3) {
    H3Index edges[6] = {H3_NULL};
    t_assertSuccess(H3_EXPORT(originToDirectedEdges)(h3, edges));
    H3Index destination;
    H3Index revEdge;
    CellBoundary edgeBoundary;
    CellBoundary revEdgeBoundary;

    for (int i = 0; i < 6; i++) {
        if (edges[i] == H3_NULL) continue;
        t_assertSuccess(
            H3_EXPORT(directedEdgeToBoundary)(edges[i], &edgeBoundary));
        t_assertSuccess(
            H3_EXPORT(getDirectedEdgeDestination)(edges[i], &destination));
        t_assertSuccess(
            H3_EXPORT(cellsToDirectedEdge)(destination, h3, &revEdge));
        t_assertSuccess(
            H3_EXPORT(directedEdgeToBoundary)(revEdge, &revEdgeBoundary));

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

SUITE(directedEdge) {
    TEST(directedEdge_correctness) {
        iterateAllIndexesAtRes(0, directedEdge_correctness_assertions);
        iterateAllIndexesAtRes(1, directedEdge_correctness_assertions);
        iterateAllIndexesAtRes(2, directedEdge_correctness_assertions);
        iterateAllIndexesAtRes(3, directedEdge_correctness_assertions);
        iterateAllIndexesAtRes(4, directedEdge_correctness_assertions);
    }

    TEST(directedEdge_boundary) {
        iterateAllIndexesAtRes(0, directedEdge_boundary_assertions);
        iterateAllIndexesAtRes(1, directedEdge_boundary_assertions);
        iterateAllIndexesAtRes(2, directedEdge_boundary_assertions);
        iterateAllIndexesAtRes(3, directedEdge_boundary_assertions);
        iterateAllIndexesAtRes(4, directedEdge_boundary_assertions);
        // Res 5: normal base cell
        iterateBaseCellIndexesAtRes(5, directedEdge_boundary_assertions, 0);
        // Res 5: pentagon base cell
        iterateBaseCellIndexesAtRes(5, directedEdge_boundary_assertions, 14);
        // Res 5: polar pentagon base cell
        iterateBaseCellIndexesAtRes(5, directedEdge_boundary_assertions, 117);
        // Res 6: Test one pentagon just to check for new edge cases
        iterateBaseCellIndexesAtRes(6, directedEdge_boundary_assertions, 14);
    }
}
