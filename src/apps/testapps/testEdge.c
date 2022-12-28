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
/** @file testEdge.c
 * @brief Tests functions for manipulating undirected edge H3Indexes
 *
 * usage: `testEdge`
 */

#include <stdlib.h>

#include "algos.h"
#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

// Fixtures
static LatLng sfGeo = {0.659966917655, -2.1364398519396};

SUITE(edge) {
    TEST(cellsToEdgeAndFriends) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index ring[7] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 1, ring));
        H3Index sf2 = ring[0];

        H3Index edge;
        t_assertSuccess(H3_EXPORT(cellsToEdge)(sf, sf2, &edge));
        H3Index cells[2] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(edgeToCells)(edge, cells));
        t_assert(cells[0] == sf || cells[1] == sf,
                 "One of the cells is the origin");
        t_assert(cells[0] == sf2 || cells[1] == sf2,
                 "One of the cells is the destination");
        t_assert(sf != sf2, "Sanity check for cells not being the same");
        t_assert(cells[0] < cells[1],
                 "Directed edge cells are in normalized order");

        t_assert(H3_EXPORT(edgeToCells)(0, cells) == E_UNDIR_EDGE_INVALID,
                 "edgeToCells fails for invalid edges");
        H3Index invalidEdge;
        setH3Index(&invalidEdge, 1, 4, 0);
        H3_SET_RESERVED_BITS(invalidEdge, INVALID_DIGIT);
        H3_SET_MODE(invalidEdge, H3_EDGE_MODE);
        t_assert(H3_EXPORT(edgeToCells)(invalidEdge, cells) != E_SUCCESS,
                 "edgeToCells fails for invalid edges");

        H3Index largerRing[19] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 2, largerRing));
        H3Index sf3 = largerRing[0];

        H3Index notEdge;
        t_assert(H3_EXPORT(cellsToEdge)(sf, sf3, &notEdge) == E_NOT_NEIGHBORS,
                 "Non-neighbors can't have edges");
    }

    TEST(cellsToEdgeFromPentagon) {
        H3Index pentagons[NUM_PENTAGONS] = {H3_NULL};
        H3Index ring[7] = {H3_NULL};
        H3Index pentagon;
        H3Index edge, edge2;

        for (int res = 0; res < MAX_H3_RES; res++) {
            t_assertSuccess(H3_EXPORT(getPentagons)(res, pentagons));
            for (int p = 0; p < NUM_PENTAGONS; p++) {
                pentagon = pentagons[p];
                t_assertSuccess(H3_EXPORT(gridDisk)(pentagon, 1, ring));

                for (int i = 0; i < 7; i++) {
                    H3Index neighbor = ring[i];
                    if (neighbor == pentagon || neighbor == H3_NULL) continue;
                    t_assertSuccess(
                        H3_EXPORT(cellsToEdge)(pentagon, neighbor, &edge));
                    t_assert(H3_EXPORT(isValidEdge)(edge),
                             "pentagon-to-neighbor is a valid edge");
                    t_assertSuccess(
                        H3_EXPORT(cellsToEdge)(neighbor, pentagon, &edge2));
                    t_assert(H3_EXPORT(isValidEdge)(edge2),
                             "neighbor-to-pentagon is a valid edge");
                    t_assert(edge == edge2,
                             "direction does not matter for edge");
                }
            }
        }
    }

    TEST(isValidEdge) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index ring[7] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(gridRingUnsafe)(sf, 1, ring));
        H3Index sf2 = ring[0];

        H3Index edge;
        t_assertSuccess(H3_EXPORT(cellsToEdge)(sf, sf2, &edge));
        t_assert(H3_EXPORT(isValidEdge)(edge) == 1, "edges validate correctly");
        t_assert(H3_EXPORT(isValidEdge)(sf) == 0, "hexagons do not validate");

        H3Index directedEdge = edge;
        H3_SET_MODE(directedEdge, H3_DIRECTEDEDGE_MODE);
        t_assert(H3_EXPORT(isValidEdge)(directedEdge) == 0,
                 "directed edges do not validate");

        H3Index hexagonWithReserved = sf;
        H3_SET_RESERVED_BITS(hexagonWithReserved, 1);
        t_assert(H3_EXPORT(isValidEdge)(hexagonWithReserved) == 0,
                 "hexagons with reserved bits do not validate");

        H3Index fakeEdge = sf;
        H3_SET_MODE(fakeEdge, H3_EDGE_MODE);
        t_assert(H3_EXPORT(isValidEdge)(fakeEdge) == 0,
                 "edges without an edge specified don't work");
        H3Index invalidEdge = sf;
        H3_SET_MODE(invalidEdge, H3_EDGE_MODE);
        H3_SET_RESERVED_BITS(invalidEdge, INVALID_DIGIT);
        t_assert(H3_EXPORT(isValidEdge)(invalidEdge) == 0,
                 "edges with an invalid edge specified don't work");

        H3Index pentagon = 0x821c07fffffffff;
        H3Index goodPentagonalEdge = pentagon;
        H3_SET_MODE(goodPentagonalEdge, H3_EDGE_MODE);
        H3_SET_RESERVED_BITS(goodPentagonalEdge, 2);
        t_assert(H3_EXPORT(isValidEdge)(goodPentagonalEdge) == 1,
                 "pentagonal edge validates");

        H3Index badPentagonalEdge = goodPentagonalEdge;
        H3_SET_RESERVED_BITS(badPentagonalEdge, 1);
        t_assert(H3_EXPORT(isValidEdge)(badPentagonalEdge) == 0,
                 "missing pentagonal edge does not validate");
        // Case discovered by fuzzer that triggers pentagon deleted direction
        // condition
        H3Index badPentagonalEdge2 = 0x1a53002880009900;
        t_assert(H3_EXPORT(isValidEdge)(badPentagonalEdge2) == 0,
                 "missing pentagonal edge 2 does not validate");

        H3Index highBitEdge = edge;
        H3_SET_HIGH_BIT(highBitEdge, 1);
        t_assert(H3_EXPORT(isValidEdge)(highBitEdge) == 0,
                 "high bit set edge does not validate");
    }

    TEST(cellToEdges) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index edges[6] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(cellToEdges)(sf, edges));

        for (int i = 0; i < 6; i++) {
            t_assert(H3_EXPORT(isValidEdge)(edges[i]) == 1, "edge is an edge");
            H3Index owner, destination;
            owner = edges[i];
            Direction dir = H3_GET_RESERVED_BITS(edges[i]);
            H3_SET_MODE(owner, H3_CELL_MODE);
            H3_SET_RESERVED_BITS(owner, 0);
            int rotations = 0;
            t_assertSuccess(
                h3NeighborRotations(owner, dir, &rotations, &destination));
            t_assert(owner == sf || destination == sf,
                     "original cell is owner or neighbor");
            t_assert(owner < destination, "owning cell sorts first");
            H3Index cells[2] = {0};
            t_assertSuccess(H3_EXPORT(edgeToCells)(edges[i], cells));
            t_assert(owner == cells[0], "owning cell is returned first");
            t_assert(destination == cells[1],
                     "destination cell is returned second");
        }
    }

    TEST(cellToEdges_invalid) {
        // Test case discovered by fuzzer that triggers cellsToEdge to fail
        // within cellToEdges
        H3Index invalid = 0x26262626262600fa;
        H3Index edges[6] = {H3_NULL};
        t_assert(H3_EXPORT(cellToEdges)(invalid, edges) == E_NOT_NEIGHBORS,
                 "cellToEdges fails");
    }

    TEST(getEdgesFromPentagon) {
        H3Index pentagon = 0x821c07fffffffff;
        H3Index edges[6] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(cellToEdges)(pentagon, edges));

        int missingEdgeCount = 0;
        for (int i = 0; i < 6; i++) {
            if (edges[i] == 0) {
                missingEdgeCount++;
            } else {
                t_assert(H3_EXPORT(isValidEdge)(edges[i]) == 1,
                         "edge is an edge");
                H3Index cells[2] = {0};
                t_assertSuccess(H3_EXPORT(edgeToCells)(edges[i], cells));
                t_assert(pentagon == cells[0] || pentagon == cells[1],
                         "origin is correct");
                t_assert(cells[0] < cells[1],
                         "destination is not origin and origin is lower");
            }
        }
        t_assert(missingEdgeCount == 1,
                 "Only one edge was deleted for the pentagon");
    }

    TEST(invalid_pentagon_edge) {
        H3Index pentagonEdge = 0x821c07fffffffff;
        H3_SET_MODE(pentagonEdge, H3_EDGE_MODE);
        H3_SET_RESERVED_BITS(pentagonEdge, K_AXES_DIGIT);
        t_assert(H3_EXPORT(isValidEdge)(pentagonEdge) == 0,
                 "Invalid edge off a pentagon");
    }

    TEST(nonNormalizedEdge) {
        H3Index pentagon = 0x821c07fffffffff;
        H3Index neighbors[7] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(gridDisk)(pentagon, 1, neighbors));

        for (int i = 0; i < 7; i++) {
            if (neighbors[i] != H3_NULL && neighbors[i] != pentagon) {
                H3Index edge;
                t_assertSuccess(
                    H3_EXPORT(cellsToEdge)(pentagon, neighbors[i], &edge));
                H3Index originDestination[2] = {H3_NULL};
                t_assertSuccess(
                    H3_EXPORT(edgeToCells)(edge, originDestination));
                Direction revDir = directionForNeighbor(originDestination[1],
                                                        originDestination[0]);
                H3Index fakeEdge = originDestination[1];
                H3_SET_MODE(fakeEdge, H3_EDGE_MODE);
                H3_SET_RESERVED_BITS(fakeEdge, revDir);
                t_assert(H3_EXPORT(isValidEdge)(fakeEdge) == 0,
                         "Edge in non normalized form is invalid");
            }
        }
    }

    TEST(cellToEdgesFailed) {
        H3Index edges[6] = {H3_NULL};
        t_assert(
            H3_EXPORT(cellToEdges)(0x7fffffffffffffff, edges) == E_CELL_INVALID,
            "cellToEdges of invalid index");
    }

    TEST(edgeToBoundary_invalid) {
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index invalidEdge = sf;
        H3_SET_MODE(invalidEdge, H3_EDGE_MODE);
        CellBoundary cb;
        t_assert(
            H3_EXPORT(edgeToBoundary)(invalidEdge, &cb) == E_UNDIR_EDGE_INVALID,
            "edgeToBoundary fails on invalid edge direction");

        H3Index invalidEdge2 = sf;
        H3_SET_RESERVED_BITS(invalidEdge2, 1);
        H3_SET_BASE_CELL(invalidEdge2, NUM_BASE_CELLS + 1);
        H3_SET_MODE(invalidEdge2, H3_EDGE_MODE);
        t_assert(H3_EXPORT(edgeToBoundary)(invalidEdge2, &cb) != E_SUCCESS,
                 "edgeToBoundary fails on invalid edge indexing digit");
    }

    TEST(directedEdgeToEdge) {
        H3Index edge;
        t_assert(H3_EXPORT(directedEdgeToEdge)(0, &edge) == E_DIR_EDGE_INVALID,
                 "can't convert 0");
        H3Index sf;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sfGeo, 9, &sf));
        H3Index edges[6] = {H3_NULL};
        t_assertSuccess(H3_EXPORT(originToDirectedEdges)(sf, edges));
        for (int i = 0; i < 6; i++) {
            t_assertSuccess(H3_EXPORT(directedEdgeToEdge)(edges[i], &edge));
            t_assert(H3_EXPORT(isValidEdge)(edge) == 1,
                     "resulting edge is valid");
            H3Index originDestination[2];
            t_assertSuccess(H3_EXPORT(edgeToCells)(edge, originDestination));
            t_assert(originDestination[0] == sf || originDestination[1] == sf,
                     "one of the cells is the origin");
            t_assert(originDestination[0] != originDestination[1],
                     "there is another cell");
            int areNeighbors;
            t_assertSuccess(H3_EXPORT(areNeighborCells)(
                originDestination[0], originDestination[1], &areNeighbors));
            t_assert(areNeighbors == 1, "are neighbors");
        }
    }
}
