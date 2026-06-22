/*
 * Copyright 2025 Uber Technologies, Inc.
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

/** @file geodesicIterator.h
 * @brief Declares helpers for the geodesic polygon iterator state machine.
 */

#ifndef GEODESIC_ITERATOR_H
#define GEODESIC_ITERATOR_H

#include "h3api.h"
#include "polyfillIterator.h"

/**
 * Advance the geodesic polyfill iterator state for the provided starting cell.
 *
 * @param iter Iterator that owns the geodesic acceleration structure.
 * @param cell The cell to continue iterating from.
 */
void geodesicIteratorStep(IterCellsPolygonCompact *iter, H3Index cell);

/**
 * Release any geodesic polygon state allocated for the iterator.
 *
 * @param iter Iterator that may own geodesic acceleration state.
 */
void geodesicIteratorDestroyState(IterCellsPolygonCompact *iter);

#endif
