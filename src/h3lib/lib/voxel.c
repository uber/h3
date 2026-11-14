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
/** @file voxel.c
 * @brief Voxels: altitude-aware volumes built on H3 cells.
 *
 * Voxel index layout (docs/adr/0005-packed-voxel-bit-layout.md), from the
 * high bit down:
 *
 *  | Region       | # bits                 |
 *  |--------------|------------------------|
 *  | High         | 1 (always 0)           |
 *  | Mode         | 4 (VOXEL_MODE)         |
 *  | Resolution   | 4                      |
 *  | Base cell    | 7                      |
 *  | Digits       | 3 x resolution         |
 *  | Stored layer | 48 - 3 x resolution    |
 */

#include "voxel.h"

#include <math.h>
#include <stdbool.h>

#include "h3Index.h"
#include "h3voxel.h"

/** Voxel index mode, from the far end of the mode space (ADR-0006). */
#define VOXEL_MODE 15

#define VOXEL_MODE_OFFSET 59
#define VOXEL_MODE_MASK UINT64_C(0xF)
#define VOXEL_RES_OFFSET 55
#define VOXEL_RES_MASK UINT64_C(0xF)
#define VOXEL_BASE_CELL_OFFSET 48
#define VOXEL_BASE_CELL_MASK UINT64_C(0x7F)

/** Number of bits below the base cell: digits plus stored layer. */
#define VOXEL_LOW_BITS 48

/** WGS84 semi-major axis in metres: the radius layers are measured from. */
#define WGS84_A_M 6378137.0

/**
 * Log-radial thickness of a resolution 0 layer: sqrt(H3's average resolution
 * 0 hexagon area) / WGS84_A. Fixed by the spec; changing it changes the
 * meaning of every stored voxel (docs/adr/0003-nested-log-radial-layers.md).
 */
#define RES0_LAYER_LOG_THICKNESS 0.32728216066118615

/**
 * Number of layers at each resolution per resolution 0 layer: the running
 * product of the split schedule 3 2 3 3 2 3 3 3 2 3 3 2 3 3 2 (ADR-0003).
 */
static const int32_t LAYERS_PER_RES0_LAYER[MAX_H3_RES + 1] = {
    1,    3,    6,     18,    54,     108,    324,    972,
    2916, 5832, 17496, 52488, 104976, 314928, 944784, 1889568};

/** Number of resolution 0 layers below altitude zero in the full range. */
#define RES0_LAYERS_BELOW_ZERO 1
/** Number of resolution 0 layers above altitude zero in the full range. */
#define RES0_LAYERS_ABOVE_ZERO 13
/** Finest resolution whose altitude range is the full range. */
#define MAX_FULL_RANGE_RES 10

bool layerRange(int res, int32_t *min, int32_t *maxExclusive) {
    if (res < 0 || res > MAX_FULL_RANGE_RES) {
        return false;
    }
    *min = -RES0_LAYERS_BELOW_ZERO * LAYERS_PER_RES0_LAYER[res];
    *maxExclusive = RES0_LAYERS_ABOVE_ZERO * LAYERS_PER_RES0_LAYER[res];
    return true;
}

double layerFloorM(int32_t layer, int res) {
    return WGS84_A_M * expm1((double)layer * RES0_LAYER_LOG_THICKNESS /
                             LAYERS_PER_RES0_LAYER[res]);
}

/**
 * Finds the layer containing an ellipsoidal altitude at a resolution with
 * voxels. Layer boundaries are defined by `layerFloorM`: the log1p estimate
 * can round to a neighbouring layer near a boundary, so it is snapped
 * against the floors it lies between. Returns false outside the range.
 */
static bool _locateLayer(double altitudeM, int res, int32_t *out) {
    int32_t minLayer, maxLayer;
    if (!layerRange(res, &minLayer, &maxLayer) || !isfinite(altitudeM)) {
        return false;
    }
    double estimate =
        floor(log1p(altitudeM / WGS84_A_M) * LAYERS_PER_RES0_LAYER[res] /
              RES0_LAYER_LOG_THICKNESS);
    // Written so a NaN estimate (altitude at or below the Earth's centre)
    // fails. One layer of slack either side leaves room to snap.
    if (!(estimate >= minLayer - 1 && estimate <= maxLayer)) {
        return false;
    }
    int32_t layer = (int32_t)estimate;
    if (altitudeM < layerFloorM(layer, res)) {
        layer--;
    } else if (altitudeM >= layerFloorM(layer + 1, res)) {
        layer++;
    }
    if (layer < minLayer || layer >= maxLayer) {
        return false;
    }
    *out = layer;
    return true;
}

/** Number of bits holding the stored layer at a resolution. */
static int _storedLayerBits(int res) { return VOXEL_LOW_BITS - 3 * res; }

static int _voxelRes(H3Index voxel) {
    return (int)((voxel >> VOXEL_RES_OFFSET) & VOXEL_RES_MASK);
}

