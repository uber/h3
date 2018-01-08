/*
 * Copyright 2016-2017 Uber Technologies, Inc.
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
/** @file h3api.c
 * @brief   Major H3 core library entry points.
 */

#include "h3api.h"
#include <math.h>
#include <stdio.h>
#include "h3Index.h"

/**
 * Encodes a coordinate on the sphere to the H3 index of the containing cell at
 * the specified resolution.
 *
 * Returns 0 on invalid input.
 *
 * @param g The spherical coordinates to encode.
 * @param res The desired H3 resolution for the encoding.
 * @return The encoded H3Index (or 0 on failure).
 */
H3Index H3_EXPORT(geoToH3)(const GeoCoord* g, int res) {
    H3IndexFat hf;

    if (res < 0 || res > MAX_H3_RES) {
        return 0;
    }
    if (!isfinite(g->lat) || !isfinite(g->lon)) {
        return 0;
    }

    geoToH3Fat(g, res, &hf);
    return h3FatToH3(&hf);
}

/**
 * Determines the spherical coordinates of the center point of an H3 index.
 *
 * @param h3 The H3 index.
 * @param g The spherical coordinates of the H3 cell center.
 */
void H3_EXPORT(h3ToGeo)(H3Index h3, GeoCoord* g) {
    H3IndexFat hf;
    h3ToH3Fat(h3, &hf);
    h3FatToGeo(&hf, g);
}

/**
 * Determines the cell boundary in spherical coordinates for an H3 index.
 *
 * @param h3 The H3 index.
 * @param gp The boundary of the H3 cell in spherical coordinates.
 */
void H3_EXPORT(h3ToGeoBoundary)(H3Index h3, GeoBoundary* gp) {
    H3IndexFat hf;
    h3ToH3Fat(h3, &hf);
    h3FatToGeoBoundary(&hf, gp);
}
