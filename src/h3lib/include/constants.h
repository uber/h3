/*
 * Copyright 2016-2017, 2020 Uber Technologies, Inc.
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
/** @file constants.h
 * @brief   Constants used by more than one source code file.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <math.h>

#ifndef M_PI
/** pi */
static const double M_PI = 3.14159265358979323846;
#endif

#ifndef M_PI_2
/** pi / 2.0 */
static const double M_PI_2  = M_PI / 2.0;
#endif

/** 2.0 * PI */
static const double M_2PI = 2.0 * M_PI;

/** pi / 180 */
static const double M_PI_180 = M_PI / 180.0;
/** 180 / pi */
static const double M_180_PI = 180 / M_PI;

/** threshold epsilon */
static const double EPSILON = 0.0000000000000001;
/** sqrt(3) / 2.0 */
static const double M_SQRT3_2 = sqrt(3) / 2.0;
/** sin(60') */
static const double M_SIN60 = M_SQRT3_2;
/** 1/sin(60') **/
static const double M_RSIN60 = 1.0 / M_SQRT3_2;

/** one third **/
static const double M_ONETHIRD = 1.0 / 3.0;

/** one seventh (1/7) **/
static const double M_ONESEVENTH = 1.0 / 7.0;

/** rotation angle between Class II and Class III resolution axes
 * (asin(sqrt(3.0 / 28.0))) */
static const double M_AP7_ROT_RADS = asin(sqrt(3.0 / 28.0));

/** sin(M_AP7_ROT_RADS) */
static const double M_SIN_AP7_ROT = sin(asin(sqrt(3.0 / 28.0)));

/** cos(M_AP7_ROT_RADS) */
static const double M_COS_AP7_ROT = cos(asin(sqrt(3.0 / 28.0)));

/** earth radius in kilometers using WGS84 authalic radius */
static const double EARTH_RADIUS_KM = 6371.007180918475;

/** scaling factor from hex2d resolution 0 unit length
 * (or distance between adjacent cell center points
 * on the plane) to gnomonic unit length. */
static const double RES0_U_GNOMONIC = 0.38196601125010500003;
static const double INV_RES0_U_GNOMONIC = 1.0 / RES0_U_GNOMONIC;

/** max H3 resolution; H3 version 1 has 16 resolutions, numbered 0 through 15 */
#define MAX_H3_RES 15

/** The number of faces on an icosahedron */
#define NUM_ICOSA_FACES 20
/** The number of H3 base cells */
#define NUM_BASE_CELLS 122
/** The number of vertices in a hexagon */
#define NUM_HEX_VERTS 6
/** The number of vertices in a pentagon */
#define NUM_PENT_VERTS 5
/** The number of pentagons per resolution **/
#define NUM_PENTAGONS 12

/** H3 index modes */
#define H3_CELL_MODE 1
#define H3_DIRECTEDEDGE_MODE 2
#define H3_EDGE_MODE 3
#define H3_VERTEX_MODE 4

#endif
