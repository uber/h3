/*
 * Copyright 2019 Uber Technologies, Inc.
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
 * @brief tests IJK grid functions
 *
 *  usage: `testCoordIjk`
 */

#include "coordijk.h"
#include "test.h"

SUITE(coordIjk) {
    TEST(_unitIjkToDigit) {
        CoordIJK zero = {0};
        CoordIJK i = {1, 0, 0};
        CoordIJK outOfRange = {2, 0, 0};
        CoordIJK unnormalizedZero = {2, 2, 2};

        t_assert(_unitIjkToDigit(&zero) == CENTER_DIGIT, "Unit IJK to zero");
        t_assert(_unitIjkToDigit(&i) == I_AXES_DIGIT, "Unit IJK to I axis");
        t_assert(_unitIjkToDigit(&outOfRange) == INVALID_DIGIT,
                 "Unit IJK out of range");
        t_assert(_unitIjkToDigit(&unnormalizedZero) == CENTER_DIGIT,
                 "Unnormalized unit IJK to zero");
    }

    TEST(_neighbor) {
        CoordIJK ijk = {0};

        CoordIJK zero = {0};
        CoordIJK i = {1, 0, 0};

        _neighbor(&ijk, CENTER_DIGIT);
        t_assert(_ijkMatches(&ijk, &zero), "Center neighbor is self");
        _neighbor(&ijk, I_AXES_DIGIT);
        t_assert(_ijkMatches(&ijk, &i), "I neighbor as expected");
        _neighbor(&ijk, INVALID_DIGIT);
        t_assert(_ijkMatches(&ijk, &i), "Invalid neighbor is self");
    }
}
