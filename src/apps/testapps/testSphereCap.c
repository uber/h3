/*
 * Copyright 2016-2018, 2020-2021 Uber Technologies, Inc.
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

/** @file testSphereCap.c
 * @brief Tests the internal SphereCap helpers used by geodesic algorithms
 */

#include <math.h>
#include <stdlib.h>

#include "../../h3lib/lib/sphereCapTables.h"
#include "bbox.h"
#include "constants.h"
#include "h3Index.h"
#include "latLng.h"
#include "polyfill.h"
#include "test.h"
#include "vec3d.h"

// If update of the precomputed values if needed for any reason this
// function can be used to re-record them
static void _printPrecomputedCosRadius(void) {
    printf("Printing \n");
    for (int res = 0; res <= MAX_H3_RES; res++) {
        printf("res=%2d: %a\n", res,
               cos(MAX_EDGE_LENGTH_RADS[res] * H3_SPHERE_CAP_SCALE_FACTOR));
    }
}

/**
 * Verify that PRECOMPUTED_COS_RADIUS matches cos(MAX_EDGE_LENGTH_RADS[res] *
 * H3_SPHERE_CAP_SCALE_FACTOR) for every resolution.
 */
static bool _verifyCosRadiusValues(void) {
    bool pass = true;

    for (int res = 0; res <= MAX_H3_RES; res++) {
        const double radius =
            MAX_EDGE_LENGTH_RADS[res] * H3_SPHERE_CAP_SCALE_FACTOR;
        const double expected = cos(radius);
        const double actual = PRECOMPUTED_COS_RADIUS[res];
        const double diff = fabs(expected - actual);

        if (diff > EPSILON) {
            printf(
                "Cos radius mismatch at res=%d: expected=%a actual=%a "
                "diff=%0.3e\n",
                res, expected, actual, diff);
            pass = false;
        }
    }

    if (!pass) {
        _printPrecomputedCosRadius();
    }

    return pass;
}

static bool _cellWithinCap(const H3Index cell, const SphereCap *cap) {
    CellBoundary boundary;
    if (H3_EXPORT(cellToBoundary)(cell, &boundary) != E_SUCCESS) {
        return false;
    }

    for (int i = 0; i < boundary.numVerts; i++) {
        Vec3d vertex = latLngToVec3(&boundary.verts[i]);
        double dot = vec3Dot(&vertex, &cap->center);
        if (dot + EPSILON < cap->cosRadius) {
            printf(
                "Cell %llx vertex %d outside cap (dot=%0.6f, thresh=%0.6f)\n",
                (unsigned long long)cell, i, dot, cap->cosRadius);
            return false;
        }

        Vec3d next = latLngToVec3(&boundary.verts[(i + 1) % boundary.numVerts]);
        Vec3d midpoint = {vertex.x + next.x, vertex.y + next.y,
                          vertex.z + next.z};
        double mag = vec3Mag(&midpoint);
        if (mag > EPSILON) {
            midpoint.x /= mag;
            midpoint.y /= mag;
            midpoint.z /= mag;
            double midDot = vec3Dot(&midpoint, &cap->center);
            if (midDot + EPSILON < cap->cosRadius) {
                printf(
                    "Cell %llx edge %d midpoint outside cap (dot=%0.6f, "
                    "thresh=%0.6f)\n",
                    (unsigned long long)cell, i, midDot, cap->cosRadius);
                return false;
            }
        }
    }

    return true;
}

static bool _testCellRecursive(const H3Index cell, const int minTestRes,
                               const int maxTestRes) {
    SphereCap cap;
    if (cellToSphereCap(cell, &cap) != E_SUCCESS) {
        return false;
    }

    if (!_cellWithinCap(cell, &cap)) {
        return false;
    }

    int cellRes = H3_GET_RESOLUTION(cell);
    if (cellRes >= maxTestRes) {
        return true;
    }

    int64_t childCount = 0;
    if (H3_EXPORT(cellToChildrenSize)(cell, cellRes + 1, &childCount) !=
            E_SUCCESS ||
        childCount == 0) {
        return false;
    }

    H3Index *children = calloc(childCount, sizeof(H3Index));
    if (!children) {
        return false;
    }

    H3Error err = H3_EXPORT(cellToChildren)(cell, cellRes + 1, children);
    if (err != E_SUCCESS) {
        free(children);
        return false;
    }

    bool pass = true;
    for (int64_t i = 0; i < childCount; i++) {
        if (cellRes >= minTestRes) {
            if (!_cellWithinCap(children[i], &cap)) {
                pass = false;
                break;
            }
        }
        if (!_testCellRecursive(children[i], minTestRes, maxTestRes)) {
            pass = false;
            break;
        }
    }

    free(children);
    return pass;
}

SUITE(SphereCap) {
    TEST(cellToSphereCapOutputs) {
        const LatLng sf = {0.659966917655, -2.1364398519394};
        H3Index cell;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, 3, &cell));

        SphereCap cap;
        t_assertSuccess(cellToSphereCap(cell, &cap));
        t_assert(fabs(cap.cosRadius - 0x1.fff685c0bd115p-1) < 1e-15,
                 "Matches precomputed cosine bound at resolution 3");
        t_assert(fabs(vec3Mag(&cap.center) - 1.0) < 1e-12,
                 "Center vector is normalized");
    }

    TEST(testCap) {
        const int testRes = 0;
        const int maxTestRes = 5;

        if (testRes < 0 || testRes > MAX_H3_RES || maxTestRes < testRes ||
            maxTestRes > MAX_H3_RES) {
            t_assert(false, "testCaps invalid resolution parameters");
        }

        if (!_verifyCosRadiusValues()) {
            t_assert(false, "Precomputed cos radii failed verification");
        }

        for (int baseCell = 0; baseCell < NUM_BASE_CELLS; baseCell++) {
            H3Index baseIndex = baseCellNumToCell(baseCell);
            if (!_testCellRecursive(baseIndex, testRes, maxTestRes)) {
                t_assert(false, "Bounding cap test failed");
            }
        }
    }
}
