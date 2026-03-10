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
/** @file iterGosper.c
 * @brief Iterator for the directed edges forming the "Gosper island" outline
 * of a cell's child set at a given resolution.
 *
 * For a cell with resolution `r`, the boundary of the cell's children at
 * resolution `R` forms a closed loop of directed edges (the Gosper island).
 * For a hexagon this is 6 * 3^(R - r) edges; for a pentagon, 5 * 3^(R - r).
 * We refer to `R - r` as the resolution delta.
 *
 * ## Geometric intuition
 *
 * At delta 0, we just return the cell's own edges in counter-clockwise
 * order, cycling through the `edge_dir` array.
 *
 * At delta 1 (for a hexagon), each side of the parent subdivides into 3 edges
 * at the finer resolution, giving 6 * 3 = 18 edges total. The origin cells
 * share the parent's digit path, extended by one digit into the child
 * resolution. The `walk_digit` array maps these 18 walk positions to
 * H3 digit values: the 6 non-center child digits in counter-clockwise order,
 * each repeated 3x for the subdivision.
 *
 * ## Pentagon handling
 *
 * Pentagons have 5 sides instead of 6: the K-axis (H3 digit 1) has no
 * child cell (the deleted subsequence). To handle pentagonal input, we build
 * the digit sequence as if it were a hexagon, then skip over the deleted
 * subsequences (edges that land on the missing K-axis side).
 */

#include <stdbool.h>
#include <stdint.h>

#include "h3Assert.h"
#include "h3Index.h"
#include "iterators.h"
#include "mathExtensions.h"

// H3 digit at each of the 18 boundary walk positions.
// The 6 non-center digits {1,5,4,6,2,3} in counter-clockwise order around
// the hexagon, each repeated 3x for the fractal subdivision at each side.
// Note: walk_digit[i] == edge_dir[(i/3 + 1) % 6]; the same cyclic sequence
// of digits, rotated by 1 and each element repeated 3x.
static const int8_t walk_digit[] = {1, 1, 1, 5, 5, 5, 4, 4, 4,
                                    6, 6, 6, 2, 2, 2, 3, 3, 3};

// H3 edge direction at each edge index, in counter-clockwise order around the
// hexagon. Stored in the directed edge's reserved bits.
static const int8_t edge_dir[] = {3, 1, 5, 4, 6, 2};

/**
 * Advance the walk along origin cells on the Gosper island boundary,
 * updating the child digits.
 *
 * Each resolution level cycles through 18 walk positions (6 sides * 3
 * segments per side). Across resolutions, each step at a coarser level
 * maps to 3 steps at the next finer level. When the coarser level
 * moves to the next cell, the finer level shifts back by 6 positions.
 *
 * The point where each level moves to the next cell depends on
 * resolution parity: Class II (even r) at walkPos % 3 == 0,
 * Class III (odd r) at walkPos % 3 == 1.
 *
 * @return true if the origin cell changed.
 */
static bool advanceOriginCell(int8_t *walkPos, H3Index *h, int8_t r,
                              int8_t parentRes) {
    int prevDigit = H3_GET_INDEX_DIGIT(*h, r);

    // When moving to the next cell, recurse to advance the parent resolution.
    if ((r > parentRes + 1) && (walkPos[r] % 3 == r % 2)) {
        bool parentChanged = advanceOriginCell(walkPos, h, r - 1, parentRes);
        if (parentChanged) {
            // New parent cell: shift back by 6 positions.
            walkPos[r] -= 6;
        }
    }

    // Advance to next position: Increase the cyclic index by one, but
    // we use +19 == +1 (mod 18), so that it stays positive,
    // even after potentially subtracting by 6 above.
    walkPos[r] = (walkPos[r] + 19) % 18;

    // Update the child digit
    int8_t newDigit = walk_digit[walkPos[r]];
    H3_SET_INDEX_DIGIT(*h, r, newDigit);

    // A change in the finest digit is sufficient to detect a cell change.
    return newDigit != prevDigit;
}

/**
 * Advance to the next boundary edge (origin cell + edge direction).
 */
