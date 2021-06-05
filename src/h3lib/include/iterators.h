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
DECLSPEC IterCellsChildren iterInitBaseCellNum(int baseCellNum, int childRes);
DECLSPEC void iterStepChild(IterCellsChildren *iter);

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

#endif
