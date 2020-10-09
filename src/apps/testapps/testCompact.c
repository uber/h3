/*
 * Copyright 2017-2020 Uber Technologies, Inc.
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

H3Index uncompactCellsable[] = {0x89283470803ffff, 0x8928347081bffff,
                                0x8928347080bffff};
H3Index uncompactCellsableWithZero[] = {0x89283470803ffff, 0x8928347081bffff, 0,
                                        0x8928347080bffff};

SUITE(compactCells) {
    TEST(roundtrip) {
        int k = 9;
        int hexCount = H3_EXPORT(maxGridDiskSize)(k);
        int expectedCompactCount = 73;

        // Generate a set of hexagons to compactCells
        H3Index* sunnyvaleExpanded = calloc(hexCount, sizeof(H3Index));
        H3_EXPORT(gridDisk)(sunnyvale, k, sunnyvaleExpanded);

        H3Index* compressed = calloc(hexCount, sizeof(H3Index));
        int err =
            H3_EXPORT(compactCells)(sunnyvaleExpanded, compressed, hexCount);
        t_assert(err == 0, "no error on compactCells");

        int count = 0;
        for (int i = 0; i < hexCount; i++) {
            if (compressed[i] != 0) {
                count++;
            }
        }
        t_assert(count == expectedCompactCount,
                 "got expected compactCellsed count");

        H3Index* decompressed =
            calloc(H3_EXPORT(maxUncompactCellsSize)(compressed, count, 9),
                   sizeof(H3Index));
        int err2 = H3_EXPORT(uncompactCells)(compressed, count, decompressed,
                                             hexCount, 9);
        t_assert(err2 == 0, "no error on uncompactCells");

        int count2 = 0;
        for (int i = 0; i < hexCount; i++) {
            if (decompressed[i] != 0) {
                count2++;
            }
        }
        t_assert(count2 == hexCount, "got expected uncompactCellsed count");

        free(compressed);
        free(decompressed);
        free(sunnyvaleExpanded);
    }

    TEST(res0) {
        int hexCount = NUM_BASE_CELLS;

        H3Index* res0Hexes = calloc(hexCount, sizeof(H3Index));
        for (int i = 0; i < hexCount; i++) {
            setH3Index(&res0Hexes[i], 0, i, 0);
        }
        H3Index* compressed = calloc(hexCount, sizeof(H3Index));
        int err = H3_EXPORT(compactCells)(res0Hexes, compressed, hexCount);
        t_assert(err == 0, "no error on compactCells");

        for (int i = 0; i < hexCount; i++) {
            // At resolution 0, it will be an exact copy.
            // The test is further optimizing that it will be in order (which
            // isn't guaranteed.)
            t_assert(compressed[i] == res0Hexes[i],
                     "got expected compressed result");
        }

        H3Index* decompressed =
            calloc(H3_EXPORT(maxUncompactCellsSize)(compressed, hexCount, 0),
                   sizeof(H3Index));
        int err2 = H3_EXPORT(uncompactCells)(compressed, hexCount, decompressed,
                                             hexCount, 0);
        t_assert(err2 == 0, "no error on uncompactCells");

        int count2 = 0;
        for (int i = 0; i < hexCount; i++) {
            if (decompressed[i] != 0) {
                count2++;
            }
        }
        t_assert(count2 == hexCount, "got expected uncompactCellsed count");

        free(res0Hexes);
        free(compressed);
        free(decompressed);
    }

    TEST(uncompactCellsable) {
        int hexCount = 3;
        int expectedCompactCount = 3;

        H3Index* compressed = calloc(hexCount, sizeof(H3Index));
        int err =
            H3_EXPORT(compactCells)(uncompactCellsable, compressed, hexCount);
        t_assert(err == 0, "no error on compactCells");

        int count = 0;
        for (int i = 0; i < hexCount; i++) {
            if (compressed[i] != 0) {
                count++;
            }
        }
        t_assert(count == expectedCompactCount,
                 "got expected compactCellsed count");

        H3Index* decompressed =
            calloc(H3_EXPORT(maxUncompactCellsSize)(compressed, count, 9),
                   sizeof(H3Index));
        int err2 = H3_EXPORT(uncompactCells)(compressed, count, decompressed,
                                             hexCount, 9);
        t_assert(err2 == 0, "no error on uncompactCells");

        int count2 = 0;
        for (int i = 0; i < hexCount; i++) {
            if (decompressed[i] != 0) {
                count2++;
            }
        }
        t_assert(count2 == hexCount, "got expected uncompactCellsed count");

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

        t_assert(H3_EXPORT(compactCells)(someHexagons, compressed, numHex) != 0,
                 "compactCells fails on duplicate input");
    }

    TEST(compactCells_duplicateMinimum) {
        // Test that the minimum number of duplicate hexagons causes failure
        H3Index h3;
        int res = 10;
        // Arbitrary index
        setH3Index(&h3, res, 0, 2);

        int arrSize = H3_EXPORT(maxCellToChildrenSize)(h3, res + 1) + 1;
        H3Index* children = calloc(arrSize, sizeof(H3Index));

        H3_EXPORT(cellToChildren)(h3, res + 1, children);
        // duplicate one index
        children[arrSize - 1] = children[0];

        H3Index* output = calloc(arrSize, sizeof(H3Index));

        int compactCellsResult =
            H3_EXPORT(compactCells)(children, output, arrSize);
        t_assert(compactCellsResult == COMPACT_DUPLICATE,
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

        int arrSize = H3_EXPORT(maxCellToChildrenSize)(h3, res + 1) + 1;
        H3Index* children = calloc(arrSize, sizeof(H3Index));

        H3_EXPORT(cellToChildren)(h3, res + 1, children);
        // duplicate one index
        children[arrSize - 1] = H3_EXPORT(cellToCenterChild)(h3, res + 1);

        H3Index* output = calloc(arrSize, sizeof(H3Index));

        int compactCellsResult =
            H3_EXPORT(compactCells)(children, output, arrSize);
        t_assert(compactCellsResult == COMPACT_DUPLICATE,
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

        int arrSize = H3_EXPORT(maxCellToChildrenSize)(h3, res + 1);
        H3Index* children = calloc(arrSize, sizeof(H3Index));

        H3_EXPORT(cellToChildren)(h3, res + 1, children);
        // duplicate one index
        children[arrSize - 1] = children[0];

        H3Index* output = calloc(arrSize, sizeof(H3Index));

        int compactCellsResult =
            H3_EXPORT(compactCells)(children, output, arrSize);
        t_assert(compactCellsResult == COMPACT_SUCCESS,
                 "compactCells succeeds on duplicate input (correct count)");

        free(output);
        free(children);
    }

    TEST(compactCells_empty) {
        t_assert(H3_EXPORT(compactCells)(NULL, NULL, 0) == 0,
                 "compactCells succeeds on empty input");
    }

    TEST(compactCells_disparate) {
        // Exercises a case where compactCellsion needs to be tested but none is
        // possible
        const int numHex = 7;
        H3Index disparate[] = {0, 0, 0, 0, 0, 0, 0};
        for (int i = 0; i < numHex; i++) {
            setH3Index(&disparate[i], 1, i, CENTER_DIGIT);
        }
        H3Index output[] = {0, 0, 0, 0, 0, 0, 0};

        t_assert(H3_EXPORT(compactCells)(disparate, output, numHex) == 0,
                 "compactCells succeeds on disparate input");

        // Assumes that `output` is an exact copy of `disparate`, including
        // the ordering (which may not necessarily be the case)
        for (int i = 0; i < numHex; i++) {
            t_assert(disparate[i] == output[i], "output set equals input set");
        }
    }

    TEST(uncompactCells_wrongRes) {
        int numHex = 3;
        H3Index someHexagons[] = {0, 0, 0};
        for (int i = 0; i < numHex; i++) {
            setH3Index(&someHexagons[i], 5, i, 0);
        }

        int sizeResult =
            H3_EXPORT(maxUncompactCellsSize)(someHexagons, numHex, 4);
        t_assert(
            sizeResult < 0,
            "maxUncompactCellsSize fails when given illogical resolutions");
        sizeResult = H3_EXPORT(maxUncompactCellsSize)(someHexagons, numHex, -1);
        t_assert(sizeResult < 0,
                 "maxUncompactCellsSize fails when given illegal resolutions");
        sizeResult = H3_EXPORT(maxUncompactCellsSize)(someHexagons, numHex,
                                                      MAX_H3_RES + 1);
        t_assert(
            sizeResult < 0,
            "maxUncompactCellsSize fails when given resolutions beyond max");

        H3Index uncompressed[] = {0, 0, 0};
        int uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex, 0);
        t_assert(uncompactCellsResult != 0,
                 "uncompactCells fails when given illogical resolutions");
        uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex, 6);
        t_assert(uncompactCellsResult != 0,
                 "uncompactCells fails when given too little buffer");
        uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex - 1, 5);
        t_assert(uncompactCellsResult != 0,
                 "uncompactCells fails when given too little buffer (same "
                 "resolution)");

        for (int i = 0; i < numHex; i++) {
            setH3Index(&someHexagons[i], MAX_H3_RES, i, 0);
        }
        uncompactCellsResult = H3_EXPORT(uncompactCells)(
            someHexagons, numHex, uncompressed, numHex * 7, MAX_H3_RES + 1);
        t_assert(uncompactCellsResult != 0,
                 "uncompactCells fails when given resolutions beyond max");
    }

    TEST(someHexagon) {
        H3Index origin;
        setH3Index(&origin, 1, 5, 0);

        int childrenSz = H3_EXPORT(maxUncompactCellsSize)(&origin, 1, 2);
        H3Index* children = calloc(childrenSz, sizeof(H3Index));
        int uncompactCellsResult =
            H3_EXPORT(uncompactCells)(&origin, 1, children, childrenSz, 2);
        t_assert(uncompactCellsResult == 0, "uncompactCells origin succeeds");

        H3Index* result = calloc(childrenSz, sizeof(H3Index));
        int compactCellsResult =
            H3_EXPORT(compactCells)(children, result, childrenSz);
        t_assert(compactCellsResult == 0, "compactCells origin succeeds");

        int found = 0;
        for (int i = 0; i < childrenSz; i++) {
            if (result[i] != 0) {
                found++;
                t_assert(result[i] == origin, "compacted to correct origin");
            }
        }
        t_assert(found == 1, "compactCellsed to a single hexagon");

        free(children);
        free(result);
    }

    TEST(uncompactCells_empty) {
        int uncompactCellsSz = H3_EXPORT(maxUncompactCellsSize)(NULL, 0, 0);
        t_assert(uncompactCellsSz == 0,
                 "maxUncompactCellsSize accepts empty input");
        t_assert(H3_EXPORT(uncompactCells)(NULL, 0, NULL, 0, 0) == 0,
                 "uncompactCells accepts empty input");
    }

    TEST(uncompactCells_onlyZero) {
        // maxUncompactCellsSize and uncompactCells both permit 0 indexes
        // in the input array, and skip them. When only a zero is
        // given, it's a no-op.

        H3Index origin = 0;

        int childrenSz = H3_EXPORT(maxUncompactCellsSize)(&origin, 1, 2);
        H3Index* children = calloc(childrenSz, sizeof(H3Index));
        int uncompactCellsResult =
            H3_EXPORT(uncompactCells)(&origin, 1, children, childrenSz, 2);
        t_assert(uncompactCellsResult == 0, "uncompactCells only zero success");

        free(children);
    }

    TEST(uncompactCells_withZero) {
        // maxUncompactCellsSize and uncompactCells both permit 0 indexes
        // in the input array, and skip them.

        int childrenSz =
            H3_EXPORT(maxUncompactCellsSize)(uncompactCellsableWithZero, 4, 10);
        H3Index* children = calloc(childrenSz, sizeof(H3Index));
        int uncompactCellsResult = H3_EXPORT(uncompactCells)(
            uncompactCellsableWithZero, 4, children, childrenSz, 10);
        t_assert(uncompactCellsResult == 0,
                 "uncompactCells with zero succeeds");

        int found = 0;
        for (int i = 0; i < childrenSz; i++) {
            if (children[i] != 0) {
                found++;
            }
        }
        t_assert(found == childrenSz,
                 "uncompactCellsed with zero to expected number of hexagons");

        free(children);
    }

    TEST(pentagon) {
        H3Index pentagon;
        setH3Index(&pentagon, 1, 4, 0);

        int childrenSz = H3_EXPORT(maxUncompactCellsSize)(&pentagon, 1, 2);
        H3Index* children = calloc(childrenSz, sizeof(H3Index));
        int uncompactCellsResult =
            H3_EXPORT(uncompactCells)(&pentagon, 1, children, childrenSz, 2);
        t_assert(uncompactCellsResult == 0, "uncompactCells pentagon succeeds");

        H3Index* result = calloc(childrenSz, sizeof(H3Index));
        int compactCellsResult =
            H3_EXPORT(compactCells)(children, result, childrenSz);
        t_assert(compactCellsResult == 0, "compactCells pentagon succeeds");

        int found = 0;
        for (int i = 0; i < childrenSz; i++) {
            if (result[i] != 0) {
                found++;
                t_assert(result[i] == pentagon,
                         "compactCellsed to correct pentagon");
            }
        }
        t_assert(found == 1, "compacted to a single pentagon");

        free(children);
        free(result);
    }
}
