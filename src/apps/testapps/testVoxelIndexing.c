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
/** @file testVoxelIndexing.c
 * @brief Tests locating voxels from lat/lng/altitude (`latLngAltToVoxel`).
 *
 * Expected layer numbers and altitude ranges are worked examples computed
 * independently from the spec (.scratch/voxels/spec.md), not by the library.
 */

#include <math.h>
#include <string.h>

#include "h3api.h"
#include "h3voxel.h"
#include "test.h"
#include "utility.h"

// San Francisco, in radians.
static const LatLng sf = {0.659966917655, -2.1364398519396};

SUITE(voxelIndexing) {
    TEST(footprintIsLatLngToCell) {
        for (int res = 0; res <= 10; res++) {
            H3Index cell;
            t_assertSuccess(H3_EXPORT(latLngToCell)(&sf, res, &cell));

            H3Index voxel;
            t_assertSuccess(
                H3_EXPORT(latLngAltToVoxel)(&sf, 100.0, res, NULL, &voxel));
            H3Index footprint;
            t_assertSuccess(H3_EXPORT(voxelToFootprint)(voxel, &footprint));
            t_assert(footprint == cell, "footprint is latLngToCell's cell");
        }
    }

    TEST(layerNumberCountsFromAltitudeZero) {
        // Worked examples. Near the ground a res 0 layer is ~2,470 km thick
        // and a res 10 layer ~119.3 m; layer 0 is the first above zero.
        const struct {
            double altitudeM;
            int res;
            int32_t layer;
        } cases[] = {
            {0.0, 0, 0},       {-1.0, 0, -1},   {2000e3, 0, 0},
            {3000e3, 0, 1},    {35786e3, 0, 5}, {-1700e3, 0, -1},
            {440000e3, 0, 12}, {100.0, 10, 0},  {150.0, 10, 1},
            {-1.0, 10, -1},    {0.0, 10, 0},
        };
        for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
            H3Index voxel;
            t_assertSuccess(H3_EXPORT(latLngAltToVoxel)(
                &sf, cases[i].altitudeM, cases[i].res, NULL, &voxel));
            int32_t layer;
            t_assertSuccess(H3_EXPORT(getLayerNumber)(voxel, &layer));
            t_assert(layer == cases[i].layer, "expected layer number");
        }
    }

    TEST(altitudeRangeEnds) {
        // The full range is about -1,780.264 km to +442,855.965 km.
        H3Index voxel;
        for (int res = 0; res <= 10; res++) {
            t_assertSuccess(
                H3_EXPORT(latLngAltToVoxel)(&sf, -1780e3, res, NULL, &voxel));
            t_assertSuccess(
                H3_EXPORT(latLngAltToVoxel)(&sf, 442800e3, res, NULL, &voxel));
            t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, -1781e3, res, NULL,
                                                 &voxel) == E_ALTITUDE_DOMAIN,
                     "below the range");
            t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, 442900e3, res, NULL,
                                                 &voxel) == E_ALTITUDE_DOMAIN,
                     "above the range");
        }
    }

    TEST(nonFiniteAltitude) {
        H3Index voxel;
        t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, NAN, 5, NULL, &voxel) ==
                     E_ALTITUDE_DOMAIN,
                 "NaN altitude");
        t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, INFINITY, 5, NULL, &voxel) ==
                     E_ALTITUDE_DOMAIN,
                 "infinite altitude");
    }

    TEST(resolutionsWithoutVoxels) {
        H3Index voxel;
        t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, 0.0, 14, NULL, &voxel) ==
                     E_ALTITUDE_DOMAIN,
                 "res 14 has no voxels");
        t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, 0.0, 15, NULL, &voxel) ==
                     E_ALTITUDE_DOMAIN,
                 "res 15 has no voxels");
        t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, 0.0, -1, NULL, &voxel) ==
                     E_RES_DOMAIN,
                 "negative resolution");
        t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, 0.0, 16, NULL, &voxel) ==
                     E_RES_DOMAIN,
                 "resolution above 15");
    }

    TEST(goldenVoxels) {
        // Packed by hand from the spec's layout; pins the stored format
        // (docs/adr/0005-packed-voxel-bit-layout.md).
        const struct {
            double altitudeM;
            int res;
            H3Index voxel;
        } cases[] = {
            {1000.0, 9, 0x7c9418406e4016ca},   // layer 2 over 89283080dcbffff
            {-500.0, 9, 0x7c9418406e4016c6},   // layer -2 over 89283080dcbffff
            {35786e3, 5, 0x7a941840000002db},  // layer 623 over 85283083fffffff
        };
        for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
            H3Index voxel;
            t_assertSuccess(H3_EXPORT(latLngAltToVoxel)(
                &sf, cases[i].altitudeM, cases[i].res, NULL, &voxel));
            t_assert(voxel == cases[i].voxel, "golden voxel index");
        }
    }

    TEST(stringRoundTrip) {
        H3Index voxel;
        t_assertSuccess(
            H3_EXPORT(latLngAltToVoxel)(&sf, 1000.0, 9, NULL, &voxel));
        char str[17];
        t_assertSuccess(H3_EXPORT(h3ToString)(voxel, str, sizeof(str)));
        t_assert(strcmp(str, "7c9418406e4016ca") == 0, "hex string form");
        H3Index parsed;
        t_assertSuccess(H3_EXPORT(stringToH3)(str, &parsed));
        t_assert(parsed == voxel, "string round-trips");
    }

    TEST(pentagonFootprint) {
        H3Index pentagons[12];
        for (int res = 0; res <= 10; res++) {
            t_assertSuccess(H3_EXPORT(getPentagons)(res, pentagons));
            for (int i = 0; i < 12; i++) {
                LatLng centre;
                t_assertSuccess(H3_EXPORT(cellToLatLng)(pentagons[i], &centre));
                H3Index voxel;
                t_assertSuccess(H3_EXPORT(latLngAltToVoxel)(&centre, 1000.0,
                                                            res, NULL, &voxel));
                t_assert(H3_EXPORT(isValidVoxel)(voxel), "valid voxel");
                H3Index footprint;
                t_assertSuccess(H3_EXPORT(voxelToFootprint)(voxel, &footprint));
                t_assert(footprint == pentagons[i], "pentagon footprint");
            }
        }
    }

    TEST(columnsSortContiguously) {
        // Voxels over a footprint and all six of its neighbours, at three
        // resolutions, generated altitude-first so columns start out
        // interleaved; sorted as unsigned integers, each column must be one
        // contiguous run in ascending layer order.
        const double altitudes[] = {442800e3, -1.0,  35786e3, 0.0,
                                    -1700e3,  120.0, 9000.0};
        const int nAlt = sizeof(altitudes) / sizeof(altitudes[0]);
        const int resolutions[] = {3, 7, 10};
        LatLng centres[3][7];
        for (int r = 0; r < 3; r++) {
            H3Index origin, disk[7];
            t_assertSuccess(
                H3_EXPORT(latLngToCell)(&sf, resolutions[r], &origin));
            t_assertSuccess(H3_EXPORT(gridDisk)(origin, 1, disk));
            for (int c = 0; c < 7; c++) {
                t_assert(disk[c] != H3_NULL, "hexagon has six neighbours");
                t_assertSuccess(
                    H3_EXPORT(cellToLatLng)(disk[c], &centres[r][c]));
            }
        }
        H3Index voxels[3 * 7 * 7];
        int n = 0;
        for (int a = 0; a < nAlt; a++) {
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 7; c++) {
                    t_assertSuccess(H3_EXPORT(latLngAltToVoxel)(
                        &centres[r][c], altitudes[a], resolutions[r], NULL,
                        &voxels[n++]));
                }
            }
        }
        // Insertion sort as unsigned 64-bit integers.
        for (int i = 1; i < n; i++) {
            H3Index v = voxels[i];
            int j = i - 1;
            for (; j >= 0 && voxels[j] > v; j--) voxels[j + 1] = voxels[j];
            voxels[j + 1] = v;
        }
        for (int i = 0; i < n; i++) {
            H3Index footprint;
            int32_t layer;
            t_assertSuccess(H3_EXPORT(voxelToFootprint)(voxels[i], &footprint));
            t_assertSuccess(H3_EXPORT(getLayerNumber)(voxels[i], &layer));
            for (int j = i + 1; j < n; j++) {
                H3Index otherFootprint;
                int32_t otherLayer;
                t_assertSuccess(
                    H3_EXPORT(voxelToFootprint)(voxels[j], &otherFootprint));
                t_assertSuccess(
                    H3_EXPORT(getLayerNumber)(voxels[j], &otherLayer));
                if (otherFootprint == footprint) {
                    t_assert(otherLayer >= layer, "column sorted by layer");
                    // Everything between i and j shares the footprint.
                    for (int k = i + 1; k < j; k++) {
                        H3Index between;
                        t_assertSuccess(
                            H3_EXPORT(voxelToFootprint)(voxels[k], &between));
                        t_assert(between == footprint, "column contiguous");
                    }
                }
            }
        }
    }

    TEST(altitudeOptions) {
        H3Index withoutOptions, withOptions, voxel;
        t_assertSuccess(
            H3_EXPORT(latLngAltToVoxel)(&sf, 1000.0, 9, NULL, &withoutOptions));
        AltitudeOptions ellipsoidal = {.type = ALTITUDE_ELLIPSOIDAL};
        t_assertSuccess(H3_EXPORT(latLngAltToVoxel)(
            &sf, 1000.0, 9, &ellipsoidal, &withOptions));
        t_assert(withOptions == withoutOptions,
                 "NULL options means ellipsoidal altitude");

        AltitudeOptions unknown = {.type = (AltitudeType)99};
        t_assert(H3_EXPORT(latLngAltToVoxel)(&sf, 1000.0, 9, &unknown,
                                             &voxel) == E_OPTION_INVALID,
                 "unknown altitude type");
    }

    TEST(invalidLatLng) {
        LatLng bad = {NAN, 0.0};
        H3Index voxel;
        t_assert(H3_EXPORT(latLngAltToVoxel)(&bad, 0.0, 5, NULL, &voxel) ==
                     E_LATLNG_DOMAIN,
                 "non-finite latitude");
    }
}
