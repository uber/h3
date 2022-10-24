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
/** @file h3Assert.c
 * @brief   Support code for unit testing
 */

#include "h3Assert.h"

#if defined(H3_COVERAGE_TEST) || defined(H3_DEBUG)
/*
** Counter used for coverage testing.  Does not come into play for
** release builds.
**
** Access to this global variable is not mutex protected.  This might
** result in TSAN warnings.  But as the variable does not exist in
** release builds, that should not be a concern.
*/
unsigned int h3CoverageCounter;
#endif