/** The layer number counted from the bottom of the resolution's range. */
static uint64_t _storedLayer(H3Index voxel) {
    uint64_t storedMask =
        (UINT64_C(1) << _storedLayerBits(_voxelRes(voxel))) - 1;
    return voxel & storedMask;
}

/** Mask of the low `3 * res` bits, which hold a resolution's digits. */
static uint64_t _digitsMask(int res) { return (UINT64_C(1) << (3 * res)) - 1; }

/** Number of cell digit bits past a resolution, filled with 7s in a cell. */
static int _unusedDigitBits(int res) { return 3 * (MAX_H3_RES - res); }

static H3Index _makeVoxel(H3Index cell, uint64_t storedLayer) {
    int res = H3_GET_RESOLUTION(cell);
    uint64_t digits = (cell >> _unusedDigitBits(res)) & _digitsMask(res);
    return ((uint64_t)VOXEL_MODE << VOXEL_MODE_OFFSET) |
           ((uint64_t)res << VOXEL_RES_OFFSET) |
           ((uint64_t)H3_GET_BASE_CELL(cell) << VOXEL_BASE_CELL_OFFSET) |
           (digits << _storedLayerBits(res)) | storedLayer;
}

static H3Index _footprint(H3Index voxel) {
    int res = _voxelRes(voxel);
    int baseCell =
        (int)((voxel >> VOXEL_BASE_CELL_OFFSET) & VOXEL_BASE_CELL_MASK);
    uint64_t digits = (voxel >> _storedLayerBits(res)) & _digitsMask(res);
    int unusedBits = _unusedDigitBits(res);

    // H3_INIT has every digit set to 7; replace the used digits.
    H3Index cell = H3_INIT;
    H3_SET_MODE(cell, H3_CELL_MODE);
    H3_SET_RESOLUTION(cell, res);
    H3_SET_BASE_CELL(cell, baseCell);
    cell &= ~(_digitsMask(res) << unusedBits);
    cell |= digits << unusedBits;
    return cell;
}

const char *H3_EXPORT(describeVoxelError)(H3Error err) {
    switch (err) {
        case E_VOXEL_INVALID:
            return "Voxel argument was not valid";
        case E_ALTITUDE_DOMAIN:
            return "Altitude argument was outside the altitude range of the "
                   "resolution, or the resolution has no voxels";
        case E_LOOKUP_FAILED:
            return "Caller-supplied altitude lookup failed";
        default:
            return H3_EXPORT(describeH3Error)(err);
    }
}

H3Error H3_EXPORT(latLngAltToVoxel)(const LatLng *g, double altitudeM, int res,
                                    const AltitudeOptions *options,
                                    H3Index *out) {
    if (options != NULL && options->type != ALTITUDE_ELLIPSOIDAL) {
        return E_OPTION_INVALID;
    }
    if (res < 0 || res > MAX_H3_RES) {
        return E_RES_DOMAIN;
    }
    H3Index cell;
    H3Error err = H3_EXPORT(latLngToCell)(g, res, &cell);
    if (err) {
        return err;
    }
    int32_t layer;
    if (!_locateLayer(altitudeM, res, &layer)) {
        return E_ALTITUDE_DOMAIN;
    }
    int32_t minLayer, maxLayer;
    layerRange(res, &minLayer, &maxLayer);
    *out = _makeVoxel(cell, (uint64_t)(layer - minLayer));
    return E_SUCCESS;
}

int H3_EXPORT(isValidVoxel)(H3Index voxel) {
    if (voxel >> 63) {
        return false;
    }
    if (((voxel >> VOXEL_MODE_OFFSET) & VOXEL_MODE_MASK) != VOXEL_MODE) {
        return false;
    }
    int32_t minLayer, maxLayer;
    if (!layerRange(_voxelRes(voxel), &minLayer, &maxLayer)) {
        return false;
    }
    if (_storedLayer(voxel) >= (uint64_t)(maxLayer - minLayer)) {
        return false;
    }
    // Checks the base cell and digits, including pentagon deleted
    // subsequences.
    return H3_EXPORT(isValidCell)(_footprint(voxel));
}

H3Error H3_EXPORT(getVoxelResolution)(H3Index voxel, int *out) {
    if (!H3_EXPORT(isValidVoxel)(voxel)) {
        return E_VOXEL_INVALID;
    }
    *out = _voxelRes(voxel);
    return E_SUCCESS;
}

H3Error H3_EXPORT(getLayerNumber)(H3Index voxel, int32_t *out) {
    if (!H3_EXPORT(isValidVoxel)(voxel)) {
        return E_VOXEL_INVALID;
    }
    int32_t minLayer, maxLayer;
    layerRange(_voxelRes(voxel), &minLayer, &maxLayer);
    *out = (int32_t)_storedLayer(voxel) + minLayer;
    return E_SUCCESS;
}

H3Error H3_EXPORT(voxelToFootprint)(H3Index voxel, H3Index *out) {
    if (!H3_EXPORT(isValidVoxel)(voxel)) {
        return E_VOXEL_INVALID;
    }
    *out = _footprint(voxel);
    return E_SUCCESS;
}
