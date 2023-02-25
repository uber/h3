/*
 * Copyright 2022 Uber Technologies, Inc.
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
/** @file testEdgeExhaustive.c
 * @brief Tests functions for manipulating undirected edge H3Indexes
 *
 * usage: `testEdgeExhaustive`
 */

#include <math.h>
#include <stdlib.h>

#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

static void edge_correctness_assertions(H3Index h3) {
    H3Index edges[6] = {H3_NULL};
    int pentagon = H3_EXPORT(isPentagon)(h3);
    t_assertSuccess(H3_EXPORT(cellToEdges(h3, edges)));

    int emptyEdgeCount = 0;
    for (int i = 0; i < 6; i++) {
        if (edges[i] == H3_NULL) {
            emptyEdgeCount++;
            continue;
        }
        t_assert(H3_EXPORT(isValidEdge)(edges[i]) == 1, "edge is an edge");

        H3Index originDestination[2] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(edgeToCells)(edges[i], originDestination));
        t_assert(h3 == originDestination[0] || h3 == originDestination[1],
                 "origin is one of the cells");
        t_assert(originDestination[0] != originDestination[1],
                 "origin and destination are not the same");

        H3Index reencode1, reencode2;
        t_assertSuccess(H3_EXPORT(cellsToEdge)(
            originDestination[0], originDestination[1], &reencode1));
        t_assertSuccess(H3_EXPORT(cellsToEdge)(
            originDestination[1], originDestination[0], &reencode2));
        t_assert(reencode1 == reencode2,
                 "origin and destination produce same the edge either way");
        t_assert(reencode1 == edges[i],
                 "reencoded edges are the same as the original edge");

        int isNeighbor;
        t_assertSuccess(H3_EXPORT(areNeighborCells)(
            originDestination[0], originDestination[1], &isNeighbor));
        t_assert(isNeighbor, "destination is a neighbor");
    }
    if (pentagon) {
        t_assert(emptyEdgeCount == 1, "last pentagon edge is empty");
    } else {
        t_assert(emptyEdgeCount == 0, "non pentagon edges cannot be empty");
    }
}

static void edge_length_assertions(H3Index h3) {
    H3Index edges[6] = {H3_NULL};
    t_assertSuccess(H3_EXPORT(cellToEdges)(h3, edges));

    for (int i = 0; i < 6; i++) {
        if (edges[i] == H3_NULL) continue;

        double length;
        t_assertSuccess(H3_EXPORT(edgeLengthRads)(edges[i], &length));
        double lengthKm;
        t_assertSuccess(H3_EXPORT(edgeLengthKm)(edges[i], &lengthKm));
        double lengthM;
        t_assertSuccess(H3_EXPORT(edgeLengthM)(edges[i], &lengthM));
        double directedEdgeLength;
        H3Index asDirectedEdge = edges[i];
        H3_SET_MODE(asDirectedEdge, H3_DIRECTEDEDGE_MODE);
        t_assertSuccess(
            H3_EXPORT(edgeLengthRads)(asDirectedEdge, &directedEdgeLength));
        t_assert(length > 0, "length is positive");
        t_assert(lengthKm > length, "length in KM is greater than rads");
        t_assert(lengthM > lengthKm, "length in M is greater than KM");
        t_assert(fabs(length - directedEdgeLength) < EPSILON_RAD,
                 "edge and directed edge length are approximately equal");
    }
}

static void edge_boundary_assertions(H3Index h3) {
    H3Index edges[6] = {H3_NULL};
    t_assertSuccess(H3_EXPORT(cellToEdges)(h3, edges));
    H3Index revEdge;
    CellBoundary edgeBoundary;
    CellBoundary revEdgeBoundary;

    for (int i = 0; i < 6; i++) {
        if (edges[i] == H3_NULL) continue;
        t_assertSuccess(H3_EXPORT(edgeToBoundary)(edges[i], &edgeBoundary));
        H3Index originDestination[2] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(edgeToCells)(edges[i], originDestination));
        t_assertSuccess(H3_EXPORT(cellsToDirectedEdge)(
            originDestination[1], originDestination[0], &revEdge));
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

SUITE(edge) {
    TEST(edge_correctness) {
        iterateAllIndexesAtRes(0, edge_correctness_assertions);
        iterateAllIndexesAtRes(1, edge_correctness_assertions);
        iterateAllIndexesAtRes(2, edge_correctness_assertions);
        iterateAllIndexesAtRes(3, edge_correctness_assertions);
        iterateAllIndexesAtRes(4, edge_correctness_assertions);
    }

    TEST(edge_length) {
        iterateAllIndexesAtRes(0, edge_length_assertions);
        iterateAllIndexesAtRes(1, edge_length_assertions);
        iterateAllIndexesAtRes(2, edge_length_assertions);
        iterateAllIndexesAtRes(3, edge_length_assertions);
        iterateAllIndexesAtRes(4, edge_length_assertions);
    }

    TEST(edge_boundary) {
        iterateAllIndexesAtRes(0, edge_boundary_assertions);
        iterateAllIndexesAtRes(1, edge_boundary_assertions);
        iterateAllIndexesAtRes(2, edge_boundary_assertions);
        iterateAllIndexesAtRes(3, edge_boundary_assertions);
        iterateAllIndexesAtRes(4, edge_boundary_assertions);
        // Res 5: normal base cell
        iterateBaseCellIndexesAtRes(5, edge_boundary_assertions, 0);
        // Res 5: pentagon base cell
        iterateBaseCellIndexesAtRes(5, edge_boundary_assertions, 14);
        // Res 5: polar pentagon base cell
        iterateBaseCellIndexesAtRes(5, edge_boundary_assertions, 117);
        // Res 6: Test one pentagon just to check for new edge cases
        iterateBaseCellIndexesAtRes(6, edge_boundary_assertions, 14);
    }
}
