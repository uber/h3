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
/** @file h3voxel.h
 * @brief Public API for voxels: altitude-aware volumes built on H3 cells.
 *
 * A voxel is one cell (its footprint) combined with one layer of altitudes at
 * the same resolution. See GLOSSARY.md and docs/adr/ for the model. This
 * header is separate from h3api.h so upstream H3 merges never touch it
 * (docs/adr/0006-coexist-with-upstream-h3.md).
 */

#ifndef H3VOXEL_H
#define H3VOXEL_H

#include "h3api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Voxel error codes. They extend `H3ErrorCodes` from a block far above
 * upstream's codes, so upstream additions never collide with them (ADR-0006).
 * The `h3` CLI exits with the error code, which shells truncate to 8 bits, so
 * no voxel error code may be a multiple of 256 (it would exit with status 0).
 */
typedef enum {
    E_VOXEL_INVALID = 1000,    // `H3Index` voxel argument was not valid
    E_ALTITUDE_DOMAIN = 1001,  // Altitude was outside the resolution's
                               // altitude range, or the resolution has no
                               // voxels
    E_LOOKUP_FAILED = 1002,    // A caller-supplied altitude lookup failed
} H3VoxelErrorCodes;

/** @defgroup describeVoxelError describeVoxelError
 * Functions for describeVoxelError
 * @{
 */
/** @brief converts the provided H3Error value into a description string,
 * covering voxel error codes and falling back to `describeH3Error` for all
 * others */
DECLSPEC const char *H3_EXPORT(describeVoxelError)(H3Error err);
/** @} */

/** @brief What an input altitude is measured against. */
typedef enum {
    /** Height above the WGS84 ellipsoid (what GPS reports). */
    ALTITUDE_ELLIPSOIDAL = 0
} AltitudeType;

/** @brief How `latLngAltToVoxel` interprets its altitude argument. */
typedef struct {
    AltitudeType type;  ///< what the altitude is measured against
} AltitudeOptions;

/** @defgroup latLngAltToVoxel latLngAltToVoxel
 * Functions for latLngAltToVoxel
 * @{
 */
/** @brief find the voxel containing a lat/lng and altitude at a resolution */
DECLSPEC H3Error H3_EXPORT(latLngAltToVoxel)(const LatLng *g, double altitudeM,
                                             int res,
                                             const AltitudeOptions *options,
                                             H3Index *out);
/** @} */

/** @defgroup isValidVoxel isValidVoxel
 * Functions for isValidVoxel
 * @{
 */
/** @brief confirms if an H3Index is a valid voxel */
DECLSPEC int H3_EXPORT(isValidVoxel)(H3Index voxel);
/** @} */

/** @defgroup getVoxelResolution getVoxelResolution
 * Functions for getVoxelResolution
 * @{
 */
/** @brief returns a voxel's resolution. Upstream `getResolution` is not
 * voxel-aware and must not be used on voxels. */
DECLSPEC H3Error H3_EXPORT(getVoxelResolution)(H3Index voxel, int *out);
/** @} */

/** @defgroup voxelToFootprint voxelToFootprint
 * Functions for voxelToFootprint
 * @{
 */
/** @brief returns the cell a voxel lies over */
DECLSPEC H3Error H3_EXPORT(voxelToFootprint)(H3Index voxel, H3Index *out);
/** @} */

/** @defgroup getLayerNumber getLayerNumber
 * Functions for getLayerNumber
 * @{
 */
/** @brief returns a voxel's layer number, counted from altitude zero (layer 0
 * is the first layer above zero, layer -1 the first below) */
DECLSPEC H3Error H3_EXPORT(getLayerNumber)(H3Index voxel, int32_t *out);
/** @} */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
