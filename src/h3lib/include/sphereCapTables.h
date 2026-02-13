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
/** @file sphereCapTables.h
 *  @brief Shared lookup tables for SphereCap calculations.
 *
 *  These values are used by the library implementation and tests. They are
 *  defined in a header as `static const` arrays so that including translation
 *  units receive private copies without exporting additional symbols.
 */

#ifndef SPHERE_CAP_TABLES_H
#define SPHERE_CAP_TABLES_H

#include "constants.h"

/**
 * Precomputed cosine values for the bounding cap radius at each resolution.
 * Each entry is an IEEE-754 hex float literal equal to
 *   cos(MAX_EDGE_LENGTH_RADS[res] * CAP_SCALE_FACTOR)
 *
 * CAP_SCALE_FACTOR (1.01) is the smallest multiplier such that a circle with
 * radius MAX_EDGE_LENGTH_RADS[res] * CAP_SCALE_FACTOR fully encloses the cell
 * and all of its descendants.
 */
#define H3_SPHERE_CAP_SCALE_FACTOR ((double)1.01)

static const double MAX_EDGE_LENGTH_RADS[MAX_H3_RES + 1] = {
    0.21577206265130, 0.08308767068495, 0.03148970436439, 0.01190662871439,
    0.00450053330908, 0.00170105523619, 0.00064293917678, 0.00024300820659,
    0.00009184847087, 0.00003471545901, 0.00001312121017, 0.00000495935129,
    0.00000187445860, 0.00000070847876, 0.00000026777980, 0.00000010121125};

static const double PRECOMPUTED_COS_RADIUS[MAX_H3_RES + 1] = {
    0x1.f3e3c5bae6cb9p-1, 0x1.fe32bec116ad3p-1, 0x1.ffbdb6ba770aep-1,
    0x1.fff685c0bd115p-1, 0x1.fffea559e01f7p-1, 0x1.ffffce7a554acp-1,
    0x1.fffff8ece6761p-1, 0x1.fffffefd45789p-1, 0x1.ffffffdb09ec8p-1,
    0x1.fffffffab8466p-1, 0x1.ffffffff3ee58p-1, 0x1.ffffffffe469fp-1,
    0x1.fffffffffc0f2p-1, 0x1.ffffffffff6fep-1, 0x1.ffffffffffeb7p-1,
    0x1.fffffffffffd1p-1};

#endif
