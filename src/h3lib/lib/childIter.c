/*
 * Copyright 2020 Uber Technologies, Inc.
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
 * @brief An iterator struct/functions for the children of a cell
 */

#include "childIter.h"

static int _get(ChildIter* I, int res) {
    // get the `res` index number of the current cell
    int s = 3 * (15 - res);
    uint64_t m = 7;

    m <<= s;

    return (I->h & m) >> s;
}

static void _inc(ChildIter* I, int res) {
    uint64_t val = 1;
    val <<= 3 * (15 - res);
    I->h += val;
}

void setup(ChildIter* CI, H3Index h, int childRes) {
    CI->pr = H3_GET_RESOLUTION(h);
    CI->cr = childRes;

    if (CI->cr < CI->pr || CI->cr > MAX_H3_RES || h == H3_NULL) {
        // make an empty iterator
        CI->h = H3_NULL;
        return;
    }

    h = _zero_index_digits(h, CI->pr + 1, CI->cr);
    H3_SET_RESOLUTION(h, CI->cr);
    CI->h = h;

    if (H3_EXPORT(h3IsPentagon)(h))
        // The first nonzero digit skips `1` for pentagons.
        // The "fnz" moves to the left as we count up from the child resolution
        // to the parent resolution.
        CI->fnz = CI->cr;
    else
        // if not a pentagon, we can ignore "first nonzero digit" logic
        CI->fnz = -1;
}

void step(ChildIter* CI) {
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
            // Do not pass `1`. Do not collect $200.
            // I.e., skip the `1` in this digit
            _inc(CI, i);
            CI->fnz -= 1;
            return;
        }

        if (_get(CI, i) == 7) {
            _inc(CI, i);  // zeros out CI[i] and increments CI[i-1] by 1
        } else
            break;
    }
}
