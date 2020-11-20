/*
 * Copyright 2017 Uber Technologies, Inc.
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
/** @file mathExtensions.c
 * @brief   Math functions that should've been in math.h but aren't
 */

#include "mathExtensions.h"

/**
 * _ipow does integer exponentiation efficiently. Taken from StackOverflow.
 *
 * @param base the integer base (can be positive or negative)
 * @param exp the integer exponent (should be nonnegative)
 *
 * @return the exponentiated value
 */
int64_t _ipow(int64_t base, int64_t exp) {
    int64_t result = 1;
    while (exp) {
        if (exp & 1) result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}
