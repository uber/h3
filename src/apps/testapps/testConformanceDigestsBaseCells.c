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
 * @brief Conformance runner for `digests/baseCells.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceDigestsBaseCells <record-file>`
 *
 *  Each line holds one resolution and base cell with the SHA-256 of every
 *  digest stream over the cells of that resolution descending from that
 *  base cell. The runner recomputes the streams from the library. Rows
 *  above `H3_CONFORMANCE_DIGESTS_MAXRES` (default 3) are skipped;
 *  resolution 5 takes about a minute and resolution 7 about an hour.
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

static void checkBaseCell(const cJSON *record) {
    int res = jsonInt(record, "res");
    int baseCell = jsonInt(record, "baseCell");
    if (res > maxRes) {
        skipped++;
        return;
    }
    c_assert(baseCell >= 0 && baseCell < NUM_BASE_CELLS, "baseCell");
    Line line = {0};
    char hex[NUM_STREAMS][65];
    int64_t count = digestBaseCell(res, baseCell, &line, hex);
    free(line.buf);
    assertDigests(record, res, count, hex);
}

int main(int argc, char *argv[]) {
    maxRes =
        conformanceEnvInt("H3_CONFORMANCE_DIGESTS_MAXRES", DEFAULT_MAX_RES);
    int status = conformanceMain(argc, argv, checkBaseCell);
    if (skipped) {
        printf("%ld rows above resolution %lld skipped\n", skipped,
               (long long)maxRes);
    }
    return status;
}
