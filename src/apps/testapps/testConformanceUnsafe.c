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
 * @brief Conformance runner for the `unsafe` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceUnsafe <record-file>`
 *
 *  Each line holds one valid cell and, for k 1 to 3, whether
 *  `gridDiskUnsafe` and `gridRingUnsafe` succeed or which error they
 *  return. When an unsafe traversal succeeds it must return the safe
 *  traversal's cells; `gridDiskDistancesUnsafe` must agree with
 *  `gridDiskUnsafe`.
 */

#include <string.h>

#include "conformance.h"
#include "h3api.h"

#define MAX_K 3
#define DISK_SIZE (1 + 3 * MAX_K * (MAX_K + 1))

/** Compares an unsafe outcome with the record: on a recorded error the call
 * must fail with it; on a recorded `true` it must succeed and `got` must
 * equal `safe` as a set. */
static void checkOutcome(const cJSON *item, H3Error err, H3Index *got,
                         H3Index *safe, const char *what) {
    if (expectResult(item, err, what)) {
        return;
    }
    c_assert(cJSON_IsTrue(item), "outcome is neither true nor an error");
    int64_t gotCount = normalizeIndexSet(got, DISK_SIZE);
    int64_t safeCount = normalizeIndexSet(safe, DISK_SIZE);
    c_assert(gotCount == safeCount &&
                 !memcmp(got, safe, (size_t)gotCount * sizeof *got),
             "unsafe traversal does not return the safe traversal's cells");
}

static void checkUnsafe(const cJSON *record) {
    H3Index h = jsonIndex(record, "index");
    const cJSON *disks = jsonArray(record, "diskUnsafe");
    const cJSON *rings = jsonArray(record, "ringUnsafe");
    c_assert(cJSON_GetArraySize(disks) == MAX_K, "diskUnsafe: expected 3");
    c_assert(cJSON_GetArraySize(rings) == MAX_K, "ringUnsafe: expected 3");

    for (int k = 1; k <= MAX_K; k++) {
        const cJSON *diskItem = cJSON_GetArrayItem(disks, k - 1);
        const cJSON *ringItem = cJSON_GetArrayItem(rings, k - 1);

        H3Index safeDisk[DISK_SIZE] = {0};
        c_assert(!H3_EXPORT(gridDisk)(h, k, safeDisk), "gridDisk");
        H3Index safeRing[DISK_SIZE] = {0};
        c_assert(!H3_EXPORT(gridRing)(h, k, safeRing), "gridRing");

        H3Index disk[DISK_SIZE] = {0};
        H3Error err = H3_EXPORT(gridDiskUnsafe)(h, k, disk);
        H3Index diskCopy[DISK_SIZE];
        memcpy(diskCopy, disk, sizeof diskCopy);
        H3Index safeCopy[DISK_SIZE];
        memcpy(safeCopy, safeDisk, sizeof safeCopy);
        checkOutcome(diskItem, err, diskCopy, safeCopy, "diskUnsafe");

        H3Index withDistances[DISK_SIZE] = {0};
        int distances[DISK_SIZE] = {0};
        err =
            H3_EXPORT(gridDiskDistancesUnsafe)(h, k, withDistances, distances);
        memcpy(safeCopy, disk, sizeof safeCopy);
        checkOutcome(diskItem, err, withDistances, safeCopy,
                     "gridDiskDistancesUnsafe");

        H3Index ring[DISK_SIZE] = {0};
        err = H3_EXPORT(gridRingUnsafe)(h, k, ring);
        checkOutcome(ringItem, err, ring, safeRing, "ringUnsafe");
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkUnsafe);
}
