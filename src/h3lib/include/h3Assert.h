/*
 * Copyright 2022 Uber Technologies, Inc.
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
/** @file h3Assert.h
 * @brief   Support code for unit testing and assertions
 *
 * This file defines macros needed for defensive programming in the H3 core
 * library. H3 strives to have complete code and branch coverage, but this is
 * not feasible if some branches cannot be reached because they are defensive -
 * that is, we do not know of a test case that would exercise the branch but we
 * do have an opinion of how to recover from such an error. These defensive
 * branches are excluded from coverage.
 *
 * In other testing, such as unit tests or fuzzer testing, they trigger
 * assertions if the conditions fail.
 *
 * Adapted from https://www.sqlite.org/testing.html and
 * https://www.sqlite.org/assert.html
 *
 * Used under the terms of the SQLite3 project, reproduced below:
 * The author disclaims copyright to this source code.  In place of
 * a legal notice, here is a blessing:
 *
 *    May you do good and not evil.
 *    May you find forgiveness for yourself and forgive others.
 *    May you share freely, never taking more than you give.
 */

#ifndef H3ASSERT_H
#define H3ASSERT_H

#include <assert.h>

/*
** The testcase() macro is used to aid in coverage testing.  When
** doing coverage testing, the condition inside the argument to
** testcase() must be evaluated both true and false in order to
** get full branch coverage.  The testcase() macro is inserted
** to help ensure adequate test coverage in places where simple
** condition/decision coverage is inadequate.  For example, testcase()
** can be used to make sure boundary values are tested.  For
** bitmask tests, testcase() can be used to make sure each bit
** is significant and used at least once.  On switch statements
** where multiple cases go to the same block of code, testcase()
** can insure that all cases are evaluated.
*/
#if defined(H3_COVERAGE_TEST) || defined(H3_DEBUG)
extern unsigned int h3CoverageCounter;
#define testcase(X)                              \
    if (X) {                                     \
        h3CoverageCounter += (unsigned)__LINE__; \
    }
#else
#define testcase(X)
#endif

/*
** Disable ALWAYS() and NEVER() (make them pass-throughs) for coverage
** and mutation testing
*/
#if defined(H3_COVERAGE_TEST)
#define H3_OMIT_AUXILIARY_SAFETY_CHECKS 1
#endif

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#if !defined(NDEBUG) || defined(H3_COVERAGE_TEST)
#define TESTONLY(X) X
#else
#define TESTONLY(X)
#endif

/*
** The DEFENSEONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within ALWAYS() or NEVER() macros.
*/
#if !defined(H3_OMIT_AUXILIARY_SAFETY_CHECKS)
#define DEFENSEONLY(X) X
#else
#define DEFENSEONLY(X)
#endif

/*
** The ALWAYS and NEVER macros surround boolean expressions which
** are intended to always be true or false, respectively.  Such
** expressions could be omitted from the code completely.  But they
** are included in a few cases in order to enhance the resilience
** of the H3 library to unexpected behavior - to make the code "self-healing"
** or "ductile" rather than being "brittle" and crashing at the first
** hint of unplanned behavior.
**
** In other words, ALWAYS and NEVER are added for defensive code.
**
** When doing coverage testing ALWAYS and NEVER are hard-coded to
** be true and false so that the unreachable code they specify will
** not be counted as untested code.
*/
#if defined(H3_OMIT_AUXILIARY_SAFETY_CHECKS)
#define ALWAYS(X) (1)
#define NEVER(X) (0)
#elif !defined(NDEBUG)
#define ALWAYS(X) ((X) ? 1 : (assert(0), 0))
#define NEVER(X) ((X) ? (assert(0), 1) : 0)
#else
#define ALWAYS(X) (X)
#define NEVER(X) (X)
#endif

#endif
