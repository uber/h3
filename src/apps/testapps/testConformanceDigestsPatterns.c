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
 * @brief Conformance runner for `digests/patterns.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceDigestsPatterns <record-file>`
 *
 *  Each line holds a pattern count and the SHA-256 of the validity stream
 *  over that many 64-bit patterns drawn from the suite's seed. Rows above
 *  `H3_CONFORMANCE_PATTERNS_MAXCOUNT` (default 1000000) are skipped; the
 *  ten-million row takes about half a minute.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "conformance.h"
#include "conformanceGen.h"
#include "h3api.h"
#include "sha256.h"

#define DEFAULT_MAX_COUNT 1000000

static int64_t maxCount;
static long skipped = 0;

static void checkPatterns(const cJSON *record) {
    int64_t count = jsonInt64(record, "count");
    if (count > maxCount) {
        skipped++;
        return;
    }
    const cJSON *digests = jsonItem(record, "digests");
    c_assert(cJSON_IsObject(digests) && cJSON_GetArraySize(digests) == 1,
             "digests: expected the validity stream only");
    const char *want = jsonString(digests, "validity");

    Splitmix64 rng = {CONFORMANCE_SEED};
    Sha256 digest;
    sha256Init(&digest);
    Line line = {0};
    for (int64_t n = 0; n < count; n++) {
        uint64_t word = drawPattern(&rng);
        char buf[32];
        snprintf(buf, sizeof buf, "%" PRIx64, word);
        line.len = 0;
        lineAppend(&line, buf);
        lineInt(&line, H3_EXPORT(isValidCell)(word));
        lineInt(&line, H3_EXPORT(isValidIndex)(word));
        lineInt(&line, H3_EXPORT(isValidDirectedEdge)(word));
        lineInt(&line, H3_EXPORT(isValidVertex)(word));
        lineAppend(&line, "\n");
        sha256Update(&digest, (const uint8_t *)line.buf, line.len);
    }
    free(line.buf);
    char hex[65];
    sha256Final(&digest, hex);
    if (strcmp(hex, want) != 0) {
        conformanceFail("digests.validity: got %s, want %s", hex, want);
    }
}

int main(int argc, char *argv[]) {
    maxCount = conformanceEnvInt("H3_CONFORMANCE_PATTERNS_MAXCOUNT",
                                 DEFAULT_MAX_COUNT);
    int status = conformanceMain(argc, argv, checkPatterns);
    if (skipped) {
        printf("%ld rows above %lld patterns skipped\n", skipped,
               (long long)maxCount);
    }
    return status;
}
