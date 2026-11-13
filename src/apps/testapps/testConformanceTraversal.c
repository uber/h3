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
 * @brief Conformance runner for the `traversal` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceTraversal <record-file>`
 *
 *  Each line holds one valid cell with its rings 1 to 3 and a target cell
 *  with the distance, path and neighbour relation to it. The k-disk is the
 *  cell and the rings 1 to k, so `gridDisk` and `gridDiskDistances` are
 *  checked against sets built from the rings.
 */

#include <stdlib.h>
#include <string.h>

#include "conformance.h"
#include "h3api.h"

#define MAX_K 3
#define DISK_SIZE (1 + 3 * MAX_K * (MAX_K + 1))

static void checkTraversal(const cJSON *record) {
    H3Index h = jsonIndex(record, "index");
    H3Index target = jsonIndex(record, "target");

    // The expected k-disk, accumulated from the rings as a set.
    H3Index disk[DISK_SIZE];
    int64_t diskCount = 0;
    disk[diskCount++] = h;

    for (int k = 1; k <= MAX_K; k++) {
        char key[8];
        snprintf(key, sizeof key, "ring%d", k);
        const cJSON *ringItem = jsonArray(record, key);

        H3Index ring[DISK_SIZE] = {0};
        c_assert(!H3_EXPORT(gridRing)(h, k, ring), "gridRing");
        assertIndexSet(ringItem, ring, DISK_SIZE, key);

        int64_t ringCount;
        H3Index *ringWant = parseIndexArray(ringItem, &ringCount);
        c_assert(diskCount + ringCount <= DISK_SIZE, "ring too large");
        memcpy(disk + diskCount, ringWant, (size_t)ringCount * sizeof *disk);
        diskCount += ringCount;
        free(ringWant);

        H3Index got[DISK_SIZE] = {0};
        c_assert(!H3_EXPORT(gridDisk)(h, k, got), "gridDisk");
        int64_t gotCount = normalizeIndexSet(got, DISK_SIZE);
        H3Index want[DISK_SIZE];
        memcpy(want, disk, (size_t)diskCount * sizeof *want);
        int64_t wantCount = normalizeIndexSet(want, diskCount);
        c_assert(gotCount == wantCount &&
                     !memcmp(got, want, (size_t)gotCount * sizeof *got),
                 "gridDisk: not the cell plus rings 1 to k");

        // gridDiskDistances grouped by distance is the cell, then each ring.
        H3Index cells[DISK_SIZE] = {0};
        int distances[DISK_SIZE] = {0};
        c_assert(!H3_EXPORT(gridDiskDistances)(h, k, cells, distances),
                 "gridDiskDistances");
        for (int d = 0; d <= k; d++) {
            H3Index atDistance[DISK_SIZE];
            int64_t n = 0;
            for (int i = 0; i < DISK_SIZE; i++) {
                if (cells[i] != 0 && distances[i] == d) {
                    atDistance[n++] = cells[i];
                }
            }
            if (d == 0) {
                c_assert(n == 1 && atDistance[0] == h,
                         "gridDiskDistances: distance 0 is not the cell");
                continue;
            }
            char ringKey[8];
            snprintf(ringKey, sizeof ringKey, "ring%d", d);
            assertIndexSet(jsonArray(record, ringKey), atDistance, n,
                           "gridDiskDistances");
        }
    }

    const cJSON *distanceItem = jsonItem(record, "distance");
    int64_t distance;
    H3Error err = H3_EXPORT(gridDistance)(h, target, &distance);
    if (!expectResult(distanceItem, err, "distance")) {
        c_assert(cJSON_IsNumber(distanceItem), "distance: not a number");
        c_assert(distance == (int64_t)distanceItem->valuedouble, "distance");
    }

    const cJSON *pathItem = jsonItem(record, "path");
    int64_t pathSize = 0;
    err = H3_EXPORT(gridPathCellsSize)(h, target, &pathSize);
    H3Index *path = NULL;
    if (!err) {
        path = calloc((size_t)pathSize, sizeof *path);
        c_assert(path, "out of memory");
        err = H3_EXPORT(gridPathCells)(h, target, path);
    }
    if (!expectResult(pathItem, err, "path")) {
        assertIndexSequence(pathItem, path, pathSize, "path");
    }
    free(path);

    const cJSON *neighborItem = jsonItem(record, "neighbor");
    int neighbor;
    err = H3_EXPORT(areNeighborCells)(h, target, &neighbor);
    if (!expectResult(neighborItem, err, "neighbor")) {
        c_assert(cJSON_IsBool(neighborItem), "neighbor: not a boolean");
        c_assert(!!neighbor == cJSON_IsTrue(neighborItem), "neighbor");
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkTraversal);
}
