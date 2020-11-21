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

H3Index _zero_out_workspace(H3Index h, int parentRes, int childRes) {
    /* Zero out digits from parentRes + 1 to childRes
     **/
    uint64_t m = ~0;

    m <<= 19 + 3 * parentRes;
    m >>= 19 + 3 * parentRes;
    m >>= 3 * (15 - childRes);
    m <<= 3 * (15 - childRes);
    m = ~m;

    return h & m;
}

void setup(ChildIter* CI, H3Index h, int childRes) {
    CI->pr = H3_GET_RESOLUTION(h);
    CI->cr = childRes;  // we can encode children in the h. eh, maybe too fancy

    if (CI->cr < CI->pr || CI->cr > MAX_H3_RES) {
        // the iterator is empty
        CI->h = 0;
        return;
    }

    h = _zero_out_workspace(h, CI->pr, CI->cr);
    H3_SET_RESOLUTION(h, CI->cr);
    CI->h = h;

    if (H3_EXPORT(h3IsPentagon)(h))
        CI->fnz = CI->cr;
    else
        CI->fnz = -1;
}

int _get(ChildIter* I, int res) {
    int s = 3 * (15 - res);
    uint64_t m = 7;

    m <<= s;

    return (I->h & m) >> s;
}

void _inc(ChildIter* I, int res, uint64_t val) {
    val <<= 3 * (15 - res);
    I->h += val;
}

void step(ChildIter* CI) {
    if (CI->h == 0) return;

    _inc(CI, CI->cr, 1);

    for (int i = CI->cr; i >= CI->pr; i--) {
        if (i == CI->pr) {
            CI->h = 0;
            return;
        }

        if (i == CI->fnz && _get(CI, i) == 1) {
            _inc(CI, i, 1);
            CI->fnz -= 1;  // the skip 1 digit
            return;
        }

        if (_get(CI, i) == 7) {
            _inc(CI, i, 1);  // zeros out A[i] and increments A[i-1] by 1
        } else
            break;
    }
}
