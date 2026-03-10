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

#include <stdlib.h>

#include "iterators.h"
#include "mathExtensions.h"
#include "test.h"
#include "utility.h"

// Iterator mechanics:
//   1. produces exact number of edges:
//         num_edges = num_sides * 3^(childRes - parentRes)
//   2. each step produces a different edge
//   3. stays H3_NULL after finishing
void check_iterator_mechanics(H3Index h, int childRes) {
    bool isPent = H3_EXPORT(isPentagon)(h);
    int parentRes = H3_EXPORT(getResolution)(h);
    int numSides = isPent ? 5 : 6;
    int64_t expectedEdges = numSides * _ipow(3, childRes - parentRes);

    IterEdgesGosper iter = iterInitGosper(h, childRes);
    t_assert(iter.remaining == expectedEdges, "correct number of edges");

    int64_t i;
    for (i = 0; iter.e; i++) {
        H3Index prev = iter.e;
        iterStepGosper(&iter);
        t_assert(iter.e != prev, "edge should advance");
    }
    t_assert(i == expectedEdges, "correct number of edges");

    // iterator should stay exhausted
    for (int j = 0; j < 100; j++) {
        t_assert(iter.remaining == 0, "num edges to zero");
        t_assert(iter.e == H3_NULL, "iterator exhausted");
        iterStepGosper(&iter);
    }
}

// Per-edge correctness:
//   1. each edge is a valid directed edge
//   2. each edge is at childRes
//   3. confirm the edge is on the Gosper island boundary:
//      origin cell is a child of parent cell, destination cell is not
void check_edge_correctness(H3Index h, int childRes) {
    int parentRes = H3_EXPORT(getResolution)(h);
    IterEdgesGosper iter = iterInitGosper(h, childRes);

    while (iter.e) {
        t_assert(H3_EXPORT(isValidDirectedEdge)(iter.e), "edge is valid");
        t_assert(H3_EXPORT(getResolution)(iter.e) == childRes,
                 "correct resolution");

        H3Index s, d, ps, pd;
        H3_EXPORT(getDirectedEdgeOrigin)(iter.e, &s);  // source/origin cell
        H3_EXPORT(getDirectedEdgeDestination)(iter.e, &d);  // destination cell
        H3_EXPORT(cellToParent)(s, parentRes, &ps);         // parent of source
        H3_EXPORT(cellToParent)(d, parentRes, &pd);  // parent of destination

        t_assert(ps == h, "origin *is* child of parent cell");
        t_assert(pd != h, "destination *is not* child of parent cell");

        iterStepGosper(&iter);
    }
}

// Check that edge_a's last vertex matches edge_b's first vertex,
// i.e. the edges connect end-to-start. Tolerance is relative to
// the shorter edge length to handle varying resolutions.
// TODO: Replace floating-point distance check with exact vertex comparison
// once an edgeToVertexes function exists.
bool do_edges_connect(H3Index edge_a, H3Index edge_b) {
    double len_a, len_b;
    H3_EXPORT(edgeLengthRads)(edge_a, &len_a);
    H3_EXPORT(edgeLengthRads)(edge_b, &len_b);
    double tol = MIN(len_a, len_b) / 1000.0;

    CellBoundary bd_a, bd_b;
    H3_EXPORT(directedEdgeToBoundary)(edge_a, &bd_a);
    H3_EXPORT(directedEdgeToBoundary)(edge_b, &bd_b);

    LatLng end_a = bd_a.verts[bd_a.numVerts - 1];
    LatLng start_b = bd_b.verts[0];
    double dist = H3_EXPORT(greatCircleDistanceRads)(&end_a, &start_b);

    return dist < tol;
}

// Loop structure:
//   1. consecutive edges share an endpoint (floating-point test)
//   2. loop closes (returns to where it started)
void check_loop_valid(H3Index h, int childRes) {
    IterEdgesGosper iter = iterInitGosper(h, childRes);
    H3Index first_edge = iter.e;
    H3Index prev_edge = iter.e;
    iterStepGosper(&iter);

    while (iter.e) {
        t_assert(do_edges_connect(prev_edge, iter.e), "edges should connect");

        prev_edge = iter.e;
        iterStepGosper(&iter);
    }

    t_assert(do_edges_connect(prev_edge, first_edge), "loop should close");
}

void check_gosper_island(H3Index h, int childRes) {
    check_iterator_mechanics(h, childRes);
    check_edge_correctness(h, childRes);
    check_loop_valid(h, childRes);
}

// Verify iterator produces some cyclic rotation of the expected edges
void check_expected_edges(H3Index h, int childRes, H3Index *expected,
                          int numExpected) {
    IterEdgesGosper iter = iterInitGosper(h, childRes);
    t_assert(iter.remaining == numExpected, "correct number of edges");

    // Find rotation offset: where does the iterator's first edge appear?
    int offset = -1;
    for (int i = 0; i < numExpected; i++) {
        if (expected[i] == iter.e) {
            offset = i;
            break;
        }
    }
    t_assert(offset >= 0, "first edge found in expected array");

    for (int i = 0; iter.e; i++) {
        t_assert(iter.e == expected[(offset + i) % numExpected],
                 "correct edge");
        iterStepGosper(&iter);
    }
}

