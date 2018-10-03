/*
 * Copyright 2018 Uber Technologies, Inc.
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
/** @file localij.h
 * @brief   Local IJ coordinate space functions.
 */

#ifndef LOCALIJ_H
#define LOCALIJ_H

#include "coordijk.h"
#include "h3api.h"

int h3ToLocalIjk(H3Index origin, H3Index h3, CoordIJK* out);
int localIjkToH3(H3Index origin, const CoordIJK* ijk, H3Index* out);

#endif
