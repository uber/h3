/*
 * Copyright 2017-2021 Uber Technologies, Inc.
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
/** @file testDirectedEdge.c
 * @brief Tests functions for manipulating directed edge H3Indexes
 *
 * usage: `testDirectedEdge`
 */

#include <stdlib.h>

#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

// Fixtures
static LatLng sfGeo = {0.659966917655, -2.1364398519396};

SUITE(directedEdge) {
    TEST(areNeighborCells) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index ring[7] = {0};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 1, ring));

        int isNeighbor;
        t_assertSuccess(H3_EXPORT(areNeighborCells)(sf, sf, &isNeighbor));
        t_assert(!isNeighbor, "an index does not neighbor itself");

        int neighbors = 0;
        int64_t neighborsSize;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(1, &neighborsSize));
        for (int64_t i = 0; i < neighborsSize; i++) {
            if (ring[i] != 0) {
                t_assertSuccess(
                    H3_EXPORT(areNeighborCells)(sf, ring[i], &isNeighbor));
                if (isNeighbor) {
                    neighbors++;
                }
            }
        }
        t_assert(neighbors == 6,
                 "got the expected number of neighbors from a k-ring of 1");

        H3Index largerRing[19] = {0};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 2, largerRing));

        neighbors = 0;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(2, &neighborsSize));
        for (int64_t i = 0; i < neighborsSize; i++) {
            if (largerRing[i] != 0) {
                t_assertSuccess(H3_EXPORT(areNeighborCells)(sf, largerRing[i],
                                                            &isNeighbor));
                if (isNeighbor) {
                    neighbors++;
                }
            }
        }
        t_assert(neighbors == 0,
                 "got no neighbors, as expected, from a k-ring of 2");

        H3Index sfBroken = sf;
        H3_SET_MODE(sfBroken, H3_DIRECTEDEDGE_MODE);
        t_assert(H3_EXPORT(areNeighborCells)(sf, sfBroken, &isNeighbor) ==
                     E_CELL_INVALID,
                 "broken H3Indexes can't be neighbors");
        t_assert(H3_EXPORT(areNeighborCells)(sfBroken, sf, &isNeighbor) ==
                     E_CELL_INVALID,
                 "broken H3Indexes can't be neighbors (reversed)");

        H3Index sfBigger;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 7, &sfBigger));
        t_assert(H3_EXPORT(areNeighborCells)(sf, sfBigger, &isNeighbor) ==
                     E_RES_MISMATCH,
                 "hexagons of different resolution can't be neighbors");

        t_assertSuccess(
            H3_EXPORT(areNeighborCells)(ring[2], ring[1], &isNeighbor));
        t_assert(isNeighbor, "hexagons in a ring are neighbors");
    }

    TEST(areNeighborCells_invalid) {
        H3Index origin;
        setH3Index(&origin, 5, 0, CENTER_DIGIT);
        H3Index dest = origin;
        H3_SET_INDEX_DIGIT(origin, 5, INVALID_DIGIT);
        H3_SET_INDEX_DIGIT(dest, 5, JK_AXES_DIGIT);
        int out;

        t_assert(
            H3_EXPORT(areNeighborCells)(origin, dest, &out) == E_CELL_INVALID,
            "Invalid index digit origin is rejected");

        setH3Index(&origin, 5, 4, CENTER_DIGIT);
        dest = origin;
        H3_SET_INDEX_DIGIT(origin, 5, K_AXES_DIGIT);
        H3_SET_INDEX_DIGIT(dest, 5, IK_AXES_DIGIT);
        t_assert(
            H3_EXPORT(areNeighborCells)(origin, dest, &out) == E_CELL_INVALID,
            "Invalid k subsequence origin is rejected");
        H3_SET_INDEX_DIGIT(origin, 5, IK_AXES_DIGIT);
        H3_SET_INDEX_DIGIT(dest, 5, K_AXES_DIGIT);
        t_assert(
            H3_EXPORT(areNeighborCells)(origin, dest, &out) == E_CELL_INVALID,
            "Invalid k subsequence destination is rejected");
        // Can't test origin and dest both having K_AXES_DIGIT as this will be
        // rejected as the same cell.
    }

    TEST(cellsToDirectedEdgeAndFriends) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index ring[7] = {0};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 1, ring));
        H3Index sf2 = ring[0];

        H3Index edge;
        t_assertSuccess(H3_EXPORT(cellsToDirectedEdge)(sf, sf2, &edge));
        H3Index edgeOrigin;
        t_assertSuccess(H3_EXPORT(getDirectedEdgeOrigin)(edge, &edgeOrigin));
        t_assert(sf == edgeOrigin, "can retrieve the origin from the edge");
        H3Index edgeDestination;
        t_assertSuccess(
            H3_EXPORT(getDirectedEdgeDestination)(edge, &edgeDestination));
        t_assert(sf2 == edgeDestination,
                 "can retrieve the destination from the edge");

        H3Index originDestination[2] = {0};
        t_assertSuccess(
            H3_EXPORT(directedEdgeToCells)(edge, originDestination));
        t_assert(originDestination[0] == sf,
                 "got the origin first in the pair request");
        t_assert(originDestination[1] == sf2,
                 "got the destination last in the pair request");

        t_assert(H3_EXPORT(directedEdgeToCells)(0, originDestination) ==
                     E_DIR_EDGE_INVALID,
                 "directedEdgeToCells fails for invalid edges");
        H3Index invalidEdge;
        setH3Index(&invalidEdge, 1, 4, 0);
        H3_SET_RESERVED_BITS(invalidEdge, INVALID_DIGIT);
        H3_SET_MODE(invalidEdge, H3_DIRECTEDEDGE_MODE);
        t_assert(H3_EXPORT(directedEdgeToCells)(invalidEdge,
                                                originDestination) != E_SUCCESS,
                 "directedEdgeToCells fails for invalid edges");

        H3Index largerRing[19] = {0};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 2, largerRing));
        H3Index sf3 = largerRing[0];

        H3Index notEdge;
        t_assert(H3_EXPORT(cellsToDirectedEdge)(sf, sf3, &notEdge) ==
                     E_NOT_NEIGHBORS,
                 "Non-neighbors can't have edges");
    }

    TEST(getDirectedEdgeOriginBadInput) {
        H3Index hexagon = 0x891ea6d6533ffff;

        H3Index out;
        t_assert(H3_EXPORT(getDirectedEdgeOrigin)(hexagon, &out) ==
                     E_DIR_EDGE_INVALID,
                 "getting the origin from a hexagon index returns error");
        t_assert(
            H3_EXPORT(getDirectedEdgeOrigin)(0, &out) == E_DIR_EDGE_INVALID,
            "getting the origin from a null index returns error");
    }

    TEST(getDirectedEdgeOriginBadInput) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index ring[7] = {0};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 1, ring));
        H3Index sf2 = ring[0];

        H3Index edge;
        t_assertSuccess(H3_EXPORT(cellsToDirectedEdge)(sf, sf2, &edge));
        H3_SET_RESERVED_BITS(edge, INVALID_DIGIT);
        H3Index out;
        t_assert(H3_EXPORT(getDirectedEdgeDestination)(edge, &out) == E_FAILED,
                 "Invalid directed edge fails");
    }

    TEST(getDirectedEdgeDestination) {
        H3Index hexagon = 0x891ea6d6533ffff;

        H3Index out;
        t_assert(H3_EXPORT(getDirectedEdgeDestination)(hexagon, &out) ==
                     E_DIR_EDGE_INVALID,
                 "getting the destination from a hexagon index returns 0");
        t_assert(H3_EXPORT(getDirectedEdgeDestination)(0, &out) ==
                     E_DIR_EDGE_INVALID,
                 "getting the destination from a null index returns 0");
    }

    TEST(cellsToDirectedEdgeFromPentagon) {
        H3Index pentagons[NUM_PENTAGONS] = {0};
        H3Index ring[7] = {0};
        H3Index pentagon;
        H3Index edge;

        for (int res = 0; res < MAX_H3_RES; res++) {
            t_assertSuccess(H3_EXPORT(getPentagons)(res, pentagons));
            for (int p = 0; p < NUM_PENTAGONS; p++) {
                pentagon = pentagons[p];
                t_assertSuccess(H3_EXPORT(gridDisk)(pentagon, 1, ring));

                for (int i = 0; i < 7; i++) {
                    H3Index neighbor = ring[i];
                    if (neighbor == pentagon || neighbor == H3_NULL) continue;
                    t_assertSuccess(H3_EXPORT(cellsToDirectedEdge)(
                        pentagon, neighbor, &edge));
                    t_assert(H3_EXPORT(isValidDirectedEdge)(edge),
                             "pentagon-to-neighbor is a valid edge");
                    t_assertSuccess(H3_EXPORT(cellsToDirectedEdge)(
                        neighbor, pentagon, &edge));
                    t_assert(H3_EXPORT(isValidDirectedEdge)(edge),
                             "neighbor-to-pentagon is a valid edge");
                }
            }
        }
    }

    TEST(isValidDirectedEdge) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index ring[7] = {0};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 1, ring));
        H3Index sf2 = ring[0];

        H3Index edge;
        t_assertSuccess(H3_EXPORT(cellsToDirectedEdge)(sf, sf2, &edge));
        t_assert(H3_EXPORT(isValidDirectedEdge)(edge) == 1,
                 "edges validate correctly");
        t_assert(H3_EXPORT(isValidDirectedEdge)(sf) == 0,
                 "hexagons do not validate");

        H3Index undirectedEdge = edge;
        H3_SET_MODE(undirectedEdge, H3_EDGE_MODE);
        t_assert(H3_EXPORT(isValidDirectedEdge)(undirectedEdge) == 0,
                 "undirected edges do not validate");

        H3Index hexagonWithReserved = sf;
        H3_SET_RESERVED_BITS(hexagonWithReserved, 1);
        t_assert(H3_EXPORT(isValidDirectedEdge)(hexagonWithReserved) == 0,
                 "hexagons with reserved bits do not validate");

        H3Index fakeEdge = sf;
        H3_SET_MODE(fakeEdge, H3_DIRECTEDEDGE_MODE);
        t_assert(H3_EXPORT(isValidDirectedEdge)(fakeEdge) == 0,
                 "edges without an edge specified don't work");
        H3Index invalidEdge = sf;
        H3_SET_MODE(invalidEdge, H3_DIRECTEDEDGE_MODE);
        H3_SET_RESERVED_BITS(invalidEdge, INVALID_DIGIT);
        t_assert(H3_EXPORT(isValidDirectedEdge)(invalidEdge) == 0,
                 "edges with an invalid edge specified don't work");

        H3Index pentagon = 0x821c07fffffffff;
        H3Index goodPentagonalEdge = pentagon;
        H3_SET_MODE(goodPentagonalEdge, H3_DIRECTEDEDGE_MODE);
        H3_SET_RESERVED_BITS(goodPentagonalEdge, 2);
        t_assert(H3_EXPORT(isValidDirectedEdge)(goodPentagonalEdge) == 1,
                 "pentagonal edge validates");

        H3Index badPentagonalEdge = goodPentagonalEdge;
        H3_SET_RESERVED_BITS(badPentagonalEdge, 1);
        t_assert(H3_EXPORT(isValidDirectedEdge)(badPentagonalEdge) == 0,
                 "missing pentagonal edge does not validate");

        H3Index highBitEdge = edge;
        H3_SET_HIGH_BIT(highBitEdge, 1);
        t_assert(H3_EXPORT(isValidDirectedEdge)(highBitEdge) == 0,
                 "high bit set edge does not validate");
    }

    TEST(originToDirectedEdges) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index edges[6] = {0};
        t_assertSuccess(H3_EXPORT(originToDirectedEdges)(sf, edges));

        for (int i = 0; i < 6; i++) {
            t_assert(H3_EXPORT(isValidDirectedEdge)(edges[i]) == 1,
                     "edge is an edge");
            H3Index origin;
            t_assertSuccess(
                H3_EXPORT(getDirectedEdgeOrigin)(edges[i], &origin));
            t_assert(sf == origin, "origin is correct");
            H3Index destination;
            t_assertSuccess(
                H3_EXPORT(getDirectedEdgeDestination)(edges[i], &destination));
            t_assert(sf != destination, "destination is not origin");
        }
    }

    TEST(getH3DirectedEdgesFromPentagon) {
        H3Index pentagon = 0x821c07fffffffff;
        H3Index edges[6] = {0};
        t_assertSuccess(H3_EXPORT(originToDirectedEdges)(pentagon, edges));

        int missingEdgeCount = 0;
        for (int i = 0; i < 6; i++) {
            if (edges[i] == 0) {
                missingEdgeCount++;
            } else {
                t_assert(H3_EXPORT(isValidDirectedEdge)(edges[i]) == 1,
                         "edge is an edge");
                H3Index origin;
                t_assertSuccess(
                    H3_EXPORT(getDirectedEdgeOrigin)(edges[i], &origin));
                t_assert(pentagon == origin, "origin is correct");
                H3Index destination;
                t_assertSuccess(H3_EXPORT(getDirectedEdgeDestination)(
                    edges[i], &destination));
                t_assert(pentagon != destination, "destination is not origin");
            }
        }
        t_assert(missingEdgeCount == 1,
                 "Only one edge was deleted for the pentagon");
    }

    TEST(directedEdgeToBoundary) {
        H3Index sf;
        CellBoundary boundary;
        CellBoundary edgeBoundary;
        H3Index edges[6] = {0};

        const int expectedVertices[][2] = {{3, 4}, {1, 2}, {2, 3},
                                           {5, 0}, {4, 5}, {0, 1}};

        for (int res = 0; res < MAX_H3_RES; res++) {
            t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, res, &sf));
            t_assertSuccess(H3_EXPORT(cellToBoundary)(sf, &boundary));
            t_assertSuccess(H3_EXPORT(originToDirectedEdges)(sf, edges));

            for (int i = 0; i < 6; i++) {
                t_assertSuccess(
                    H3_EXPORT(directedEdgeToBoundary)(edges[i], &edgeBoundary));
                t_assert(edgeBoundary.numVerts == 2,
                         "Got the expected number of vertices back");
                for (int j = 0; j < edgeBoundary.numVerts; j++) {
                    t_assert(
                        geoAlmostEqual(&edgeBoundary.verts[j],
                                       &boundary.verts[expectedVertices[i][j]]),
                        "Got expected vertex");
                }
            }
        }
    }

    TEST(directedEdgeToBoundaryPentagonClassIII) {
        H3Index pentagon;
        CellBoundary boundary;
        CellBoundary edgeBoundary;
        H3Index edges[6] = {0};

        const int expectedVertices[][3] = {{-1, -1, -1}, {2, 3, 4}, {4, 5, 6},
                                           {8, 9, 0},    {6, 7, 8}, {0, 1, 2}};

        for (int res = 1; res < MAX_H3_RES; res += 2) {
            setH3Index(&pentagon, res, 24, 0);
            t_assertSuccess(H3_EXPORT(cellToBoundary)(pentagon, &boundary));
            t_assertSuccess(H3_EXPORT(originToDirectedEdges)(pentagon, edges));

            int missingEdgeCount = 0;
            for (int i = 0; i < 6; i++) {
                if (edges[i] == 0) {
                    missingEdgeCount++;
                } else {
                    t_assertSuccess(H3_EXPORT(directedEdgeToBoundary)(
                        edges[i], &edgeBoundary));
                    t_assert(edgeBoundary.numVerts == 3,
                             "Got the expected number of vertices back for a "
                             "Class III "
                             "pentagon");
                    for (int j = 0; j < edgeBoundary.numVerts; j++) {
                        t_assert(geoAlmostEqual(
                                     &edgeBoundary.verts[j],
                                     &boundary.verts[expectedVertices[i][j]]),
                                 "Got expected vertex");
                    }
                }
            }
            t_assert(missingEdgeCount == 1,
                     "Only one edge was deleted for the pentagon");
        }
    }

    TEST(directedEdgeToBoundaryPentagonClassII) {
        H3Index pentagon;
        CellBoundary boundary;
        CellBoundary edgeBoundary;
        H3Index edges[6] = {0};

        const int expectedVertices[][3] = {{-1, -1}, {1, 2}, {2, 3},
                                           {4, 0},   {3, 4}, {0, 1}};

        for (int res = 0; res < MAX_H3_RES; res += 2) {
            setH3Index(&pentagon, res, 24, 0);
            t_assertSuccess(H3_EXPORT(cellToBoundary)(pentagon, &boundary));
            t_assertSuccess(H3_EXPORT(originToDirectedEdges)(pentagon, edges));

            int missingEdgeCount = 0;
            for (int i = 0; i < 6; i++) {
                if (edges[i] == 0) {
                    missingEdgeCount++;
                } else {
                    t_assertSuccess(H3_EXPORT(directedEdgeToBoundary)(
                        edges[i], &edgeBoundary));
                    t_assert(edgeBoundary.numVerts == 2,
                             "Got the expected number of vertices back for a "
                             "Class II "
                             "pentagon");
                    for (int j = 0; j < edgeBoundary.numVerts; j++) {
                        t_assert(geoAlmostEqual(
                                     &edgeBoundary.verts[j],
                                     &boundary.verts[expectedVertices[i][j]]),
                                 "Got expected vertex");
                    }
                }
            }
            t_assert(missingEdgeCount == 1,
                     "Only one edge was deleted for the pentagon");
        }
    }

    TEST(directedEdgeToBoundary_invalid) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index invalidEdge = sf;
        H3_SET_MODE(invalidEdge, H3_DIRECTEDEDGE_MODE);
        CellBoundary cb;
        t_assert(H3_EXPORT(directedEdgeToBoundary)(invalidEdge, &cb) ==
                     E_DIR_EDGE_INVALID,
                 "directedEdgeToBoundary fails on invalid edge direction");

        H3Index invalidEdge2 = sf;
        H3_SET_RESERVED_BITS(invalidEdge2, 1);
        H3_SET_BASE_CELL(invalidEdge2, NUM_BASE_CELLS + 1);
        H3_SET_MODE(invalidEdge2, H3_DIRECTEDEDGE_MODE);
        t_assert(
            H3_EXPORT(directedEdgeToBoundary)(invalidEdge2, &cb) != E_SUCCESS,
            "directedEdgeToBoundary fails on invalid edge indexing digit");
    }

    TEST(edgeLength_invalid) {
        double length;
        // Test that invalid inputs do not cause crashes.
        t_assert(H3_EXPORT(edgeLengthRads)(0, &length) == E_DIR_EDGE_INVALID,
                 "Invalid edge has zero length");
        LatLng zero = {0, 0};
        H3Index h3;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&zero, 0, &h3));
        t_assert(H3_EXPORT(edgeLengthRads)(h3, &length) == E_DIR_EDGE_INVALID,
                 "Non-edge (cell) has zero edge length");
    }
}
