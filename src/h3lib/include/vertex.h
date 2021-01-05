/*
 * Copyright 2020-2021 Uber Technologies, Inc.
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
/** @file  vertex.h
 *  @brief Functions for working with cell vertexes.
 */

#ifndef H3VERTEX_H
#define H3VERTEX_H

#include "h3Index.h"
#include "faceijk.h"

/** @struct PentagonDirectionFaces
 *  @brief  The faces in each axial direction of a given pentagon base cell
 */
typedef struct {
    int baseCell;               ///< base cell number
    int faces[NUM_PENT_VERTS];  ///< face numbers for each axial direction,
                                ///  in order, starting with J
} PentagonDirectionFaces;

/** Invalid vertex number */
#define INVALID_VERTEX_NUM -1

/** Max number of faces a base cell's descendants may appear on */
#define MAX_BASE_CELL_FACES 5

int vertexNumForDirection(const H3Index origin, const Direction direction);
Direction directionForVertexNum(const H3Index origin, const int vertexNum);

#endif
