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
 * @brief Conformance runner for the `sets` group (dev-docs/conformance.md).
 *
 *  usage: `testConformanceSets <record-file>`
 *
 *  Each line holds one input set, which may contain duplicates and invalid
 *  cells, with `compactCells` of it and `uncompactCells` of it at a given
 *  resolution. When every input is valid and compaction succeeded,
 *  uncompacting the result at the input's finest resolution must give the
 *  input back as a set.
 */

#include <stdlib.h>
#include <string.h>

#include "conformance.h"
#include "h3api.h"

/** Uncompacts `cells` to `res`, returning the error or filling `out` and
 * `count` with a malloc'd result. */
static H3Error uncompact(const H3Index *cells, int64_t numCells, int res,
                         H3Index **out, int64_t *count) {
    *out = NULL;
    *count = 0;
    H3Error err = H3_EXPORT(uncompactCellsSize)(cells, numCells, res, count);
    if (err) {
        return err;
    }
    *out = calloc(*count ? (size_t)*count : 1, sizeof **out);
    c_assert(*out, "out of memory");
    err = H3_EXPORT(uncompactCells)(cells, numCells, *out, *count, res);
    if (err) {
        free(*out);
        *out = NULL;
    }
    return err;
}

/** Checks that two buffers hold the same set of indexes; both are
 * reordered. */
static void assertSameSet(H3Index *got, int64_t gotCount, H3Index *want,
                          int64_t wantCount, const char *what) {
    gotCount = normalizeIndexSet(got, gotCount);
    wantCount = normalizeIndexSet(want, wantCount);
    c_assert(gotCount == wantCount &&
                 !memcmp(got, want, (size_t)gotCount * sizeof *got),
             what);
}

static void checkSets(const cJSON *record) {
    int64_t numInput;
    H3Index *input = parseIndexArray(jsonArray(record, "input"), &numInput);
    bool allValid = true;
    int finest = 0;
    for (int64_t i = 0; i < numInput; i++) {
        allValid = allValid && H3_EXPORT(isValidCell)(input[i]);
        int res = H3_EXPORT(getResolution)(input[i]);
        finest = res > finest ? res : finest;
    }

    const cJSON *compactItem = jsonItem(record, "compact");
    H3Index *compacted =
        calloc(numInput ? (size_t)numInput : 1, sizeof *compacted);
    c_assert(compacted, "out of memory");
    H3Error compactErr = H3_EXPORT(compactCells)(input, compacted, numInput);
    if (!expectResult(compactItem, compactErr, "compact")) {
        H3Index *copy = malloc((size_t)numInput * sizeof *copy + 1);
        c_assert(copy, "out of memory");
        memcpy(copy, compacted, (size_t)numInput * sizeof *copy);
        assertIndexSet(compactItem, copy, numInput, "compact");
        free(copy);
    }

    int uncompactRes = jsonInt(record, "uncompactRes");
    const cJSON *uncompactItem = jsonItem(record, "uncompact");
    H3Index *uncompacted;
    int64_t numUncompacted;
    H3Error err =
        uncompact(input, numInput, uncompactRes, &uncompacted, &numUncompacted);
    if (!expectResult(uncompactItem, err, "uncompact")) {
        if (cJSON_IsTrue(uncompactItem)) {
            // true means the input as a set.
            H3Index *want = malloc((size_t)numInput * sizeof *want + 1);
            c_assert(want, "out of memory");
            memcpy(want, input, (size_t)numInput * sizeof *want);
            assertSameSet(uncompacted, numUncompacted, want, numInput,
                          "uncompact: not the input as a set");
            free(want);
        } else {
            assertIndexSet(uncompactItem, uncompacted, numUncompacted,
                           "uncompact");
        }
    }
    free(uncompacted);

    if (!compactErr && allValid) {
        H3Index *roundTrip;
        int64_t numRoundTrip;
        c_assert(
            !uncompact(compacted, numInput, finest, &roundTrip, &numRoundTrip),
            "uncompactCells of the compacted set");
        assertSameSet(roundTrip, numRoundTrip, input, numInput,
                      "uncompactCells(compact) does not round trip");
        free(roundTrip);
    }
    free(compacted);
    free(input);
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkSets);
}
