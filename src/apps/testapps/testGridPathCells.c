/*
 * Copyright 2018-2020 Uber Technologies, Inc.
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
/** @file
 * @brief tests H3 grid path function.
 *
 *  usage: `testGridPathCells`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h3Index.h"
#include "h3api.h"
#include "localij.h"
#include "test.h"
#include "utility.h"

SUITE(gridPathCells) {
    TEST(gridPathCells_acrossMultipleFaces) {
        H3Index start = 0x85285aa7fffffff;
        H3Index end = 0x851d9b1bfffffff;

        int64_t size;
        H3Error lineError = H3_EXPORT(gridPathCellsSize)(start, end, &size);
        t_assert(lineError == E_FAILED,
                 "Line not computable across multiple icosa faces");
    }

    TEST(gridPathCells_pentagon) {
        H3Index start = 0x820807fffffffff;
        H3Index end = 0x8208e7fffffffff;

        int64_t size;
        t_assertSuccess(H3_EXPORT(gridPathCellsSize)(start, end, &size));
        H3Index *path = calloc(sizeof(H3Index), size);
        t_assert(H3_EXPORT(gridPathCells)(start, end, path) == E_PENTAGON,
                 "Line not computable due to pentagon distortion");
        free(path);
    }
}
