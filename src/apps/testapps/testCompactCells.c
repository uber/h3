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

#include <stdlib.h>

#include "constants.h"
#include "h3Index.h"
#include "test.h"

H3Index sunnyvale = 0x89283470c27ffff;

H3Index uncompactable[] = {0x89283470803ffff, 0x8928347081bffff,
                           0x8928347080bffff};
H3Index uncompactableWithZero[] = {0x89283470803ffff, 0x8928347081bffff, 0,
                                   0x8928347080bffff};

SUITE(compactCells) {
    TEST(roundtrip) {
        int k = 9;
        int64_t hexCount;
        t_assertSuccess(H3_EXPORT(maxGridDiskSize)(k, &hexCount));
        int expectedCompactCount = 73;

        // Generate a set of hexagons to compact
        H3Index *sunnyvaleExpanded = calloc(hexCount, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(gridDisk)(sunnyvale, k, sunnyvaleExpanded));

        H3Index *compressed = calloc(hexCount, sizeof(H3Index));
        t_assertSuccess(
            H3_EXPORT(compactCells)(sunnyvaleExpanded, compressed, hexCount));

        int count = 0;
        for (int64_t i = 0; i < hexCount; i++) {
            if (compressed[i] != 0) {
                count++;
            }
        }
        t_assert(count == expectedCompactCount, "got expected compacted count");

        int64_t countUncompact;
        t_assertSuccess(H3_EXPORT(uncompactCellsSize)(compressed, count, 9,
                                                      &countUncompact));
        H3Index *decompressed = calloc(countUncompact, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(uncompactCells)(compressed, count,
                                                  decompressed, hexCount, 9));

        int count2 = 0;
        for (int i = 0; i < hexCount; i++) {
            if (decompressed[i] != 0) {
                count2++;
            }
        }
        t_assert(count2 == hexCount, "got expected uncompacted count");

        free(compressed);
        free(decompressed);
        free(sunnyvaleExpanded);
    }

    TEST(res0children) {
        H3Index parent;
        setH3Index(&parent, 0, 0, 0);

        int64_t arrSize;
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(parent, 1, &arrSize));

        H3Index *children = calloc(arrSize, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(cellToChildren)(parent, 1, children));

        H3Index *compressed = calloc(arrSize, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(compactCells(children, compressed, arrSize)));
        t_assert(compressed[0] == parent, "got expected parent");
        for (int idx = 1; idx < arrSize; idx++) {
            t_assert(compressed[idx] == 0, "expected only 1 cell");
        }

        free(compressed);
        free(children);
    }

    TEST(res0) {
        int hexCount = NUM_BASE_CELLS;

        H3Index *res0Hexes = calloc(hexCount, sizeof(H3Index));
        for (int i = 0; i < hexCount; i++) {
            setH3Index(&res0Hexes[i], 0, i, 0);
        }
        H3Index *compressed = calloc(hexCount, sizeof(H3Index));
        t_assertSuccess(
            H3_EXPORT(compactCells)(res0Hexes, compressed, hexCount));

        for (int i = 0; i < hexCount; i++) {
            // At resolution 0, it will be an exact copy.
            // The test is further optimizing that it will be in order (which
            // isn't guaranteed.)
            t_assert(compressed[i] == res0Hexes[i],
                     "got expected compressed result");
        }

        int64_t countUncompact;
        t_assertSuccess(H3_EXPORT(uncompactCellsSize)(compressed, hexCount, 0,
                                                      &countUncompact));
        H3Index *decompressed = calloc(countUncompact, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(uncompactCells)(compressed, hexCount,
                                                  decompressed, hexCount, 0));

        int count2 = 0;
        for (int i = 0; i < hexCount; i++) {
            if (decompressed[i] != 0) {
                count2++;
            }
        }
        t_assert(count2 == hexCount, "got expected uncompacted count");

        free(res0Hexes);
        free(compressed);
        free(decompressed);
    }

    TEST(uncompactable) {
        int hexCount = 3;
        int expectedCompactCount = 3;

        H3Index *compressed = calloc(hexCount, sizeof(H3Index));
        t_assertSuccess(
            H3_EXPORT(compactCells)(uncompactable, compressed, hexCount));

        int count = 0;
        for (int i = 0; i < hexCount; i++) {
            if (compressed[i] != 0) {
                count++;
            }
        }
        t_assert(count == expectedCompactCount, "got expected compacted count");

        int64_t countUncompact;
        t_assertSuccess(H3_EXPORT(uncompactCellsSize)(compressed, count, 9,
                                                      &countUncompact));
        H3Index *decompressed = calloc(countUncompact, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(uncompactCells)(compressed, count,
                                                  decompressed, hexCount, 9));

        int count2 = 0;
        for (int i = 0; i < hexCount; i++) {
            if (decompressed[i] != 0) {
                count2++;
            }
        }
        t_assert(count2 == hexCount, "got expected uncompacted count");

        free(compressed);
        free(decompressed);
    }

    TEST(compactCells_duplicate) {
        int numHex = 10;
        H3Index someHexagons[10] = {0};
        for (int i = 0; i < numHex; i++) {
            setH3Index(&someHexagons[i], 5, 0, 2);
        }
        H3Index compressed[10];

        t_assert(H3_EXPORT(compactCells)(someHexagons, compressed, numHex) ==
                     E_DUPLICATE_INPUT,
                 "compactCells fails on duplicate input");
    }

    TEST(compactCells_duplicateMinimum) {
        // Test that the minimum number of duplicate hexagons causes failure
        H3Index h3;
        int res = 10;
        // Arbitrary index
        setH3Index(&h3, res, 0, 2);

        int64_t arrSize;
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h3, res + 1, &arrSize));
        arrSize++;
        H3Index *children = calloc(arrSize, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(cellToChildren)(h3, res + 1, children));
        // duplicate one index
        children[arrSize - 1] = children[0];

        H3Index *output = calloc(arrSize, sizeof(H3Index));

        H3Error compactCellsResult =
            H3_EXPORT(compactCells)(children, output, arrSize);
        t_assert(compactCellsResult == E_DUPLICATE_INPUT,
                 "compactCells fails on duplicate input (single duplicate)");

        free(output);
        free(children);
    }

    TEST(compactCells_duplicatePentagonLimit) {
        // Test that the minimum number of duplicate hexagons causes failure
        H3Index h3;
        int res = 10;
        // Arbitrary pentagon parent cell
        setH3Index(&h3, res, 4, 0);

        int64_t arrSize;
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h3, res + 1, &arrSize));
        arrSize++;
        H3Index *children = calloc(arrSize, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(cellToChildren)(h3, res + 1, children));
        // duplicate one index
        t_assertSuccess(
            H3_EXPORT(cellToCenterChild)(h3, res + 1, &children[arrSize - 1]));

        H3Index *output = calloc(arrSize, sizeof(H3Index));

        H3Error compactCellsResult =
            H3_EXPORT(compactCells)(children, output, arrSize);
        t_assert(compactCellsResult == E_DUPLICATE_INPUT,
                 "compactCells fails on duplicate input (pentagon parent)");

        free(output);
        free(children);
    }

    TEST(compactCells_duplicateIgnored) {
        // Test that duplicated cells are not rejected by compactCells.
        // This is not necessarily desired - just asserting the
        // existing behavior.
        H3Index h3;
        int res = 10;
        // Arbitrary index
        setH3Index(&h3, res, 0, 2);

        int64_t arrSize;
        t_assertSuccess(H3_EXPORT(cellToChildrenSize)(h3, res + 1, &arrSize));
        H3Index *children = calloc(arrSize, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(cellToChildren)(h3, res + 1, children));
        // duplicate one index
        children[arrSize - 1] = children[0];

        H3Index *output = calloc(arrSize, sizeof(H3Index));

        t_assertSuccess(H3_EXPORT(compactCells)(children, output, arrSize));

        free(output);
        free(children);
    }

    TEST(compactCells_empty) {
        t_assert(H3_EXPORT(compactCells)(NULL, NULL, 0) == E_SUCCESS,
                 "compactCells succeeds on empty input");
    }

    TEST(compactCells_disparate) {
        // Exercises a case where compaction needs to be tested but none is
        // possible
        const int numHex = 7;
        H3Index disparate[] = {0, 0, 0, 0, 0, 0, 0};
        for (int i = 0; i < numHex; i++) {
            setH3Index(&disparate[i], 1, i, CENTER_DIGIT);
        }
        H3Index output[] = {0, 0, 0, 0, 0, 0, 0};

        t_assert(
            H3_EXPORT(compactCells)(disparate, output, numHex) == E_SUCCESS,
            "compactCells succeeds on disparate input");

        // Assumes that `output` is an exact copy of `disparate`, including
        // the ordering (which may not necessarily be the case)
        for (int i = 0; i < numHex; i++) {
            t_assert(disparate[i] == output[i], "output set equals input set");
        }
    }

    TEST(compactCells_reservedBitsSet) {
        const int numHex = 7;
        H3Index bad[] = {
            0x0010000000010000, 0x0180c6c6c6c61616, 0x1616ffffffffffff,
            0xffff8affffffffff, 0xffffffffffffc6c6, 0xffffffffffffffc6,
            0xc6c6c6c6c66fffe0,
        };
        H3Index output[] = {0, 0, 0, 0, 0, 0, 0};

        t_assert(H3_EXPORT(compactCells)(bad, output, numHex) == E_CELL_INVALID,
                 "compactCells returns E_CELL_INVALID on bad input");
    }

    TEST(compactCells_parentError) {
        const int numHex = 3;
        H3Index bad[] = {0, 0, 0};
        H3Index output[] = {0, 0, 0};
        H3_SET_RESOLUTION(bad[0], 10);
        H3_SET_RESOLUTION(bad[1], 5);

        t_assert(
            H3_EXPORT(compactCells)(bad, output, numHex) == E_RES_MISMATCH,
            "compactCells returns E_RES_MISMATCH on bad input (parent error)");
    }

    TEST(compactCells_parentError2) {
        // This test primarily ensures memory is not leaked upon invalid input,
        // and ensures coverage of a very particular error branch in
        // compactCells. The particular error code is not important.
        const int numHex = 43;
        H3Index bad[] = {0x2010202020202020,
                         0x2100000000,
                         0x7,
                         0x400000000,
                         0x20000000,
                         0x5000000000,
                         0x100321,
                         0x2100000000,
                         0x7,
                         0x400000000,
                         0x20000000,
                         0x2100000000,
                         0x7,
                         0x400000000,
                         0x20000000,
                         0x5000000000,
                         0x100321,
                         0x20000000,
                         0x5000000000,
                         0x100321,
                         0x2100000000,
                         0x7,
                         0x400000000,
                         0x5000000000,
                         0x100321,
                         0x2100000000,
                         0x7,
                         0x400000000,
                         0x20000000,
                         0x5000000000,
                         0x100321,
                         0x2100000000,
                         0x7,
                         0x400000000,
                         0x20000000,
                         0x5000000000,
                         0x100321,
                         0x20000000,
                         0x5000000000,
                         0x100321,
                         0x2100000000,
                         0x7,
                         0x400000000};
        H3Index output[43] = {0};
        t_assert(H3_EXPORT(compactCells)(bad, output, numHex) == E_RES_DOMAIN,
                 "compactCells returns E_RES_DOMAIN on bad input (parent "
                 "error #2)");
    }

    TEST(uncompactCells_wrongRes) {
        int numHex = 3;
        H3Index someHexagons[] = {0, 0, 0};
        for (int i = 0; i < numHex; i++) {
            setH3Index(&someHexagons[i], 5, i, 0);
        }

        int64_t sizeResult;
        t_assert(H3_EXPORT(uncompactCellsSize)(someHexagons, numHex, 4,
                                               &sizeResult) == E_RES_MISMATCH,
                 "uncompactCellsSize fails when given illogical resolutions");
        t_assert(H3_EXPORT(uncompactCellsSize)(someHexagons, numHex, -1,
                                               &sizeResult) == E_RES_MISMATCH,
                 "uncompactCellsSize fails when given illegal resolutions");
        t_assert(
            H3_EXPORT(uncompactCellsSize)(someHexagons, numHex, MAX_H3_RES + 1,
                                          &sizeResult) == E_RES_MISMATCH,
            "uncompactCellsSize fails when given resolutions beyond max");

        H3Index uncompressed[] = {0, 0, 0};
        H3Error uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex, 0);
        t_assert(uncompactCellsResult == E_RES_MISMATCH,
                 "uncompactCells fails when given illogical resolutions");
        uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex, 6);
        t_assert(uncompactCellsResult == E_MEMORY_BOUNDS,
                 "uncompactCells fails when given too little buffer");
        uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex - 1, 5);
        t_assert(uncompactCellsResult == E_MEMORY_BOUNDS,
                 "uncompactCells fails when given too little buffer (same "
                 "resolution)");

        for (int i = 0; i < numHex; i++) {
            setH3Index(&someHexagons[i], MAX_H3_RES, i, 0);
        }
        uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex * 7, MAX_H3_RES + 1);
        t_assert(uncompactCellsResult == E_RES_MISMATCH,
                 "uncompactCells fails when given resolutions beyond max");
    }

    TEST(someHexagon) {
        H3Index origin;
        setH3Index(&origin, 1, 5, 0);

        int64_t childrenSz;
        t_assertSuccess(
            H3_EXPORT(uncompactCellsSize)(&origin, 1, 2, &childrenSz));
        H3Index *children = calloc(childrenSz, sizeof(H3Index));
        t_assertSuccess(
            H3_EXPORT(uncompactCells)(&origin, 1, children, childrenSz, 2));

        H3Index *result = calloc(childrenSz, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(compactCells)(children, result, childrenSz));

        int found = 0;
        for (int i = 0; i < childrenSz; i++) {
            if (result[i] != 0) {
                found++;
                t_assert(result[i] == origin, "compacted to correct origin");
            }
        }
        t_assert(found == 1, "compacted to a single hexagon");

        free(children);
        free(result);
    }

    TEST(uncompactCells_empty) {
        int64_t uncompactSz;
        t_assertSuccess(
            H3_EXPORT(uncompactCellsSize)(NULL, 0, 0, &uncompactSz));
        t_assert(uncompactSz == 0, "uncompactCellsSize accepts empty input");
        t_assert(H3_EXPORT(uncompactCells)(NULL, 0, NULL, 0, 0) == E_SUCCESS,
                 "uncompactCells accepts empty input");
    }

    TEST(uncompactCells_onlyZero) {
        // uncompactCellsSize and uncompactCells both permit 0 indexes
        // in the input array, and skip them. When only a zero is
        // given, it's a no-op.

        H3Index origin = 0;

        int64_t childrenSz;
        t_assertSuccess(
            H3_EXPORT(uncompactCellsSize)(&origin, 1, 2, &childrenSz));
        H3Index *children = calloc(childrenSz, sizeof(H3Index));
        t_assertSuccess(
            H3_EXPORT(uncompactCells)(&origin, 1, children, childrenSz, 2));

        free(children);
    }

    TEST(uncompactCells_withZero) {
        // uncompactCellsSize and uncompactSize both permit 0 indexes
        // in the input array, and skip them.

        int64_t childrenSz;
        t_assertSuccess(H3_EXPORT(uncompactCellsSize)(uncompactableWithZero, 4,
                                                      10, &childrenSz));
        H3Index *children = calloc(childrenSz, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(uncompactCells)(uncompactableWithZero, 4,
                                                  children, childrenSz, 10));

        int found = 0;
        for (int i = 0; i < childrenSz; i++) {
            if (children[i] != 0) {
                found++;
            }
        }
        t_assert(found == childrenSz,
                 "uncompacted with zero to expected number of hexagons");

        free(children);
    }

    TEST(pentagon) {
        H3Index pentagon;
        setH3Index(&pentagon, 1, 4, 0);

        int64_t childrenSz;
        t_assertSuccess(
            H3_EXPORT(uncompactCellsSize)(&pentagon, 1, 2, &childrenSz));
        H3Index *children = calloc(childrenSz, sizeof(H3Index));
        t_assertSuccess(
            H3_EXPORT(uncompactCells)(&pentagon, 1, children, childrenSz, 2));

        H3Index *result = calloc(childrenSz, sizeof(H3Index));
        t_assertSuccess(H3_EXPORT(compactCells)(children, result, childrenSz));

        int found = 0;
        for (int i = 0; i < childrenSz; i++) {
            if (result[i] != 0) {
                found++;
                t_assert(result[i] == pentagon,
                         "compacted to correct pentagon");
            }
        }
        t_assert(found == 1, "compacted to a single pentagon");

        free(children);
        free(result);
    }

    TEST(large_uncompact_size_hexagon) {
        H3Index cells[] = {0x806dfffffffffff};  // res 0 *hexagon*
        int res = 15;

        int64_t expected = 4747561509943L;  // 7^15
        int64_t out;
        t_assertSuccess(H3_EXPORT(uncompactCellsSize)(cells, 1, res, &out));

        t_assert(out == expected, "uncompactCells size needs 64 bit int");
    }

    TEST(large_uncompact_size_pentagon) {
        H3Index cells[] = {0x8009fffffffffff};  // res 0 *pentagon*
        int res = 15;

        int64_t expected = 3956301258286L;  // 1 + 5*(7^15 - 1)/6
        int64_t out;
        t_assertSuccess(H3_EXPORT(uncompactCellsSize)(cells, 1, res, &out));

        t_assert(out == expected, "uncompactCells size needs 64 bit int");
    }
}
