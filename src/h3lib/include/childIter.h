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
/** @file childIter.h
 * @brief An iterator struct/functions for the children of a cell
 */

#ifndef CHILDITER_H
#define CHILDITER_H

#include <stdint.h>

#include "h3Index.h"
#include "h3api.h"

typedef struct {
    H3Index h;
    int pr, cr, fnz;
} ChildIter;

void setup(ChildIter* I, H3Index h, int childRes);
void step(ChildIter* I);

#endif
