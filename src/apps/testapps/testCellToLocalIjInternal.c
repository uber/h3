/*
 * Copyright 2018-2019 Uber Technologies, Inc.
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
 * @brief tests H3 index to local IJ and IJK+ grid functions.
 *
 *  usage: `testCellToLocalIj`
 */

#include <h3api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algos.h"
#include "baseCells.h"
#include "constants.h"
#include "h3Index.h"
#include "h3api.h"
#include "localij.h"
#include "test.h"
#include "utility.h"

SUITE(h3ToLocalIj) {
    // Some indexes that represent base cells. Base cells
    // are hexagons except for `pent1`.
    H3Index bc1 = H3_INIT;
    setH3Index(&bc1, 0, 15, 0);

    H3Index pent1 = H3_INIT;
    setH3Index(&pent1, 0, 4, 0);

    TEST(ijkBaseCells) {
        CoordIJK ijk;
        t_assert(cellToLocalIjk(pent1, bc1, &ijk) == E_SUCCESS,
                 "got ijk for base cells 4 and 15");
        t_assert(_ijkMatches(&ijk, &UNIT_VECS[2]) == 1,
                 "neighboring base cell at 0,1,0");
    }

    /**
     * Test that coming from the same direction outside the pentagon is handled
     * the same as coming from the same direction inside the pentagon.
     */
    TEST(onOffPentagonSame) {
        for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
            for (int res = 1; res <= MAX_H3_RES; res++) {
                // K_AXES_DIGIT is the first internal direction, and it's also
                // invalid for pentagons, so skip to next.
                Direction startDir = K_AXES_DIGIT;
                if (_isBaseCellPentagon(bc)) {
                    startDir++;
                }

                for (Direction dir = startDir; dir < NUM_DIGITS; dir++) {
                    H3Index internalOrigin;
                    setH3Index(&internalOrigin, res, bc, dir);

                    H3Index externalOrigin;
                    setH3Index(&externalOrigin, res,
                               _getBaseCellNeighbor(bc, dir), CENTER_DIGIT);

                    for (Direction testDir = startDir; testDir < NUM_DIGITS;
                         testDir++) {
                        H3Index testIndex;
                        setH3Index(&testIndex, res, bc, testDir);

                        CoordIJ internalIj;
                        int internalIjFailed = H3_EXPORT(cellToLocalIj)(
                            internalOrigin, testIndex, 0, &internalIj);
                        CoordIJ externalIj;
                        int externalIjFailed = H3_EXPORT(cellToLocalIj)(
                            externalOrigin, testIndex, 0, &externalIj);

                        t_assert(
                            (bool)internalIjFailed == (bool)externalIjFailed,
                            "internal/external failed matches when getting IJ");

                        if (internalIjFailed) {
                            continue;
                        }

                        H3Index internalIndex;
                        int internalIjFailed2 = H3_EXPORT(localIjToCell)(
                            internalOrigin, &internalIj, 0, &internalIndex);
                        H3Index externalIndex;
                        int externalIjFailed2 = H3_EXPORT(localIjToCell)(
                            externalOrigin, &externalIj, 0, &externalIndex);

                        t_assert(
                            (bool)internalIjFailed2 == (bool)externalIjFailed2,
                            "internal/external failed matches when getting "
                            "index");

                        if (internalIjFailed2) {
                            continue;
                        }

                        t_assert(internalIndex == externalIndex,
                                 "internal/external index matches");
                    }
                }
            }
        }
    }
}
