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
 * @brief Conformance runner for `digests/resolutions.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceDigestsResolutions <record-file>`
 *
 *  Each line holds one resolution with, per digest stream, the SHA-256 of
 *  the 122 per-base-cell digests of that stream in base-cell order. The
 *  runner recomputes every base cell. Rows above
 *  `H3_CONFORMANCE_DIGESTS_MAXRES` (default 3) are skipped, except for
 *  `getNumCells` and `getPentagons`, which need no enumeration and are
 *  checked on every row.
 */

#include <stdlib.h>
#include <string.h>

#include "conformance.h"
#include "conformanceGen.h"
#include "h3api.h"

#define DEFAULT_MAX_RES 3

static int64_t maxRes;
static long skipped = 0;

/** Compares computed stream digests with a record's `digests` object. */
static void assertDigests(const cJSON *record, int res, int64_t count,
                          char hex[NUM_STREAMS][65]) {
    c_assert(count == jsonInt64(record, "count"), "count");
    const cJSON *digests = jsonItem(record, "digests");
    c_assert(cJSON_IsObject(digests), "digests: not an object");
    int expected = 0;
    for (int i = 0; i < NUM_STREAMS; i++) {
        if (i == S_POLYGONS && !hasPolygons(res)) {
            c_assert(!cJSON_HasObjectItem(digests, STREAM_KEYS[i]),
                     "digests: polygons stream above its last resolution");
            continue;
        }
        expected++;
        const char *want = jsonString(digests, STREAM_KEYS[i]);
        if (strcmp(hex[i], want) != 0) {
            conformanceFail("digests.%s: got %s, want %s", STREAM_KEYS[i],
                            hex[i], want);
        }
    }
    int actual = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, digests) { actual++; }
    c_assert(actual == expected, "digests: unexpected stream");
}

/** Checks getNumCells against the row's count, and that getPentagons returns
 * twelve valid pentagons at the row's resolution in ascending order. */
static void checkCounts(const cJSON *record, int res) {
    int64_t numCells;
    c_assert(!H3_EXPORT(getNumCells)(res, &numCells), "getNumCells");
    c_assert(numCells == jsonInt64(record, "count"), "getNumCells: not count");

    int numPentagons = H3_EXPORT(pentagonCount)();
    H3Index *pentagons = calloc((size_t)numPentagons, sizeof *pentagons);
    c_assert(pentagons, "out of memory");
    c_assert(!H3_EXPORT(getPentagons)(res, pentagons), "getPentagons");
    for (int i = 0; i < numPentagons; i++) {
        c_assert(H3_EXPORT(isValidCell)(pentagons[i]) &&
                     H3_EXPORT(isPentagon)(pentagons[i]) &&
                     H3_EXPORT(getResolution)(pentagons[i]) == res,
                 "getPentagons: not a valid pentagon at the resolution");
        c_assert(i == 0 || pentagons[i - 1] < pentagons[i],
                 "getPentagons: not in ascending order");
    }
    free(pentagons);
}

static void checkResolution(const cJSON *record) {
    int res = jsonInt(record, "res");
    checkCounts(record, res);
    if (res > maxRes) {
        skipped++;
        return;
    }
    ResolutionDigest resolution;
    resolutionInit(&resolution);
    Line line = {0};
    for (int baseCell = 0; baseCell < NUM_BASE_CELLS; baseCell++) {
        char hex[NUM_STREAMS][65];
        int64_t count = digestBaseCell(res, baseCell, &line, hex);
        resolutionAdd(&resolution, res, baseCell, count, hex);
    }
    free(line.buf);
    char hex[NUM_STREAMS][65];
    finalizeStreams(resolution.streams, hex);
    assertDigests(record, res, resolution.count, hex);
}

int main(int argc, char *argv[]) {
    maxRes =
        conformanceEnvInt("H3_CONFORMANCE_DIGESTS_MAXRES", DEFAULT_MAX_RES);
    int status = conformanceMain(argc, argv, checkResolution);
    if (skipped) {
        printf("%ld rows above resolution %lld skipped\n", skipped,
               (long long)maxRes);
    }
    return status;
}
