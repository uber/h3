/*
 * Copyright 2017-2021 Uber Technologies, Inc.
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
 * @brief tests H3 functions `cellToChildren` and `cellToChildrenSize`
 *
 *  usage: `testCellToChildren`
 */

#include <stdlib.h>

#include "h3api.h"
#include "test.h"

static void assertNoDuplicates(H3Index *cells, int n) {
    for (int i = 0; i < n; i++) {
        if (cells[i] == H3_NULL) continue;
        t_assert(H3_EXPORT(isValidCell)(cells[i]), "must be valid H3 cell");
        for (int j = i + 1; j < n; j++) {
            t_assert(cells[i] != cells[j], "can't have duplicate cells in set");
        }
    }
}

// assert that set1 is a subset of set2
static void assertSubset(H3Index *set1, int len1, H3Index *set2, int len2) {
    assertNoDuplicates(set1, len1);

    for (int i = 0; i < len1; i++) {
        if (set1[i] == H3_NULL) continue;

        bool present = false;
        for (int j = 0; j < len2; j++) {
            if (set1[i] == set2[j]) {
                present = true;
                break;
            };
        }
        t_assert(present, "children must match");
    }
}

/*
Assert that two arrays of h3 cells are equal sets:
    - No duplicate cells allowed.
    - Ignore zero elements (so array sizes may be different).
    - Ignore array order.
 */
static void assertSetsEqual(H3Index *set1, int len1, H3Index *set2, int len2) {
    assertSubset(set1, len1, set2, len2);
    assertSubset(set2, len2, set1, len1);
}

static void checkChildren(H3Index h, int res, H3Error expectedError,
                          H3Index *expected, int numExpected) {
    int64_t numChildren;
    H3Error numChildrenError =
        H3_EXPORT(cellToChildrenSize)(h, res, &numChildren);
    t_assert(numChildrenError == expectedError, "Expected error code");
    if (expectedError != E_SUCCESS) {
        return;
    }
    H3Index *children = calloc(numChildren, sizeof(H3Index));
    t_assertSuccess(H3_EXPORT(cellToChildren)(h, res, children));

    assertSetsEqual(children, numChildren, expected, numExpected);

    free(children);
}

SUITE(cellToChildren_new) {
    TEST(oneResStep) {
        H3Index h = 0x88283080ddfffff;
        int res = 9;

        H3Index expected[] = {0x89283080dc3ffff, 0x89283080dc7ffff,
                              0x89283080dcbffff, 0x89283080dcfffff,
                              0x89283080dd3ffff, 0x89283080dd7ffff,
                              0x89283080ddbffff};

        checkChildren(h, res, E_SUCCESS, expected,
                      sizeof(expected) / sizeof(H3Index));
    }

    TEST(multipleResSteps) {
        H3Index h = 0x88283080ddfffff;
        int res = 10;

        H3Index expected[] = {
            0x8a283080dd27fff, 0x8a283080dd37fff, 0x8a283080dc47fff,
            0x8a283080dcdffff, 0x8a283080dc5ffff, 0x8a283080dc27fff,
            0x8a283080ddb7fff, 0x8a283080dc07fff, 0x8a283080dd8ffff,
            0x8a283080dd5ffff, 0x8a283080dc4ffff, 0x8a283080dd47fff,
            0x8a283080dce7fff, 0x8a283080dd1ffff, 0x8a283080dceffff,
            0x8a283080dc6ffff, 0x8a283080dc87fff, 0x8a283080dcaffff,
            0x8a283080dd2ffff, 0x8a283080dcd7fff, 0x8a283080dd9ffff,
            0x8a283080dd6ffff, 0x8a283080dcc7fff, 0x8a283080dca7fff,
            0x8a283080dccffff, 0x8a283080dd77fff, 0x8a283080dc97fff,
            0x8a283080dd4ffff, 0x8a283080dd97fff, 0x8a283080dc37fff,
            0x8a283080dc8ffff, 0x8a283080dcb7fff, 0x8a283080dcf7fff,
            0x8a283080dd87fff, 0x8a283080dda7fff, 0x8a283080dc9ffff,
            0x8a283080dc77fff, 0x8a283080dc67fff, 0x8a283080dc57fff,
            0x8a283080ddaffff, 0x8a283080dd17fff, 0x8a283080dc17fff,
            0x8a283080dd57fff, 0x8a283080dc0ffff, 0x8a283080dd07fff,
            0x8a283080dc1ffff, 0x8a283080dd0ffff, 0x8a283080dc2ffff,
            0x8a283080dd67fff};

        checkChildren(h, res, E_SUCCESS, expected,
                      sizeof(expected) / sizeof(H3Index));
    }

    TEST(sameRes) {
        H3Index h = 0x88283080ddfffff;
        int res = 8;

        H3Index expected[] = {h};

        checkChildren(h, res, E_SUCCESS, expected,
                      sizeof(expected) / sizeof(H3Index));
    }

    TEST(childResTooCoarse) {
        H3Index h = 0x88283080ddfffff;
        int res = 7;

        H3Index expected[] = {0};  // empty set; zeros are ignored

        checkChildren(h, res, E_RES_DOMAIN, expected,
                      sizeof(expected) / sizeof(H3Index));
    }

    TEST(childResTooFine) {
        H3Index h = 0x8f283080dcb0ae2;  // res 15 cell
        int res = MAX_H3_RES + 1;

        H3Index expected[] = {0};  // empty set; zeros are ignored

        checkChildren(h, res, E_RES_DOMAIN, expected,
                      sizeof(expected) / sizeof(H3Index));
    }

    TEST(pentagonChildren) {
        H3Index h = 0x81083ffffffffff;  // res 1 pentagon
        int res = 3;

        H3Index expected[] = {
            0x830800fffffffff, 0x830802fffffffff, 0x830803fffffffff,
            0x830804fffffffff, 0x830805fffffffff, 0x830806fffffffff,
            0x830810fffffffff, 0x830811fffffffff, 0x830812fffffffff,
            0x830813fffffffff, 0x830814fffffffff, 0x830815fffffffff,
            0x830816fffffffff, 0x830818fffffffff, 0x830819fffffffff,
            0x83081afffffffff, 0x83081bfffffffff, 0x83081cfffffffff,
            0x83081dfffffffff, 0x83081efffffffff, 0x830820fffffffff,
            0x830821fffffffff, 0x830822fffffffff, 0x830823fffffffff,
            0x830824fffffffff, 0x830825fffffffff, 0x830826fffffffff,
            0x830828fffffffff, 0x830829fffffffff, 0x83082afffffffff,
            0x83082bfffffffff, 0x83082cfffffffff, 0x83082dfffffffff,
            0x83082efffffffff, 0x830830fffffffff, 0x830831fffffffff,
            0x830832fffffffff, 0x830833fffffffff, 0x830834fffffffff,
            0x830835fffffffff, 0x830836fffffffff};

        checkChildren(h, res, E_SUCCESS, expected,
                      sizeof(expected) / sizeof(H3Index));
    }
}
