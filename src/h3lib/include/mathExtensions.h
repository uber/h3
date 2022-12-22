/*
 * Copyright 2017-2018, 2022 Uber Technologies, Inc.
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
/** @file mathExtensions.h
 * @brief   Math functions that should've been in math.h but aren't
 */

#ifndef MATHEXTENSIONS_H
#define MATHEXTENSIONS_H

#include <stdint.h>

/**
 * MAX returns the maximum of two values.
 */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/** Evaluates to true if a + b would overflow for int32 */
#define ADD_INT32S_OVERFLOWS(a, b) \
    ((a) > 0 ? (INT32_MAX - (a) < (b)) : (INT32_MIN - (a) > (b)))

/** Evaluates to true if a - b would overflow for int32 */
#define SUB_INT32S_OVERFLOWS(a, b) \
    ((a) >= 0 ? (INT32_MIN + (a) >= (b)) : (INT32_MAX + (a) + 1 < (b)))

// Internal functions
int64_t _ipow(int64_t base, int64_t exp);

#endif
