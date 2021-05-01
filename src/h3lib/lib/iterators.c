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

/** @file Iter_Child.c
 * @brief Iterator structs and functions for the children of a cell,
 * or cells at a given resolution.
 */

#include "iterators.h"

#include "h3Index.h"

/*
extract the `res` digit (0--7) of the current cell
 */
static int _get(Iter_Child* it, int res) {
    return H3_GET_INDEX_DIGIT(it->h, res);
}

/*
increment the digit (0--7) at location `res`
 */
static void _inc(Iter_Child* it, int res) {
    uint64_t val = 1;
    val <<= H3_PER_DIGIT_OFFSET * (MAX_H3_RES - res);
    it->h += val;
}

/*
Fully nulled-out child iterator for when an iterator is exhausted.
This helps minimize the chance that a user will depend on the iterator
internal state after it's exhausted, like the child resolution, for
example.
 */
static Iter_Child _null_iter() {
    return (Iter_Child){.h = H3_NULL, ._parentRes = -1, .fnz = -1};
}

/*
Logic for iterating through the children of a cell.


### Step sequence on a hexagon (non-pentagon) cell

                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          92 | 5 | 1 | 0 | 0 | 2 | 5 | ... |

iterStepChild ->

| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          92 | 5 | 1 | 0 | 0 | 2 | 6 | ... |

iterStepChild ->

| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          92 | 5 | 1 | 0 | 0 | 3 | 0 | ... |



### Step sequence on a *pentagon* cell

Pentagon cells have a pentagon base cell number
with all zeros from digit 1 to the digit
corresponding to the cell's resolution.

                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 0 | ... |
                                           \
                                            first nonzero digit

Note that iteration skips 1 whenever we're on the first nonzero digit.
We then move the first nonzero digit up to the next coarser resolution.

iterStepChild ->
                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 2 | ... |
                                       \
                                        first nonzero digit

Iteration is normal (same as a hexagon) if we're not on the first
nonzero digit.

iterStepChild ->
                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 3 | ... |
                                       \
                                        first nonzero digit

...

iterStepChild ->
                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          97 | 0 | 0 | 0 | 0 | 0 | 6 | ... |
                                       \
                                        first nonzero digit

iterStepChild ->

We skip the `1` when we hit the next first nonzero digit.

                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          97 | 0 | 0 | 0 | 0 | 2 | 0 | ... |
                                   \
                                    first nonzero digit

iterStepChild ->

We won't skip the `1` in a previous digit when it
is no longer the first nonzero digit.

                            parent res      child res
                           /               /
| res | base cell # | 1 | 2 | 3 | 4 | 5 | 6 | ... |
|-----|-------------|---|---|---|---|---|---|-----|
|   6 |          97 | 0 | 0 | 0 | 0 | 2 | 1 | ... |
                                   \
                                    first nonzero digit
 */

/*
Initialize a Iter_Child struct representing the sequence giving
the children of cell `h` at resolution `childRes`.

At any point in the iteration, starting once
the struct is initialized, Iter_Child.h gives the current child.

Also, Iter_Child.h == H3_NULL when all the children have been iterated
through, or if the input to `iterInitParent` was invalid.
 */
Iter_Child iterInitParent(H3Index h, int childRes) {
    Iter_Child it;

    it._parentRes = H3_GET_RESOLUTION(h);

    if (childRes < it._parentRes || childRes > MAX_H3_RES || h == H3_NULL) {
        return _null_iter();
    }

    it.h = _zeroIndexDigits(h, it._parentRes + 1, childRes);
    H3_SET_RESOLUTION(it.h, childRes);

    if (H3_EXPORT(isPentagon)(it.h)) {
        // The first nonzero digit skips `1` for pentagons.
        // The "fnz" moves to the left as we count up from the child resolution
        // to the parent resolution.
        it.fnz = childRes;
    } else {
        // if not a pentagon, we can ignore "first nonzero digit" logic
        it.fnz = -1;
    }

    return it;
}

/*
Step a Iter_Child to the next child cell.
When the iteration is over, Iter_Child.h will be H3_NULL.
Handles iterating through hexagon and pentagon cells.
 */
void iterStepChild(Iter_Child* it) {
    // once h == H3_NULL, the iterator returns an infinite sequence of H3_NULL
    if (it->h == H3_NULL) return;

    int childRes = H3_GET_RESOLUTION(it->h);

    _inc(it, childRes);

    for (int i = childRes; i >= it->_parentRes; i--) {
        if (i == it->_parentRes) {
            // if we're modifying the parent resolution digit, then we're done
            *it = _null_iter();
            return;
        }

        // K_AXES_DIGIT == 1
        if (i == it->fnz && _get(it, i) == K_AXES_DIGIT) {
            // Then we are iterating through the children of a pentagon cell.
            // All children of a pentagon have the property that the first
            // nonzero digit between the parent and child resolutions is
            // not 1.
            // I.e., we never see a sequence like 00001.
            // Thus, we skip the `1` in this digit.
            _inc(it, i);
            it->fnz -= 1;
            return;
        }

        // INVALID_DIGIT == 7
        if (_get(it, i) == INVALID_DIGIT) {
            _inc(it, i);  // zeros out it[i] and increments it[i-1] by 1
        } else {
            break;
        }
    }
}

// create iterator for children of base cell at given resolution
Iter_Child iterInitBaseCellNum(int baseCellNum, int childRes) {
    if (baseCellNum < 0 || baseCellNum >= NUM_BASE_CELLS || childRes < 0 ||
        childRes > MAX_H3_RES) {
        return _null_iter();
    }

    H3Index baseCell;
    setH3Index(&baseCell, 0, baseCellNum, 0);

    return iterInitParent(baseCell, childRes);
}

// create iterator for all cells at given resolution
Iter_Res iterInitRes(int res) {
    Iter_Child itC = iterInitBaseCellNum(0, res);

    Iter_Res itR = {.h = itC.h, .baseCellNum = 0, .res = res, .itC = itC};

    return itR;
}

void iterStepRes(Iter_Res* itR) {
    // reached the end of over iterator; emits H3_NULL from now on
    if (itR->h == H3_NULL) return;

    // step child iterator
    iterStepChild(&(itR->itC));

    // If the child iterator is exhausted and there are still
    // base cells remaining, we initialize the next base cell child iterator
    if ((itR->itC.h == H3_NULL) && (itR->baseCellNum + 1 < NUM_BASE_CELLS)) {
        itR->baseCellNum += 1;
        itR->itC = iterInitBaseCellNum(itR->baseCellNum, itR->res);
    }

    // This overall iterator reflects the next cell in the child iterator.
    // Note: This sets itR->h = H3_NULL if the base cells were
    // exhausted in the check above.
    itR->h = itR->itC.h;
}
