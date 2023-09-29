/*
 * Copyright 2016-2017, 2020-2021 Uber Technologies, Inc.
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
/** @file bbox.h
 * @brief   Functions relating to the polygon-to-cells functionality
 */

#ifndef POLYFILL_H
#define POLYFILL_H

#include <stdbool.h>

#include "h3api.h"
#include "bbox.h"

/**
 * IterCellsPolygonCompact: struct for iterating through all the cells
 * within a given polygon, outputting a compact set.
 *
 * Constructors:
 *
 * Initialize with `iterInitPolygonCompact`. This will save a reference to the
 * input polygon and allocate memory for data structures used in the
 * iteration. Iterators initialized in this way must be destroyed by
 * `iterDestroyPolygon` to free allocated memory.
 *
 * Iteration:
 *
 * Step iterator with `iterStepPolygonCompact`.
 * During the lifetime of the `iterCellsPolygonCompact`, the current iterate
 * is accessed via the `iterCellsPolygonCompact.cell` member. When the iterator is 
 * exhausted  or if there was an error in initialization or iteration, 
 * `iterCellsPolygonCompact.cell` will be `H3_NULL` after calling `iterStepChild`. 
 * It is the responsibiliy of the caller to check  `iterCellsPolygonCompact.error`
 * when `H3_NULL` is received.
 *
 * Cleanup:
 *
 * Destroy the iterator and free allocated memory with `iterDestroyPolygonCompact`.
 */
typedef struct {
    H3Index cell; // current value
    H3Error error; // error, if any
    int _res;  // target resolution
    uint32_t _flags; // Mode flags for the polygonToCells operation
    GeoPolygon *_polygon; // the polygon we're filling
    BBox *_bboxes; // Bounding box(es) for the polygon and its holes
} IterCellsPolygonCompact;

DECLSPEC IterCellsPolygonCompact iterInitPolygonCompact(GeoPolygon *polygon, int res, uint32_t flags);
DECLSPEC void iterStepPolygonCompact(IterCellsPolygonCompact *iter);
DECLSPEC void iterDestroyPolygonCompact(IterCellsPolygonCompact *iter);

H3Error cellToBBox(H3Index cell, BBox *out, bool coverChildren);

#endif
