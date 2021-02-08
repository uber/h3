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
/** @file childIter.c
 * @brief An iterator struct and functions for the children of a cell
 */

#include "childIter.h"

static int _get(ChildIter* I, int res) {
    // extract the `res` digit (0--7) of the current cell
    int s = 3 * (15 - res);
    uint64_t m = 7;

    m <<= s;

    return (I->h & m) >> s;
}

static void _inc(ChildIter* I, int res) {
    // increment the digit (0--7) at location `res`
    uint64_t val = 1;
    val <<= 3 * (15 - res);
    I->h += val;
}

/*
Initialize a ChildIter struct representing the sequence giving
the children of cell `h` at resolution `childRes`.

At any point in the iteration, starting once
the struct is initialized, ChildIter.h gives the current child.

Also, ChildIter.h == H3_NULL when all the children have been iterated
through, or if the input to `ci_init` was invalid.
 */
ChildIter ci_init(const H3Index h, const int childRes) {
    ChildIter CI;

    CI.pr = H3_GET_RESOLUTION(h);
    CI.cr = childRes;

    if (CI.cr < CI.pr || CI.cr > MAX_H3_RES || h == H3_NULL) {
        // make an empty iterator
        CI.h = H3_NULL;
        return CI;
    }

    CI.h = _zero_index_digits(h, CI.pr + 1, CI.cr);
    H3_SET_RESOLUTION(CI.h, CI.cr);

    if (H3_EXPORT(h3IsPentagon)(CI.h))
        // The first nonzero digit skips `1` for pentagons.
        // The "fnz" moves to the left as we count up from the child resolution
        // to the parent resolution.
        CI.fnz = CI.cr;
    else
        // if not a pentagon, we can ignore "first nonzero digit" logic
        CI.fnz = -1;

    return CI;
}

/*
Step a ChildIter to the next child cell.
When the iteration is over, ChildIter.h will be H3_NULL.
Handles iterating through hexagon and pentagon cells.
 */
void ci_step(ChildIter* CI) {
    // once h == H3_NULL, the iterator returns an infinite sequence of H3_NULL
    if (CI->h == H3_NULL) return;

    _inc(CI, CI->cr);

    for (int i = CI->cr; i >= CI->pr; i--) {
        if (i == CI->pr) {
            // if we're modifying the parent resolution digit, then we're done
            CI->h = H3_NULL;
            return;
        }

        if (i == CI->fnz && _get(CI, i) == 1) {
            // Then we are iterating through the children of a pentagon cell.
            // All children of a pentagon have the property that the first
            // nonzero digit between the parent and child resolutions is
            // not 1.
            // I.e., we never see a sequence like 00001.
            // Thus, we skip the `1` in this digit.
            _inc(CI, i);
            CI->fnz -= 1;
            return;
        }

        if (_get(CI, i) == 7) {
            _inc(CI, i);  // zeros out CI[i] and increments CI[i-1] by 1
        } else {
            break;
        }
    }
}

// create iterator for children of base cell at given resolution
ChildIter base_children_init(const int baseCellNum, const int childRes) {
    if (baseCellNum < 0 || baseCellNum >= NUM_BASE_CELLS || childRes < 0 ||
        childRes > MAX_H3_RES) {
        return (ChildIter){.h = H3_NULL};
    }

    H3Index baseCell = H3_INIT;
    H3_SET_MODE(baseCell, H3_HEXAGON_MODE);
    H3_SET_BASE_CELL(baseCell, baseCellNum);

    return ci_init(baseCell, childRes);
}

// todo: yes, these names are terrible. will change

// create iterator for all cells at given resolution
CellsAtResIter cari_init(int res) {
    ChildIter CI = base_children_init(0, res);

    CellsAtResIter CarI = {.h = CI.h, .baseCellNum = 0, .CI = CI};

    return CarI;
}

void cari_step(CellsAtResIter* CarI) {
    if (CarI->h == H3_NULL) return;

    ci_step(&(CarI->CI));

    if (CarI->CI.h != H3_NULL) {
        CarI->h = CarI->CI.h;
        return;
    }

    // H3_NULL
    CarI->baseCellNum += 1;
    if (CarI->baseCellNum < NUM_BASE_CELLS) {
        CarI->CI = base_children_init(CarI->baseCellNum, CarI->CI.cr);
        CarI->h = CarI->CI.h;
        return;
    } else {
        CarI->h = H3_NULL;
        return;
    }
}
