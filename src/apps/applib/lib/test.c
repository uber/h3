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
/** @file test.c
 * @brief Test harness functions
 */

#include "test.h"
#include <assert.h>
#include <stdio.h>

// Assert

int globalTestCount = 0;
void t_assert(int value, const char* msg) {
    if (!value) {
        printf("%s\n", msg);
    }
    assert(value);
    globalTestCount++;
    printf(".");
}

int testCount() { return globalTestCount; }
