/*
 * Copyright 2026 Uber Technologies, Inc.
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
 * @brief tests H3 inspection functions
 *
 *  usage: `testInspection <test-file>`
 *
 *  The program reads lines containing H3 indexes and lat/lng center
 *  point pairs from stdin until EOF is encountered. For each input line,
 *  the program calls `cellToLatLng` to convert H3 index to a lat/lng, then
 *  validates against the input lat/lng within a given threshold
 */

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "test.h"
#include "utility.h"

void assertExpected(H3Index h, int res, int baseCell, int validCell,
                    int validIndex, int resClassIII, int pentagon,
                    int faceCount, int *faces, int *digits) {
    t_assert(H3_EXPORT(getResolution)(h) == res, "expected res");
    t_assert(H3_EXPORT(getBaseCellNumber)(h) == baseCell, "expected base cell");
    t_assert(H3_EXPORT(isValidCell)(h) == validCell, "expected is valid cell");
    t_assert(H3_EXPORT(isValidIndex)(h) == validIndex,
             "expected is valid index");
    t_assert(H3_EXPORT(isResClassIII)(h) == resClassIII,
             "expected is res class iii");
    t_assert(H3_EXPORT(isPentagon)(h) == pentagon, "expected is pentagon");

    int actualMaxFaceCount;
    H3Error faceCountError = H3_EXPORT(maxFaceCount)(h, &actualMaxFaceCount);
    if (faceCountError) {
        t_assert(faceCountError == -faceCount, "error for face count matches");
    } else if (faceCount >= 0) {
        int *actualFaces = calloc(actualMaxFaceCount, sizeof(int));
        t_assertSuccess(H3_EXPORT(getIcosahedronFaces)(h, actualFaces));

        int counted = 0;
        for (int i = 0; i < actualMaxFaceCount; i++) {
            if (actualFaces[i] == -1) continue;

            bool found = false;
            for (int j = 0; j < faceCount; j++) {
                if (actualFaces[i] == faces[j]) {
                    found = true;
                    break;
                }
            }

            t_assert(found, "face found in expected list");
            counted++;
        }

        t_assert(counted == faceCount, "expected face count to be equal");

        free(actualFaces);
    }

    for (int i = 1; i <= res; i++) {
        int digit;
        t_assertSuccess(H3_EXPORT(getIndexDigit)(h, i, &digit));
        t_assert(digit == digits[i - 1], "expected digit to be equal");
    }

    if (validCell) {
        H3Index reconstructed;
        t_assertSuccess(
            H3_EXPORT(constructCell)(res, baseCell, digits, &reconstructed));
        t_assert(h == reconstructed, "expected constructCell to round trip");
    }
}

int main(int argc, char *argv[]) {
    // check command line args
    if (argc != 2) {
        fprintf(stderr, "usage: %s <test-file>\n", argv[0]);
        exit(1);
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        error("could not open test file");
    }

    char h3Str[BUFF_SIZE];
    while (1) {
        int res, baseCell, validCell, validIndex, resClassIII, pentagon,
            icosaFaceCount;
        if (fscanf(f, "%s %d %d %d %d %d %d %d\n", h3Str, &res, &baseCell,
                   &validCell, &validIndex, &resClassIII, &pentagon,
                   &icosaFaceCount) != 8) {
            if (feof(f)) {
                break;
            }
            error(
                "parsing input (should be \"H3Index res baseCell validCell "
                "validIndex resClassIII pentagon icosaFaceCount\")");
        }

        int *faces = calloc(icosaFaceCount, sizeof(int));
        for (int i = 0; i < icosaFaceCount; i++) {
            if (i != 0) fgetc(f);
            if (fscanf(f, "%d", &faces[i]) != 1) {
                error("parsing input (failed to read expected icosa face)");
            }
        }

        int *digits = calloc(res, sizeof(int));
        for (int i = 0; i < res; i++) {
            if (i != 0) fgetc(f);
            if (fscanf(f, "%d", &digits[i]) != 1) {
                error("parsing input (failed to read expected digits)");
            }
        }

        H3Index h3;
        t_assertSuccess(H3_EXPORT(stringToH3)(h3Str, &h3));

        assertExpected(h3, res, baseCell, validCell, validIndex, resClassIII,
                       pentagon, icosaFaceCount, faces, digits);

        free(faces);
        free(digits);
    }

    fclose(f);
}
