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

#include "cJSON.h"
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

    char jsonStr[BUFF_SIZE];
    while (1) {
        if (!fgets(jsonStr, BUFF_SIZE, f)) {
            if (feof(f)) {
                break;
            }
            error("reading input");
        }

        cJSON *json = cJSON_Parse(jsonStr);
        if (json == NULL) {
            const char *errorStr = cJSON_GetErrorPtr();
            if (errorStr != NULL) {
                fprintf(stderr, "Error before: %s\n", errorStr);
            }
            exit(1);
        }

        const char *h3Str;
        int res, baseCell, validCell, validIndex, resClassIII, pentagon,
            icosaFaceCount;
        int *faces = NULL;

        cJSON *h3StrJson = cJSON_GetObjectItemCaseSensitive(json, "index");
        if (!cJSON_IsString(h3StrJson) || h3StrJson->valuestring == NULL) {
            error("getting name");
        }
        h3Str = h3StrJson->valuestring;
        cJSON *resJson = cJSON_GetObjectItemCaseSensitive(json, "res");
        if (!cJSON_IsNumber(resJson)) {
            error("getting res");
        }
        res = resJson->valueint;
        cJSON *baseCellJson =
            cJSON_GetObjectItemCaseSensitive(json, "baseCell");
        if (!cJSON_IsNumber(baseCellJson)) {
            error("getting baseCell");
        }
        baseCell = baseCellJson->valueint;
        cJSON *validCellJson =
            cJSON_GetObjectItemCaseSensitive(json, "validCell");
        if (!cJSON_IsBool(validCellJson)) {
            error("getting validCell");
        }
        validCell = cJSON_IsTrue(validCellJson);
        cJSON *validIndexJson =
            cJSON_GetObjectItemCaseSensitive(json, "validIndex");
        if (!cJSON_IsBool(validIndexJson)) {
            error("getting validIndex");
        }
        validIndex = cJSON_IsTrue(validIndexJson);
        cJSON *resClassIIIJson =
            cJSON_GetObjectItemCaseSensitive(json, "resClassIII");
        if (!cJSON_IsBool(resClassIIIJson)) {
            error("getting resClassIII");
        }
        resClassIII = cJSON_IsTrue(resClassIIIJson);
        cJSON *pentagonJson =
            cJSON_GetObjectItemCaseSensitive(json, "pentagon");
        if (!cJSON_IsBool(pentagonJson)) {
            error("getting pentagon");
        }
        pentagon = cJSON_IsTrue(pentagonJson);

        if (cJSON_HasObjectItem(json, "faceError")) {
            cJSON *faceErrorJson =
                cJSON_GetObjectItemCaseSensitive(json, "faceError");
            if (!cJSON_IsNumber(faceErrorJson)) {
                error("getting faceError");
            }
            icosaFaceCount = faceErrorJson->valueint;
        } else {
            cJSON *facesJson = cJSON_GetObjectItemCaseSensitive(json, "faces");
            if (!cJSON_IsArray(facesJson)) {
                error("getting faces");
            }
            icosaFaceCount = cJSON_GetArraySize(facesJson);
            faces = calloc(icosaFaceCount, sizeof(int));
            int faceIndex = 0;
            cJSON *faceJson;
            cJSON_ArrayForEach(faceJson, facesJson) {
                if (!cJSON_IsNumber(faceJson)) {
                    error("getting face");
                }

                faces[faceIndex] = faceJson->valueint;
                faceIndex++;
            }
        }

        cJSON *digitsJson = cJSON_GetObjectItemCaseSensitive(json, "digits");
        if (!cJSON_IsArray(digitsJson)) {
            error("getting digits");
        }
        int digitsCount = cJSON_GetArraySize(digitsJson);
        int *digits = calloc(digitsCount, sizeof(int));
        int digitIndex = 0;
        cJSON *digitJson;
        cJSON_ArrayForEach(digitJson, digitsJson) {
            if (!cJSON_IsNumber(digitJson)) {
                error("getting digit");
            }

            digits[digitIndex] = digitJson->valueint;
            digitIndex++;
        }

        H3Index h3;
        t_assertSuccess(H3_EXPORT(stringToH3)(h3Str, &h3));

        assertExpected(h3, res, baseCell, validCell, validIndex, resClassIII,
                       pentagon, icosaFaceCount, faces, digits);

        free(faces);
        free(digits);

        cJSON_Delete(json);
    }

    fclose(f);
}
