/*
 * Copyright 2017-2020 Uber Technologies, Inc.
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
/** @file testH3UniEdge.c
 * @brief Tests functions for manipulating directed edge H3Indexes
 *
 * usage: `testH3UniEdge`
 */

#include <stdlib.h>

#include "constants.h"
#include "geoCoord.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"

// Fixtures
static GeoCoord sfGeo = {0.659966917655, -2.1364398519396};

SUITE(h3UniEdge) {
    TEST(areNeighborCells) {
        H3Index sf = H3_EXPORT(pointToCell)(&sfGeo, 9);
        H3Index ring[7] = {0};
        H3_EXPORT(gridRingUnsafe)(sf, 1, ring);

        t_assert(H3_EXPORT(areNeighborCells)(sf, sf) == 0,
                 "an index does not neighbor itself");

        int neighbors = 0;
        for (int i = 0; i < H3_EXPORT(maxGridDiskSize)(1); i++) {
            if (ring[i] != 0 && H3_EXPORT(areNeighborCells)(sf, ring[i])) {
                neighbors++;
            }
        }
        t_assert(neighbors == 6,
                 "got the expected number of neighbors from a k-ring of 1");

        H3Index largerRing[19] = {0};
        H3_EXPORT(gridRingUnsafe)(sf, 2, largerRing);

        neighbors = 0;
        for (int i = 0; i < H3_EXPORT(maxGridDiskSize)(2); i++) {
            if (largerRing[i] != 0 &&
                H3_EXPORT(areNeighborCells)(sf, largerRing[i])) {
                neighbors++;
            }
        }
        t_assert(neighbors == 0,
                 "got no neighbors, as expected, from a k-ring of 2");

        H3Index sfBroken = sf;
        H3_SET_MODE(sfBroken, H3_UNIEDGE_MODE);
        t_assert(H3_EXPORT(areNeighborCells)(sf, sfBroken) == 0,
                 "broken H3Indexes can't be neighbors");
        t_assert(H3_EXPORT(areNeighborCells)(sfBroken, sf) == 0,
                 "broken H3Indexes can't be neighbors (reversed)");

        H3Index sfBigger = H3_EXPORT(pointToCell)(&sfGeo, 7);
        t_assert(H3_EXPORT(areNeighborCells)(sf, sfBigger) == 0,
                 "hexagons of different resolution can't be neighbors");

        t_assert(H3_EXPORT(areNeighborCells)(ring[2], ring[1]) == 1,
                 "hexagons in a ring are neighbors");
    }

    TEST(cellsToDirectedEdgeAndFriends) {
        H3Index sf = H3_EXPORT(pointToCell)(&sfGeo, 9);
        H3Index ring[7] = {0};
        H3_EXPORT(gridRingUnsafe)(sf, 1, ring);
        H3Index sf2 = ring[0];

        H3Index edge = H3_EXPORT(cellsToDirectedEdge)(sf, sf2);
        t_assert(sf == H3_EXPORT(getDirectedEdgeOrigin)(edge),
                 "can retrieve the origin from the edge");
        t_assert(sf2 == H3_EXPORT(getDirectedEdgeDestination)(edge),
                 "can retrieve the destination from the edge");

        H3Index originDestination[2] = {0};
        H3_EXPORT(directedEdgeToCells)(edge, originDestination);
        t_assert(originDestination[0] == sf,
                 "got the origin first in the pair request");
        t_assert(originDestination[1] == sf2,
                 "got the destination last in the pair request");

        H3Index largerRing[19] = {0};
        H3_EXPORT(gridRingUnsafe)(sf, 2, largerRing);
        H3Index sf3 = largerRing[0];

        H3Index notEdge = H3_EXPORT(cellsToDirectedEdge)(sf, sf3);
        t_assert(notEdge == 0, "Non-neighbors can't have edges");
    }

    TEST(getDirectedEdgeOriginBadInput) {
        H3Index hexagon = 0x891ea6d6533ffff;

        t_assert(H3_EXPORT(getDirectedEdgeOrigin)(hexagon) == 0,
                 "getting the origin from a hexagon index returns 0");
        t_assert(H3_EXPORT(getDirectedEdgeOrigin)(0) == 0,
                 "getting the origin from a null index returns 0");
    }

    TEST(getDirectedEdgeDestination) {
        H3Index hexagon = 0x891ea6d6533ffff;

        t_assert(H3_EXPORT(getDirectedEdgeDestination)(hexagon) == 0,
                 "getting the destination from a hexagon index returns 0");
        t_assert(H3_EXPORT(getDirectedEdgeDestination)(0) == 0,
                 "getting the destination from a null index returns 0");
    }

    TEST(cellsToDirectedEdgeFromPentagon) {
        H3Index pentagons[NUM_PENTAGONS] = {0};
        H3Index ring[7] = {0};
        H3Index pentagon;
        H3Index edge;

        for (int res = 0; res < MAX_H3_RES; res++) {
            H3_EXPORT(getPentagons)(res, pentagons);
            for (int p = 0; p < NUM_PENTAGONS; p++) {
                pentagon = pentagons[p];
                H3_EXPORT(gridDisk)(pentagon, 1, ring);

                for (int i = 0; i < 7; i++) {
                    H3Index neighbor = ring[i];
                    if (neighbor == pentagon || neighbor == H3_NULL) continue;
                    edge = H3_EXPORT(cellsToDirectedEdge)(pentagon, neighbor);
                    t_assert(H3_EXPORT(isValidDirectedEdge)(edge),
                             "pentagon-to-neighbor is a valid edge");
                    edge = H3_EXPORT(cellsToDirectedEdge)(neighbor, pentagon);
                    t_assert(H3_EXPORT(isValidDirectedEdge)(edge),
                             "neighbor-to-pentagon is a valid edge");
                }
            }
        }
    }

    TEST(isValidDirectedEdge) {
        H3Index sf = H3_EXPORT(pointToCell)(&sfGeo, 9);
        H3Index ring[7] = {0};
        H3_EXPORT(gridRingUnsafe)(sf, 1, ring);
        H3Index sf2 = ring[0];

        H3Index edge = H3_EXPORT(cellsToDirectedEdge)(sf, sf2);
        t_assert(H3_EXPORT(isValidDirectedEdge)(edge) == 1,
                 "edges validate correctly");
        t_assert(H3_EXPORT(isValidDirectedEdge)(sf) == 0,
                 "hexagons do not validate");

        H3Index fakeEdge = sf;
        H3_SET_MODE(fakeEdge, H3_UNIEDGE_MODE);
        t_assert(H3_EXPORT(isValidDirectedEdge)(fakeEdge) == 0,
                 "edges without an edge specified don't work");
        H3Index invalidEdge = sf;
        H3_SET_MODE(invalidEdge, H3_UNIEDGE_MODE);
        H3_SET_RESERVED_BITS(invalidEdge, INVALID_DIGIT);
        t_assert(H3_EXPORT(isValidDirectedEdge)(invalidEdge) == 0,
                 "edges with an invalid edge specified don't work");

        H3Index pentagon = 0x821c07fffffffff;
        H3Index goodPentagonalEdge = pentagon;
        H3_SET_MODE(goodPentagonalEdge, H3_UNIEDGE_MODE);
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
        H3Index sf = H3_EXPORT(pointToCell)(&sfGeo, 9);
        H3Index edges[6] = {0};
        H3_EXPORT(originToDirectedEdges)(sf, edges);

        for (int i = 0; i < 6; i++) {
            t_assert(H3_EXPORT(isValidDirectedEdge)(edges[i]) == 1,
                     "edge is an edge");
            t_assert(sf == H3_EXPORT(getDirectedEdgeOrigin)(edges[i]),
                     "origin is correct");
            t_assert(sf != H3_EXPORT(getDirectedEdgeDestination)(edges[i]),
                     "destination is not origin");
        }
    }

    TEST(getH3DirectedEdgesFromPentagon) {
        H3Index pentagon = 0x821c07fffffffff;
        H3Index edges[6] = {0};
        H3_EXPORT(originToDirectedEdges)(pentagon, edges);

        int missingEdgeCount = 0;
        for (int i = 0; i < 6; i++) {
            if (edges[i] == 0) {
                missingEdgeCount++;
            } else {
                t_assert(H3_EXPORT(isValidDirectedEdge)(edges[i]) == 1,
                         "edge is an edge");
                t_assert(pentagon == H3_EXPORT(getDirectedEdgeOrigin)(edges[i]),
                         "origin is correct");
                t_assert(
                    pentagon != H3_EXPORT(getDirectedEdgeDestination)(edges[i]),
                    "destination is not origin");
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
            sf = H3_EXPORT(pointToCell)(&sfGeo, res);
            H3_EXPORT(cellToBoundary)(sf, &boundary);
            H3_EXPORT(originToDirectedEdges)(sf, edges);

            for (int i = 0; i < 6; i++) {
                H3_EXPORT(directedEdgeToBoundary)
                (edges[i], &edgeBoundary);
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
            H3_EXPORT(cellToBoundary)(pentagon, &boundary);
            H3_EXPORT(originToDirectedEdges)(pentagon, edges);

            int missingEdgeCount = 0;
            for (int i = 0; i < 6; i++) {
                if (edges[i] == 0) {
                    missingEdgeCount++;
                } else {
                    H3_EXPORT(directedEdgeToBoundary)
                    (edges[i], &edgeBoundary);
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
            H3_EXPORT(cellToBoundary)(pentagon, &boundary);
            H3_EXPORT(originToDirectedEdges)(pentagon, edges);

            int missingEdgeCount = 0;
            for (int i = 0; i < 6; i++) {
                if (edges[i] == 0) {
                    missingEdgeCount++;
                } else {
                    H3_EXPORT(directedEdgeToBoundary)(edges[i], &edgeBoundary);
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

    TEST(exactEdgeLength_invalid) {
        // Test that invalid inputs do not cause crashes.
        t_assert(H3_EXPORT(exactEdgeLengthRads)(0) == 0,
                 "Invalid edge has zero length");
        GeoCoord zero = {0, 0};
        H3Index h3 = H3_EXPORT(pointToCell)(&zero, 0);
        t_assert(H3_EXPORT(exactEdgeLengthRads)(h3) == 0,
                 "Non-edge (cell) has zero edge length");
    }
}
