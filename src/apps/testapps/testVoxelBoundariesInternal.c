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
/** @file testVoxelBoundariesInternal.c
 * @brief Tests that every layer boundary at resolutions 0-10 locates into
 * the layer above it, and that the exact range ends behave.
 *
 * Internal because the public API does not expose layer floors until
 * `voxelToAltitudeBounds` exists (ticket 04), which should replace the
 * internal `layerFloorM` here.
 */

#include "h3voxel.h"
#include "test.h"
#include "utility.h"
#include "voxel.h"

// San Francisco, in radians.
static const LatLng sf = {0.659966917655, -2.1364398519396};

SUITE(voxelBoundariesInternal) {
    TEST(everyBoundaryBelongsToTheLayerAbove) {
        for (int res = 0; res <= 10; res++) {
            int32_t minLayer, maxLayer;
            t_assert(layerRange(res, &minLayer, &maxLayer), "has voxels");
            for (int32_t layer = minLayer; layer < maxLayer; layer++) {
                H3Index voxel;
                t_assertSuccess(H3_EXPORT(latLngAltToVoxel)(
                    &sf, layerFloorM(layer, res), res, NULL, &voxel));
                int32_t located;
                t_assertSuccess(H3_EXPORT(getLayerNumber)(voxel, &located));
                t_assert(located == layer, "floor belongs to its own layer");
            }
        }
    }

    TEST(exactRangeEnds) {
        for (int res = 0; res <= 10; res++) {
            int32_t minLayer, maxLayer;
            t_assert(layerRange(res, &minLayer, &maxLayer), "has voxels");
            H3Index voxel;
            t_assertSuccess(H3_EXPORT(latLngAltToVoxel)(
                &sf, layerFloorM(minLayer, res), res, NULL, &voxel));
            t_assert(H3_EXPORT(latLngAltToVoxel)(
                         &sf, layerFloorM(maxLayer, res), res, NULL, &voxel) ==
                         E_ALTITUDE_DOMAIN,
                     "the range ceiling is outside the range");
        }
    }
}
