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
/** @file  vertex.h
 *  @brief Functions for working with cell vertexes.
 */

#ifndef H3VERTEX_H
#define H3VERTEX_H

#include "h3Index.h"
#include "faceijk.h"

/** @struct BaseCellRotation
 *  @brief base cell at a given ijk and required rotations into its system
 */
typedef struct {
    int face;      ///< face number
    int ccwRot60;  ///< number of ccw 60 degree rotations for vertices
                   ///  relative to base cell home face
} BaseCellRotation;

int vertexRotations(H3Index cell);
int vertexNumForDirection(const H3Index origin, const Direction direction);

#endif
