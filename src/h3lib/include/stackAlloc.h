/*
 * Copyright 2016-2018 Uber Technologies, Inc.
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
/** @file stackAlloc.h
 * @brief Macro to provide cross-platform mechanism for allocating variable
 * length arrays on the stack.
 */

#ifndef STACKALLOC_H
#define STACKALLOC_H

#include <assert.h>
#include <string.h>

#ifdef H3_HAVE_VLA

#define STACK_ARRAY_CALLOC(type, name, numElements)        \
    assert((numElements) > 0);                             \
    type name##Buffer[(numElements)];                      \
    memset(name##Buffer, 0, (numElements) * sizeof(type)); \
    type* name = name##Buffer

#elif defined(H3_HAVE_ALLOCA)

#ifdef _MSC_VER

#include <malloc.h>

#define STACK_ARRAY_CALLOC(type, name, numElements)            \
    assert((numElements) > 0);                                 \
    type* name = (type*)_alloca(sizeof(type) * (numElements)); \
    memset(name, 0, sizeof(type) * (numElements))

#else

#include <alloca.h>

#define STACK_ARRAY_CALLOC(type, name, numElements)           \
    assert((numElements) > 0);                                \
    type* name = (type*)alloca(sizeof(type) * (numElements)); \
    memset(name, 0, sizeof(type) * (numElements))

#endif

#else

#error \
    "This platform does not support stack array allocation, please submit an issue on https://github.com/uber/h3 to report this error"

#endif

#endif