static void advanceEdge(IterEdgesGosper *iter) {
    bool cellChanged = advanceOriginCell(iter->_walkPos, &(iter->e),
                                         iter->_childRes, iter->_parentRes);

    // If we move to a new origin cell, shift the _edgePos back by 2
    if (cellChanged) {
        iter->_edgePos -= 2;
    }

    // Advance to next edge. Increase the cyclic index by one, but
    // we use +7 == +1 (mod 6) so that it stays positive,
    // even after potentially subtracting by 2 above.
    iter->_edgePos = (iter->_edgePos + 7) % 6;
    H3_SET_RESERVED_BITS(iter->e, edge_dir[iter->_edgePos]);
}

// Skip edges corresponding to deleted subsequences of a pentagon.
static void skipPentagonEdges(IterEdgesGosper *iter) {
    while (iter->_isPentagon && ALWAYS(iter->e != H3_NULL) &&
           H3_GET_INDEX_DIGIT(iter->e, iter->_parentRes + 1) == 1) {
        advanceEdge(iter);
    }
}

void iterStepGosper(IterEdgesGosper *iter) {
    if (iter->e == H3_NULL) return;

    iter->remaining--;
    if (iter->remaining <= 0) {
        iter->e = H3_NULL;
        return;
    }

    if (iter->_parentRes == iter->_childRes) {
        // Easy case of 0 resolution delta:
        // Cycle through edge indices, skipping the
        // deleted subsequence for pentagons.
        iter->_edgePos++;
        if (iter->_isPentagon && edge_dir[iter->_edgePos] == 1)
            iter->_edgePos++;

        H3_SET_RESERVED_BITS(iter->e, edge_dir[iter->_edgePos]);
    } else {
        // Resolution delta > 0
        advanceEdge(iter);

        // Skip deleted subsequences for pentagons.
        skipPentagonEdges(iter);
    }
}

// Note: No input validation here. This is an internal function where
// validation is expected to be done upstream. If this is ever added to the
// public API, we'll need to validate h, childRes >= parentRes,
// and childRes <= MAX_H3_RES.
IterEdgesGosper iterInitGosper(H3Index h, int childRes) {
    int parentRes = H3_GET_RESOLUTION(h);
    bool isPent = H3_EXPORT(isPentagon)(h);

    IterEdgesGosper iter = {
        .e = h,
        .remaining = 0,

        ._edgePos = 0,
        ._walkPos = {0},

        ._parentRes = parentRes,
        ._childRes = childRes,
        ._isPentagon = isPent,
    };

    // Set number of edges for the iterator to yield
    int numSides = isPent ? 5 : 6;
    iter.remaining = numSides * _ipow(3, childRes - parentRes);

    // Same-resolution fast path: no digit/resolution setup needed
    if (parentRes == childRes) {
        H3_SET_MODE(iter.e, H3_DIRECTEDEDGE_MODE);
        // Skip the deleted subsequence for pentagons (if edge_dir happens
        // to start there)
        if (NEVER(isPent && edge_dir[iter._edgePos] == 1)) iter._edgePos++;
        H3_SET_RESERVED_BITS(iter.e, edge_dir[iter._edgePos]);
        return iter;
    }

    // Set resolution to child level and initialize boundary walk.
    // The first child level starts at walk position 0.
    // Subsequent levels start at either 14 or 16, depending on
    // if those resolutions are Class II or III.
    H3_SET_RESOLUTION(iter.e, childRes);
    for (int r = parentRes + 1; r <= childRes; r++) {
        if (r == parentRes + 1) {
            iter._walkPos[r] = 0;
        } else {
            iter._walkPos[r] = (r % 2) ? 14 : 16;
        }
        H3_SET_INDEX_DIGIT(iter.e, r, walk_digit[iter._walkPos[r]]);
    }

    // Set edge mode and initial direction
    H3_SET_MODE(iter.e, H3_DIRECTEDEDGE_MODE);
    H3_SET_RESERVED_BITS(iter.e, edge_dir[iter._edgePos]);

    // For pentagons, the walk may start on a deleted subsequence.
    // Advance past those positions to reach the first valid edge.
    skipPentagonEdges(&iter);

    return iter;
}
