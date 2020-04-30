/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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
 * @brief tests hexRange vs. _kRingInternal.
 *
 *  usage: `testH3NeighborRotations resolution [maxK]`
 *
 *  All indexes at `resolution` will be tested. For each index, kRing
 *  of `k` up to `maxK` (default 5) will be run. Standard out will show
 *  the number of cases that returned each return code from hexRange.
 *
 *  If `FAILED` is present in the output, the numbers following it
 *  are the number of test cases that failed validation.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "algos.h"
#include "baseCells.h"
#include "h3Index.h"
#include "utility.h"

/**
 * @brief testH3NeighborRotations validation result
 *
 * Number of cases with the given return code from hexRange.
 */
typedef struct {
    int ret0;
    int ret0ValidationFailures;
    int ret1;
    int ret1ValidationFailures;
    int ret2;
} TestOutput;

void doCell(H3Index h, int maxK, TestOutput* testOutput) {
    for (int k = 0; k < maxK; k++) {
        int maxSz = H3_EXPORT(maxKringSize)(k);
        H3Index* kRingInternalOutput = calloc(sizeof(H3Index), maxSz);
        H3Index* hexRangeOutput = calloc(sizeof(H3Index), maxSz);
        int* kRingInternalDistances = calloc(sizeof(int), maxSz);

        _kRingInternal(h, k, kRingInternalOutput, kRingInternalDistances, maxSz,
                       0);
        int hexRangeFailed = H3_EXPORT(hexRange)(h, k, hexRangeOutput);

        if (hexRangeFailed == 2) {
            testOutput->ret2++;
            continue;
        } else if (hexRangeFailed == 0) {
            testOutput->ret0++;
            int startIdx = 0;
            // i is the current ring number
            for (int i = 0; i <= k; i++) {
                // Number of hexagons on this ring
                int n = i * 6;
                if (i == 0) n = 1;

                for (int ii = 0; ii < n; ii++) {
                    H3Index h2 = hexRangeOutput[ii + startIdx];
                    int found = 0;

                    for (int iii = 0; iii < maxSz; iii++) {
                        if (kRingInternalOutput[iii] == h2 &&
                            kRingInternalDistances[iii] == i) {
                            found = 1;
                            break;
                        }
                    }

                    if (!found) {
                        // Failed to find a hexagon in both outputs, or it had
                        // different distances.
                        testOutput->ret0ValidationFailures++;
                        h3Println(h);
                        return;
                    }
                }

                startIdx += n;
            }
        } else if (hexRangeFailed == 1) {
            testOutput->ret1++;
            int foundPent = 0;
            for (int i = 0; i < maxSz; i++) {
                if (H3_EXPORT(h3IsPentagon)(kRingInternalOutput[i])) {
                    foundPent = 1;
                    break;
                }
            }

            if (!foundPent) {
                // Failed to find the pentagon that caused hexRange
                // to fail.
                printf("NO C k=%d h=%" PRIx64 "\n", k, h);
                testOutput->ret1ValidationFailures++;
                return;
            }
        }

        free(kRingInternalOutput);
        free(hexRangeOutput);
        free(kRingInternalDistances);
    }
}

void recursiveH3IndexToGeo(H3Index h, int res, int maxK,
                           TestOutput* testOutput) {
    for (int d = 0; d < 7; d++) {
        H3_SET_INDEX_DIGIT(h, res, d);

        // skip the pentagonal deleted subsequence

        if (_isBaseCellPentagon(H3_GET_BASE_CELL(h)) &&
            _h3LeadingNonZeroDigit(h) == 1) {
            continue;
        }

        if (res == H3_GET_RESOLUTION(h)) {
            doCell(h, maxK, testOutput);
        } else {
            recursiveH3IndexToGeo(h, res + 1, maxK, testOutput);
        }
    }
}

int main(int argc, char* argv[]) {
    // check command line args
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "usage: %s resolution [maxK]\n", argv[0]);
        exit(1);
    }

    int res = 0;
    if (!sscanf(argv[1], "%d", &res)) error("resolution must be an integer");

    int maxK = 5;
    if (argc > 2) {
        if (!sscanf(argv[2], "%d", &maxK)) error("maxK must be an integer");
    }

    TestOutput testOutput = {0, 0, 0, 0, 0};

    // generate the test cases
    for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
        H3Index rootCell = H3_INIT;
        H3_SET_MODE(rootCell, H3_HEXAGON_MODE);
        H3_SET_BASE_CELL(rootCell, bc);

        if (res == 0) {
            doCell(rootCell, maxK, &testOutput);
        } else {
            int rootRes = H3_GET_RESOLUTION(rootCell);
            H3_SET_RESOLUTION(rootCell, res);
            recursiveH3IndexToGeo(rootCell, rootRes + 1, maxK, &testOutput);
        }
    }

    printf("ret0: %d\nret1: %d\nret2: %d\n", testOutput.ret0, testOutput.ret1,
           testOutput.ret2);
    // ret2 should never occur, as it can only happen if we run over a pentagon
    if (testOutput.ret2 > 0 || testOutput.ret0ValidationFailures ||
        testOutput.ret1ValidationFailures) {
        printf("FAILED\nfailed0: %d\nfailed1: %d\n",
               testOutput.ret0ValidationFailures,
               testOutput.ret1ValidationFailures);
        return 1;
    }
    return 0;
}
