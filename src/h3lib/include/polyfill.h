/*
 * Copyright 2023 Uber Technologies, Inc.
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
/** @file polyfill.h
 * @brief   Functions relating to the polygon-to-cells functionality
 */

#ifndef POLYFILL_H
#define POLYFILL_H

#include <stdbool.h>

#include "polyfill_iterator.h"

typedef struct {
    H3Index cell;                       // current value
    H3Error error;                      // error, if any
    IterCellsPolygonCompact _cellIter;  // sub-iterator for compact cells
    IterCellsChildren _childIter;       // sub-iterator for cell children
} IterCellsPolygon;

DECLSPEC IterCellsPolygon iterInitPolygon(const GeoPolygon *polygon, int res,
                                          uint32_t flags);
DECLSPEC void iterStepPolygon(IterCellsPolygon *iter);
DECLSPEC void iterDestroyPolygon(IterCellsPolygon *iter);

H3Error cellToBBox(H3Index cell, BBox *out, bool coverChildren);
H3Index baseCellNumToCell(int baseCellNum);

#endif
