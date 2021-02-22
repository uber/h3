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

#include <stdint.h>

#include "h3api.h"

/*
Iter_Child: struct for iterating through the descendants of
a given cell
 */
typedef struct {
    H3Index h;
    int parentRes;  // parent resolution
    int childRes;   // child resolution; note that we could drop this from the
                   // struct, but we'd have to extract it from h at each
                   // iteration.
    int fnz;  // first nonzero digit (this digit skips `1` for pentagons)
} Iter_Child;

DECLSPEC Iter_Child iterInitParent(H3Index h, int childRes);
DECLSPEC Iter_Child iterInitBaseCellNum(int baseCellNum, int childRes);
DECLSPEC void iterStepChild(Iter_Child* iter);

/*
Iter_Res: struct for iterating through all cells at a given resolution
 */
typedef struct {
    H3Index h;
    int baseCellNum;
    Iter_Child itC;
} Iter_Res;

DECLSPEC Iter_Res iterInitRes(int res);
DECLSPEC void iterStepRes(Iter_Res* iter);

#endif
