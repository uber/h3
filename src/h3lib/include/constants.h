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
/** @file constants.h
 * @brief   Constants used by more than one source code file.
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifndef M_PI
/** pi */
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
/** pi / 2.0 */
#define M_PI_2 1.5707963267948966
#endif

/** 2.0 * PI */
#define M_2PI 6.28318530717958647692528676655900576839433L

/** pi / 180 */
#define M_PI_180 0.0174532925199432957692369076848861271111L
/** pi * 180 */
#define M_180_PI 57.29577951308232087679815481410517033240547L

/** threshold epsilon */
#define EPSILON 0.0000000000000001L
/** sqrt(3) / 2.0 */
#define M_SQRT3_2 0.8660254037844386467637231707529361834714L
/** sin(60') */
#define M_SIN60 M_SQRT3_2

/** rotation angle between Class II and Class III resolution axes
 * (asin(sqrt(3.0 / 28.0))) */
#define M_AP7_ROT_RADS 0.333473172251832115336090755351601070065900389L

/** sin(M_AP7_ROT_RADS) */
#define M_SIN_AP7_ROT 0.3273268353539885718950318L

/** cos(M_AP7_ROT_RADS) */
#define M_COS_AP7_ROT 0.9449111825230680680167902L

/** earth radius in kilometers using WGS84 authalic radius */
#define EARTH_RADIUS_KM 6371.007180918475L

/** scaling factor from hex2d resolution 0 unit length
 * (or distance between adjacent cell center points
 * on the plane) to gnomonic unit length. */
#define RES0_U_GNOMONIC 0.38196601125010500003L

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
#define H3_HEXAGON_MODE 1
#define H3_UNIEDGE_MODE 2

#endif
