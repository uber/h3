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

void setup(ChildIter* I, H3Index h, int parentRes, int childRes, int is_pent) {
    if (childRes < parentRes) {
        // the iterator is empty
        I->h = 0;
        return;
    }

    I->pr = parentRes;
    I->cr = childRes;  // we can encode children in the h. eh, maybe too fancy
    I->h = _zero_out_workspace(h, parentRes, childRes);

    if (is_pent)
        I->fnz = childRes;
    else
        I->fnz = -1;
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

        if (_get(CI, i) == 7) {
            _inc(CI, i, 1);  // zeros out A[i] and increments A[i-1] by 1
        } else if (i == CI->fnz && _get(CI, i) == 6) {
            _inc(CI, i, 2);  // zeros out A[i] and increments A[i-1] by 1
            CI->fnz -= 1;
        } else
            break;
    }
}
