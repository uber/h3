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
/** @file alloc.h
 * @brief   Memory management functions
 *
 * This file contains macros and the necessary declarations to be able
 * to point H3 at different memory management functions than the standard
 * malloc/free/etc functions.
 */

#ifndef ALLOC_H
#define ALLOC_H

#include "h3api.h"  // for TJOIN

#ifdef H3_ALLOC_PREFIX
#define H3_MEMORY(name) TJOIN(H3_ALLOC_PREFIX, name)

#ifdef __cplusplus
extern "C" {
#endif

void *H3_MEMORY(malloc)(size_t size);
void *H3_MEMORY(calloc)(size_t num, size_t size);
void *H3_MEMORY(realloc)(void *ptr, size_t size);
void H3_MEMORY(free)(void *ptr);

#ifdef __cplusplus
}
#endif

#else
#define H3_MEMORY(name) name
#endif

#endif