SUITE(gosper_iter) {
    // Exhaustive test of all cells, resolutions 0,1,2,3,4.
    // Expanding to Gosper islands up to res 4.
    TEST(coarse_resolutions_exhaustive) {
        for (int childRes = 0; childRes <= 4; childRes++) {
            for (int parentRes = 0; parentRes <= childRes; parentRes++) {
                IterCellsResolution cell_iter = iterInitRes(parentRes);
                while (cell_iter.h) {
                    check_gosper_island(cell_iter.h, childRes);
                    iterStepRes(&cell_iter);
                }
            }
        }
    }

    // Since an exhaustive test of all resolutions would take too long,
    // we test a few specific cells at finer resolutions, including
    // expanding up to res 15.
    TEST(finer_resolutions) {
        H3Index cells[] = {
            0x8508000ffffffff,  // res 5 hexagon
            0x85080003fffffff,  // res 5 pentagon

            0x8808000009fffff,  // res 8 hexagon
            0x8808000001fffff,  // res 8 pentagon

            0x8a0800000017fff,  // res 10 hexagon
            0x8a0800000007fff,  // res 10 pentagon

            0x8e754e64992d6c7,  // res 14 hexagon
            0x8e0800000000007,  // res 14 pentagon

            0x8f754e64992d6d8,  // res 15 hexagon
            0x8f0800000000000,  // res 15 pentagon
        };
        for (int i = 0; i < (int)ARRAY_SIZE(cells); i++) {
            int parentRes = H3_EXPORT(getResolution)(cells[i]);
            for (int gap = 0; gap <= 5; gap++) {
                int childRes = parentRes + gap;
                if (childRes > 15) continue;
                check_gosper_island(cells[i], childRes);
            }
        }
    }

    // `check_expected_edges` tests below check for a specific sequence
    // edges for a given cell and resolution input. Any cyclic rotation
    // should pass the test.

    TEST(just_hex_2) {
        H3Index h = 0x820887fffffffff;
        int res = 2;
        H3Index expected[] = {
            0x1320887fffffffff, 0x1120887fffffffff, 0x1520887fffffffff,
            0x1420887fffffffff, 0x1620887fffffffff, 0x1220887fffffffff,
        };
        check_expected_edges(h, res, expected, ARRAY_SIZE(expected));
    }

    TEST(just_pent_2) {
        H3Index h = 0x820807fffffffff;
        int res = 2;
        H3Index expected[] = {
            0x1320807fffffffff, 0x1520807fffffffff, 0x1420807fffffffff,
            0x1620807fffffffff, 0x1220807fffffffff,
        };
        check_expected_edges(h, res, expected, ARRAY_SIZE(expected));
    }

    TEST(pent_edges_2_to_4) {
        H3Index h = 0x820807fffffffff;
        int res = 4;
        H3Index expected[] = {
            0x11408053ffffffff, 0x15408053ffffffff, 0x1140805bffffffff,
            0x1540805bffffffff, 0x1440805bffffffff, 0x15408059ffffffff,
            0x14408059ffffffff, 0x16408059ffffffff, 0x1440805dffffffff,
            0x1540804bffffffff, 0x1440804bffffffff, 0x15408049ffffffff,
            0x14408049ffffffff, 0x16408049ffffffff, 0x1440804dffffffff,
            0x1640804dffffffff, 0x1240804dffffffff, 0x16408045ffffffff,
            0x14408069ffffffff, 0x16408069ffffffff, 0x1440806dffffffff,
            0x1640806dffffffff, 0x1240806dffffffff, 0x16408065ffffffff,
            0x12408065ffffffff, 0x13408065ffffffff, 0x12408067ffffffff,
            0x1640802dffffffff, 0x1240802dffffffff, 0x16408025ffffffff,
            0x12408025ffffffff, 0x13408025ffffffff, 0x12408027ffffffff,
            0x13408027ffffffff, 0x11408027ffffffff, 0x13408023ffffffff,
            0x12408035ffffffff, 0x13408035ffffffff, 0x12408037ffffffff,
            0x13408037ffffffff, 0x11408037ffffffff, 0x13408033ffffffff,
            0x11408033ffffffff, 0x15408033ffffffff, 0x1140803bffffffff,
        };
        check_expected_edges(h, res, expected, ARRAY_SIZE(expected));
    }

    TEST(pent_edges_1_to_3) {
        H3Index h = 0x81083ffffffffff;
        int res = 3;
        H3Index expected[] = {
            0x113082bfffffffff, 0x1330829fffffffff, 0x1130829fffffffff,
            0x1530829fffffffff, 0x113082dfffffffff, 0x153082dfffffffff,
            0x143082dfffffffff, 0x153082cfffffffff, 0x143082cfffffffff,
            0x1530821fffffffff, 0x1130825fffffffff, 0x1530825fffffffff,
            0x1430825fffffffff, 0x1530824fffffffff, 0x1430824fffffffff,
            0x1630824fffffffff, 0x1430826fffffffff, 0x1630826fffffffff,
            0x1430835fffffffff, 0x1530834fffffffff, 0x1430834fffffffff,
            0x1630834fffffffff, 0x1430836fffffffff, 0x1630836fffffffff,
            0x1230836fffffffff, 0x1630832fffffffff, 0x1230832fffffffff,
            0x1630814fffffffff, 0x1430816fffffffff, 0x1630816fffffffff,
            0x1230816fffffffff, 0x1630812fffffffff, 0x1230812fffffffff,
            0x1330812fffffffff, 0x1230813fffffffff, 0x1330813fffffffff,
            0x123081efffffffff, 0x163081afffffffff, 0x123081afffffffff,
            0x133081afffffffff, 0x123081bfffffffff, 0x133081bfffffffff,
            0x113081bfffffffff, 0x1330819fffffffff, 0x1130819fffffffff,
        };
        check_expected_edges(h, res, expected, ARRAY_SIZE(expected));
    }

    TEST(hex_edges_1_to_3) {
        H3Index h = 0x81c67ffffffffff;
        int res = 3;
        H3Index expected[] = {
            0x133c64afffffffff, 0x123c64bfffffffff, 0x133c64bfffffffff,
            0x113c64bfffffffff, 0x133c649fffffffff, 0x113c649fffffffff,
            0x153c649fffffffff, 0x113c64dfffffffff, 0x153c64dfffffffff,
            0x113c66bfffffffff, 0x133c669fffffffff, 0x113c669fffffffff,
            0x153c669fffffffff, 0x113c66dfffffffff, 0x153c66dfffffffff,
            0x143c66dfffffffff, 0x153c66cfffffffff, 0x143c66cfffffffff,
            0x153c661fffffffff, 0x113c665fffffffff, 0x153c665fffffffff,
            0x143c665fffffffff, 0x153c664fffffffff, 0x143c664fffffffff,
            0x163c664fffffffff, 0x143c666fffffffff, 0x163c666fffffffff,
            0x143c675fffffffff, 0x153c674fffffffff, 0x143c674fffffffff,
            0x163c674fffffffff, 0x143c676fffffffff, 0x163c676fffffffff,
            0x123c676fffffffff, 0x163c672fffffffff, 0x123c672fffffffff,
            0x163c654fffffffff, 0x143c656fffffffff, 0x163c656fffffffff,
            0x123c656fffffffff, 0x163c652fffffffff, 0x123c652fffffffff,
            0x133c652fffffffff, 0x123c653fffffffff, 0x133c653fffffffff,
            0x123c65efffffffff, 0x163c65afffffffff, 0x123c65afffffffff,
            0x133c65afffffffff, 0x123c65bfffffffff, 0x133c65bfffffffff,
            0x113c65bfffffffff, 0x133c659fffffffff, 0x113c659fffffffff,
        };
        check_expected_edges(h, res, expected, ARRAY_SIZE(expected));
    }

    TEST(hex_edges_2_to_4) {
        H3Index h = 0x82c64ffffffffff;
        int res = 4;
        H3Index expected[] = {
            0x134c6497ffffffff, 0x114c6497ffffffff, 0x134c6493ffffffff,
            0x114c6493ffffffff, 0x154c6493ffffffff, 0x114c649bffffffff,
            0x154c649bffffffff, 0x144c649bffffffff, 0x154c6499ffffffff,
            0x114c64d3ffffffff, 0x154c64d3ffffffff, 0x114c64dbffffffff,
            0x154c64dbffffffff, 0x144c64dbffffffff, 0x154c64d9ffffffff,
            0x144c64d9ffffffff, 0x164c64d9ffffffff, 0x144c64ddffffffff,
            0x154c64cbffffffff, 0x144c64cbffffffff, 0x154c64c9ffffffff,
            0x144c64c9ffffffff, 0x164c64c9ffffffff, 0x144c64cdffffffff,
            0x164c64cdffffffff, 0x124c64cdffffffff, 0x164c64c5ffffffff,
            0x144c64e9ffffffff, 0x164c64e9ffffffff, 0x144c64edffffffff,
            0x164c64edffffffff, 0x124c64edffffffff, 0x164c64e5ffffffff,
            0x124c64e5ffffffff, 0x134c64e5ffffffff, 0x124c64e7ffffffff,
            0x164c64adffffffff, 0x124c64adffffffff, 0x164c64a5ffffffff,
            0x124c64a5ffffffff, 0x134c64a5ffffffff, 0x124c64a7ffffffff,
            0x134c64a7ffffffff, 0x114c64a7ffffffff, 0x134c64a3ffffffff,
            0x124c64b5ffffffff, 0x134c64b5ffffffff, 0x124c64b7ffffffff,
            0x134c64b7ffffffff, 0x114c64b7ffffffff, 0x134c64b3ffffffff,
            0x114c64b3ffffffff, 0x154c64b3ffffffff, 0x114c64bbffffffff,
        };
        check_expected_edges(h, res, expected, ARRAY_SIZE(expected));
    }
}
