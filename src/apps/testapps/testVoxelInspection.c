/*
 * Copyright 2026 J Reynolds
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
/** @file testVoxelInspection.c
 * @brief Tests voxel validity and inspection functions.
 *
 * Corrupt voxels are built from the published voxel index layout
 * (docs/adr/0005-packed-voxel-bit-layout.md): 0 | mode (4) | res (4) |
 * base cell (7) | digits (3 x res) | stored layer (48 - 3 x res).
 */

#include <string.h>

#include "h3api.h"
#include "h3voxel.h"
#include "test.h"
#include "utility.h"

#define MODE_SHIFT 59
#define RES_SHIFT 55
#define BASE_CELL_SHIFT 48

// San Francisco, in radians.
static const LatLng sf = {0.659966917655, -2.1364398519396};

static H3Index voxelAt(double altitudeM, int res) {
    H3Index voxel;
    t_assertSuccess(
        H3_EXPORT(latLngAltToVoxel)(&sf, altitudeM, res, NULL, &voxel));
    return voxel;
}

/** Replaces `width` bits of `index` starting at `shift` with `value`. */
static H3Index withBits(H3Index index, int shift, int width, uint64_t value) {
    uint64_t mask = ((UINT64_C(1) << width) - 1) << shift;
    return (index & ~mask) | ((value << shift) & mask);
}

SUITE(voxelInspection) {
    TEST(producedVoxelsAreValid) {
        const double altitudes[] = {-1700e3, -1.0,    0.0,
                                    120.0,   35786e3, 442800e3};
        for (int res = 0; res <= 10; res++) {
            for (size_t i = 0; i < sizeof(altitudes) / sizeof(altitudes[0]);
                 i++) {
                t_assert(H3_EXPORT(isValidVoxel)(voxelAt(altitudes[i], res)),
                         "produced voxel is valid");
            }
        }
    }

    TEST(voxelsAreNotCells) {
        for (int res = 0; res <= 10; res++) {
            t_assert(!H3_EXPORT(isValidCell)(voxelAt(120.0, res)),
                     "isValidCell rejects a voxel");
        }
    }

    TEST(cellsAreNotVoxels) {
        H3Index cell;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, 5, &cell));
        t_assert(!H3_EXPORT(isValidVoxel)(cell), "a cell is not a voxel");
        t_assert(!H3_EXPORT(isValidVoxel)(H3_NULL), "H3_NULL is not a voxel");
    }

    TEST(corruptVoxelsAreInvalid) {
        H3Index res5 = voxelAt(120.0, 5);
        H3Index res10 = voxelAt(120.0, 10);
        t_assert(!H3_EXPORT(isValidVoxel)(res5 | (UINT64_C(1) << 63)),
                 "high bit set");
        t_assert(!H3_EXPORT(isValidVoxel)(withBits(res5, MODE_SHIFT, 4, 1)),
                 "cell mode");
        t_assert(!H3_EXPORT(isValidVoxel)(withBits(res5, MODE_SHIFT, 4, 5)),
                 "mode 5");
        t_assert(!H3_EXPORT(isValidVoxel)(withBits(res5, RES_SHIFT, 4, 14)),
                 "res 14 has no voxels");
        t_assert(!H3_EXPORT(isValidVoxel)(withBits(res5, RES_SHIFT, 4, 15)),
                 "res 15 has no voxels");
        t_assert(
            !H3_EXPORT(isValidVoxel)(withBits(res5, BASE_CELL_SHIFT, 7, 122)),
            "base cell 122 does not exist");
        // Digit 1 of a res 5 voxel sits just below the base cell.
        t_assert(!H3_EXPORT(isValidVoxel)(withBits(res5, 45, 3, 7)),
                 "digit 7 inside the resolution");
        // Res 10 stores layers in 18 bits; 14 x 17496 = 244944 are valid.
        t_assert(!H3_EXPORT(isValidVoxel)(withBits(res10, 0, 18, 244944)),
                 "stored layer past the top of the range");
        t_assert(H3_EXPORT(isValidVoxel)(withBits(res10, 0, 18, 244943)),
                 "last stored layer in the range");
    }

    TEST(getVoxelResolution) {
        for (int res = 0; res <= 10; res++) {
            int out;
            t_assertSuccess(
                H3_EXPORT(getVoxelResolution)(voxelAt(120.0, res), &out));
            t_assert(out == res, "resolution round-trips");
        }
    }

    TEST(gettersRejectNonVoxels) {
        H3Index cell;
        t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, 5, &cell));
        H3Index corrupt = voxelAt(120.0, 5) | (UINT64_C(1) << 63);
        const H3Index bad[] = {cell, corrupt, H3_NULL};
        for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); i++) {
            int res;
            H3Index footprint;
            int32_t layer;
            t_assert(
                H3_EXPORT(getVoxelResolution)(bad[i], &res) == E_VOXEL_INVALID,
                "getVoxelResolution rejects a non-voxel");
            t_assert(H3_EXPORT(voxelToFootprint)(bad[i], &footprint) ==
                         E_VOXEL_INVALID,
                     "voxelToFootprint rejects a non-voxel");
            t_assert(
                H3_EXPORT(getLayerNumber)(bad[i], &layer) == E_VOXEL_INVALID,
                "getLayerNumber rejects a non-voxel");
        }
    }

    TEST(describeVoxelError) {
        t_assert(strcmp(H3_EXPORT(describeVoxelError)(E_VOXEL_INVALID),
                        "Voxel argument was not valid") == 0,
                 "describes E_VOXEL_INVALID");
        t_assert(
            strcmp(H3_EXPORT(describeVoxelError)(E_ALTITUDE_DOMAIN),
                   "Altitude argument was outside the altitude range of the "
                   "resolution, or the resolution has no voxels") == 0,
            "describes E_ALTITUDE_DOMAIN");
        t_assert(strcmp(H3_EXPORT(describeVoxelError)(E_LOOKUP_FAILED),
                        "Caller-supplied altitude lookup failed") == 0,
                 "describes E_LOOKUP_FAILED");
        // Everything else is described exactly as upstream describes it.
        const H3Error others[] = {E_SUCCESS, E_CELL_INVALID, E_RES_DOMAIN,
                                  999,       1003,           0xFFFFFFFF};
        for (size_t i = 0; i < sizeof(others) / sizeof(others[0]); i++) {
            t_assert(strcmp(H3_EXPORT(describeVoxelError)(others[i]),
                            H3_EXPORT(describeH3Error)(others[i])) == 0,
                     "falls back to describeH3Error");
        }
    }

    TEST(pentagonDeletedSubsequenceIsInvalid) {
        // Base cell 4 is a pentagon; a leading 1 digit is its deleted
        // subsequence, so such a footprint is not a valid cell.
        H3Index voxel = voxelAt(120.0, 1);
        voxel = withBits(voxel, BASE_CELL_SHIFT, 7, 4);
        t_assert(H3_EXPORT(isValidVoxel)(withBits(voxel, 45, 3, 0)),
                 "pentagon centre child is valid");
        t_assert(!H3_EXPORT(isValidVoxel)(withBits(voxel, 45, 3, 1)),
                 "pentagon deleted subsequence is invalid");
    }
}
