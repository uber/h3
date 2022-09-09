/*
 * Copyright 2022 Uber Technologies, Inc.
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
/** @file  edge.h
 *  @brief Edge functions for manipulating (undirected) edge indexes.
 */

#ifndef EDGE_H
#define EDGE_H

#include "algos.h"
#include "h3Index.h"

H3Error wrapDirectedEdgeError(H3Error err);
H3Error cellsToEdgeNonNormalized(H3Index cell1, H3Index cell2, H3Index *out);
H3Index edgeAsDirectedEdge(H3Index edge);
H3Error normalizeEdge(H3Index edge, H3Index *out);

#endif
