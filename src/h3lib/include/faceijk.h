/*
 * Copyright 2016-2019 Uber Technologies, Inc.
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
/** @file faceijk.h
 * @brief   FaceIJK functions including conversion to/from lat/lon.
 *
 *  References the Vec2d cartesian coordinate systems hex2d: local face-centered
 *     coordinate system scaled a specific H3 grid resolution unit length and
 *     with x-axes aligned with the local i-axes
 */

#ifndef FACEIJK_H
#define FACEIJK_H

#include "coordijk.h"
#include "geoCoord.h"
#include "vec2d.h"

/** @struct FaceIJK
 * @brief Face number and ijk coordinates on that face-centered coordinate
 * system
 */
typedef struct {
    int face;        ///< face number
    CoordIJK coord;  ///< ijk coordinates on that face
} FaceIJK;

/** @struct FaceOrientIJK
 * @brief Information to transform into an adjacent face IJK system
 */
typedef struct {
    int face;            ///< face number
    CoordIJK translate;  ///< res 0 translation relative to primary face
    int ccwRot60;  ///< number of 60 degree ccw rotations relative to primary
                   /// face
} FaceOrientIJK;

extern const GeoCoord faceCenterGeo[NUM_ICOSA_FACES];

// indexes for faceNeighbors table
/** IJ quadrant faceNeighbors table direction */
#define IJ 1
/** KI quadrant faceNeighbors table direction */
#define KI 2
/** JK quadrant faceNeighbors table direction */
#define JK 3

/** Invalid face index */
#define INVALID_FACE -1

/** Digit representing overage type */
typedef enum {
    /** No overage (on original face) */
    NO_OVERAGE = 0,
    /** On face edge (only occurs on substrate grids) */
    FACE_EDGE = 1,
    /** Overage on new face interior */
    NEW_FACE = 2
} Overage;

// Internal functions

void _geoToFaceIjk(const GeoCoord* g, int res, FaceIJK* h);
void _geoToHex2d(const GeoCoord* g, int res, int* face, Vec2d* v);
void _faceIjkToGeo(const FaceIJK* h, int res, GeoCoord* g);
void _faceIjkToGeoBoundary(const FaceIJK* h, int res, int start, int length,
                           GeoBoundary* g);
void _faceIjkPentToGeoBoundary(const FaceIJK* h, int res, int start, int length,
                               GeoBoundary* g);
void _faceIjkToVerts(FaceIJK* fijk, int* res, FaceIJK* fijkVerts);
void _faceIjkPentToVerts(FaceIJK* fijk, int* res, FaceIJK* fijkVerts);
void _hex2dToGeo(const Vec2d* v, int face, int res, int substrate, GeoCoord* g);
Overage _adjustOverageClassII(FaceIJK* fijk, int res, int pentLeading4,
                              int substrate);
Overage _adjustPentVertOverage(FaceIJK* fijk, int res);

#endif
