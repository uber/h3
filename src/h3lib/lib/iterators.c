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

/** @file IterCellsChildren.c
 * @brief Iterator structs and functions for the children of a cell,
 * or cells at a given resolution.
 */

#include "iterators.h"

#include "h3Index.h"

// extract the `res` digit (0--7) of the current cell
static int _getResDigit(IterCellsChildren *it, int res) {
    return H3_GET_INDEX_DIGIT(it->h, res);
}

// increment the digit (0--7) at location `res`
// H3_PER_DIGIT_OFFSET == 3
static void _incrementResDigit(IterCellsChildren *it, int res) {
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
static IterCellsChildren _null_iter() {
    return (IterCellsChildren){
        .h = H3_NULL, ._parentRes = -1, ._skipDigit = -1};
}

/**

## Logic for iterating through the children of a cell

We'll describe the logic for ....

- normal (non pentagon iteration)
- pentagon iteration. define "pentagon digit"


### Cell Index Component Diagrams

The lower 56 bits of an H3 Cell Index describe the following index components:

- the cell resolution (4 bits)
- the base cell number (7 bits)
- the child cell digit for each resolution from 1 to 15 (3*15 = 45 bits)

These are the bits we'll be focused on when iterating through child cells.
To help describe the iteration logic, we'll use diagrams displaying the
(decimal) values for each component like:

                            child digit for resolution 2
                           /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   9 |          17 | 5 | 3 | 0 | 6 | 2 | 1 | ... |


### Iteration through children of a hexagon (but not a pentagon)

Iteration through the children of a *hexagon* (but not a pentagon)
simply involves iterating through all the children values (0--6)
for each child digit (up to the child's resolution).

For example, suppose a resolution 3 hexagon index has the following
components:
                                parent resolution
                               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   3 |          17 | 3 | 5 | 1 | 7 | 7 | 7 | ... |

The iteration through all children of resolution 6 would look like:


                                parent res  child res
                               /           /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | ... |
|-----|-------------|---|---|---|---|---|---|---|---|-----|
| 6   |          17 | 3 | 5 | 1 | 0 | 0 | 0 | 7 | 7 | ... |
| 6   |          17 | 3 | 5 | 1 | 0 | 0 | 1 | 7 | 7 | ... |
| ... |             |   |   |   |   |   |   |   |   |     |
| 6   |          17 | 3 | 5 | 1 | 0 | 0 | 6 | 7 | 7 | ... |
| 6   |          17 | 3 | 5 | 1 | 0 | 1 | 0 | 7 | 7 | ... |
| 6   |          17 | 3 | 5 | 1 | 0 | 1 | 1 | 7 | 7 | ... |
| ... |             |   |   |   |   |   |   |   |   |     |
| 6   |          17 | 3 | 5 | 1 | 6 | 6 | 6 | 7 | 7 | ... |


### Step sequence on a *pentagon* cell

Pentagon cells have a base cell number (e.g., 97) corresponding to a
resolution 0 pentagon, and have all zeros from digit 1 to the digit
corresponding to the cell's resolution.
(We'll drop the ellipses from now on, knowing that digits should contain
7's beyond the cell resolution.)

                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 0 |

Iteration through children of a *pentagon* is almost the same
as *hexagon* iteration, except that we skip the *first* 1 value
that appears in the "skip digit". This corresponds to the fact
that a pentagon only has 6 children, which are denoted with
the numbers {0,2,3,4,5,6}.

The skip digit starts at the child resolution position.
When iterating through children more than one resolution below
the parent, we move the skip digit to the left
(up to the next coarser resolution) each time we skip the 1 value
in that digit.

Iteration would start like:

                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 0 |
                                           \
                                            skip digit

Noticing we skip the 1 value and move the skip digit,
the next iterate would be:


| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 2 |
                                       \
                                        skip digit

Iteration continues normally until we get to:


| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 6 |
                                       \
                                        skip digit

which is followed by (skipping the 1):


| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 0 | 2 | 0 |
                                   \
                                    skip digit

For the next iterate, we won't skip the `1` in the previous digit
because it is no longer the skip digit:

| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 0 | 2 | 1 |
                                   \
                                    skip digit

Iteration continues normally until we're right before the next skip
digit:

| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 0 | 6 | 6 |
                                   \
                                    skip digit

Which is followed by

| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 |
|-----|-------------|---|---|---|---|---|---|
|   6 |          97 | 0 | 0 | 0 | 2 | 0 | 0 |
                               \
                                skip digit

and so on.

 */

/**
 * Initialize a IterCellsChildren struct representing the sequence giving
 * the children of cell `h` at resolution `childRes`.
 *
 * At any point in the iteration, starting once
 * the struct is initialized, IterCellsChildren.h gives the current child.
 *
 * Also, IterCellsChildren.h == H3_NULL when all the children have been iterated
 * through, or if the input to `iterInitParent` was invalid.
 */
IterCellsChildren iterInitParent(H3Index h, int childRes) {
    IterCellsChildren it;

    it._parentRes = H3_GET_RESOLUTION(h);

    if (childRes < it._parentRes || childRes > MAX_H3_RES || h == H3_NULL) {
        return _null_iter();
    }

    it.h = _zeroIndexDigits(h, it._parentRes + 1, childRes);
    H3_SET_RESOLUTION(it.h, childRes);

    if (H3_EXPORT(isPentagon)(it.h)) {
        // The skip digit skips `1` for pentagons.
        // The "_skipDigit" moves to the left as we count up from the
        // child resolution to the parent resolution.
        it._skipDigit = childRes;
    } else {
        // if not a pentagon, we can ignore "skip digit" logic
        it._skipDigit = -1;
    }

    return it;
}

/**
 * Step a IterCellsChildren to the next child cell.
 * When the iteration is over, IterCellsChildren.h will be H3_NULL.
 * Handles iterating through hexagon and pentagon cells.
 */
void iterStepChild(IterCellsChildren *it) {
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

// create iterator for children of base cell at given resolution
IterCellsChildren iterInitBaseCellNum(int baseCellNum, int childRes) {
    if (baseCellNum < 0 || baseCellNum >= NUM_BASE_CELLS || childRes < 0 ||
        childRes > MAX_H3_RES) {
        return _null_iter();
    }

    H3Index baseCell;
    setH3Index(&baseCell, 0, baseCellNum, 0);

    return iterInitParent(baseCell, childRes);
}

// create iterator for all cells at given resolution
IterCellsResolution iterInitRes(int res) {
    IterCellsChildren itC = iterInitBaseCellNum(0, res);

    IterCellsResolution itR = {
        .h = itC.h, ._baseCellNum = 0, ._res = res, ._itC = itC};

    return itR;
}

void iterStepRes(IterCellsResolution *itR) {
    // reached the end of over iterator; emits H3_NULL from now on
    if (itR->h == H3_NULL) return;

    // step child iterator
    iterStepChild(&(itR->_itC));

    // If the child iterator is exhausted and there are still
    // base cells remaining, we initialize the next base cell child iterator
    if ((itR->_itC.h == H3_NULL) && (itR->_baseCellNum + 1 < NUM_BASE_CELLS)) {
        itR->_baseCellNum += 1;
        itR->_itC = iterInitBaseCellNum(itR->_baseCellNum, itR->_res);
    }

    // This overall iterator reflects the next cell in the child iterator.
    // Note: This sets itR->h = H3_NULL if the base cells were
    // exhausted in the check above.
    itR->h = itR->_itC.h;
}
