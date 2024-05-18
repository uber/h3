/*
 * Copyright 2024 Uber Technologies, Inc.
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
 * @brief tests H3 function `describeH3Error`
 *
 *  usage: `testDescribeH3Error`
 *
 *  This program confirms that the `describeH3Error` function will provide
 *  a string output describing the error code (either providing a description
 *  of the error, or telling you that the error code is itself in error)
 */

#include <string.h>

#include "h3Index.h"
#include "test.h"

SUITE(describeH3Error) {
    TEST(noError) {
        H3Error err = E_SUCCESS;
        t_assert(strcmp(H3_EXPORT(describeH3Error)(err), "Success") == 0, "got expected success message");
    }

    TEST(invalidCell) {
        H3Error err = E_CELL_INVALID;
        t_assert(strcmp(H3_EXPORT(describeH3Error)(err), "Cell argument was not valid") == 0, "got expected error message");
    }

    TEST(invalidH3Error) {
        H3Error err = 9001; // Will probably never hit this
        t_assert(strcmp(H3_EXPORT(describeH3Error)(err), "Invalid error code") == 0, "got expected failure message");
    }
}
