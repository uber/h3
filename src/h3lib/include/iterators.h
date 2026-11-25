/*
 * Copyright 2021 Uber Technologies, Inc.
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

/** @file iterators.h
 * @brief Iterator structs and functions for the children of a cell,
 * or cells at a given resolution.
 */

#ifndef ITERATORS_H
#define ITERATORS_H

#include <stdbool.h>
#include <stdint.h>

#include "h3Index.h"
#include "h3api.h"

/**
 * IterCellsChildren: struct for iterating through the descendants of
 * a given cell.
 *
 * Constructors:
 *
 * Initialize with either `iterInitParent` or `iterInitBaseCellNum`.
 * `iterInitParent` sets up an iterator for all the children of a given
 * parent cell at a given resolution.
 *
 * `iterInitBaseCellNum` sets up an iterator for children cells, given
 * a base cell number (0--121).
 *
 * Iteration:
 *
 * Step iterator with `iterStepChild`.
 * During the lifetime of the `IterCellsChildren`, the current iterate
 * is accessed via the `IterCellsChildren.h` member.
 * When the iterator is exhausted or if there was an error in initialization,
 * `IterCellsChildren.h` will be `H3_NULL` even after calling `iterStepChild`.
 */
typedef struct {
    H3Index h;
    int _parentRes;  // parent resolution
    int _skipDigit;  // this digit skips `1` for pentagons
} IterCellsChildren;

DECLSPEC IterCellsChildren iterInitParent(H3Index h, int childRes);
void _iterInitParent(H3Index h, int childRes, IterCellsChildren *iter);
DECLSPEC IterCellsChildren iterInitBaseCellNum(int baseCellNum, int childRes);

// extract the `res` digit (0--7) of the current cell
static inline int _getResDigit(IterCellsChildren *it, int res) {
    return H3_GET_INDEX_DIGIT(it->h, res);
}

// increment the digit (0--7) at location `res`
// H3_PER_DIGIT_OFFSET == 3
static inline void _incrementResDigit(IterCellsChildren *it, int res) {
    H3Index val = 1;
    val <<= H3_PER_DIGIT_OFFSET * (MAX_H3_RES - res);
    it->h += val;
}

/**
 * Create a fully nulled-out child iterator for when an iterator is exhausted.
 * This helps minimize the chance that a user will depend on the iterator
 * internal state after it's exhausted, like the child resolution, for
 * example.
 */
static inline IterCellsChildren _null_iter(void) {
    return (IterCellsChildren){
        .h = H3_NULL, ._parentRes = -1, ._skipDigit = -1};
}

/**
 * Step a IterCellsChildren to the next child cell.
 * When the iteration is over, IterCellsChildren.h will be H3_NULL.
 * Handles iterating through hexagon and pentagon cells.
 *
 * See iterators.c for a description of the iteration logic.
 */
static inline void iterStepChild(IterCellsChildren *it) {
    // once h == H3_NULL, the iterator returns an infinite sequence of H3_NULL
    if (it->h == H3_NULL) return;

    int childRes = H3_GET_RESOLUTION(it->h);

    _incrementResDigit(it, childRes);

    for (int i = childRes; i >= it->_parentRes; i--) {
        if (i == it->_parentRes) {
            // if we're modifying the parent resolution digit, then we're done
            *it = _null_iter();
            return;
        }

        // PENTAGON_SKIPPED_DIGIT == 1
        if (i == it->_skipDigit &&
            _getResDigit(it, i) == PENTAGON_SKIPPED_DIGIT) {
            // Then we are iterating through the children of a pentagon cell.
            // All children of a pentagon have the property that the first
            // nonzero digit between the parent and child resolutions is
            // not 1.
            // I.e., we never see a sequence like 00001.
            // Thus, we skip the `1` in this digit.
            _incrementResDigit(it, i);
            it->_skipDigit -= 1;
            return;
        }

        // INVALID_DIGIT == 7
        if (_getResDigit(it, i) == INVALID_DIGIT) {
            _incrementResDigit(
                it, i);  // zeros out it[i] and increments it[i-1] by 1
        } else {
            break;
        }
    }
}

/**
 * IterCellsResolution: struct for iterating through all cells at a given
 * resolution
 *
 * Constructor:
 *
 * Initialize with `IterCellsResolution`.
 *
 * Iteration:
 *
 * Step iterator with `iterStepRes`.
 * During the lifetime of the iterator the current iterate
 * is accessed via the `IterCellsResolution.h` member.
 * When the iterator is exhausted or if there was an error in initialization,
 * `IterCellsResolution.h` will be `H3_NULL` even after calling `iterStepRes`.
 */
typedef struct {
    H3Index h;
    int _baseCellNum;
    int _res;
    IterCellsChildren _itC;
} IterCellsResolution;

DECLSPEC IterCellsResolution iterInitRes(int res);
DECLSPEC void iterStepRes(IterCellsResolution *iter);

/**
 * IterEdgesGosper: iterator for the directed edges on the boundary of a cell's
 * child set (the Gosper island outline) at a given resolution.
 *
 * Each yielded edge is a directed edge whose origin cell is a child of the
 * parent cell and whose destination cell is not. The edges form a closed loop
 * in counter-clockwise order around the Gosper island. The starting edge is
 * not guaranteed; any cyclic rotation of the sequence is a valid output.
 *
 * Constructor:
 *
 * Initialize with `iterInitGosper(cell, childRes)`.
 * Requires `childRes >= getResolution(cell)` and a valid cell.
 *
 * Iteration:
 *
 * Step iterator with `iterStepGosper`.
 * The current edge is accessed via the `.e` member.
 * The `.remaining` member gives the number of edges left to yield
 * (including the current edge).
 * When the iterator is exhausted, `.e` will be `H3_NULL` even after
 * further calls to `iterStepGosper`.
 */
typedef struct {
    H3Index e;          // Current directed edge (H3_NULL when exhausted)
    int64_t remaining;  // Number of edges left to yield (including current)

    // Internal state — cyclic position trackers:
    int8_t _edgePos;                  // Position in edge_dir cycle (period 6)
    int8_t _walkPos[MAX_H3_RES + 1];  // Position in walk_digit cycle
                                      // (period 18) per resolution level

    // Additional internal state
    int8_t _parentRes;
    int8_t _childRes;
    bool _isPentagon;
} IterEdgesGosper;

DECLSPEC IterEdgesGosper iterInitGosper(H3Index h, int childRes);
DECLSPEC void iterStepGosper(IterEdgesGosper *iter);

#endif
