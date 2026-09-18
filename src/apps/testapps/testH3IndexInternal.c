/*
 * Copyright 2017-2018, 2020-2021 Uber Technologies, Inc.
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
/** @file
 * @brief tests H3 functions for manipulating H3 indexes
 *
 *  usage: `testH3IndexInternal`
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "h3Index.h"
#include "test.h"
#include "utility.h"
#include "vec3d.h"

SUITE(h3IndexInternal) {
    TEST(faceIjkToH3ExtremeCoordinates) {
        FaceIJK fijk0I = {0, {3, 0, 0}};
        t_assert(_faceIjkToH3(&fijk0I, 0) == 0, "i out of bounds at res 0");
        FaceIJK fijk0J = {1, {0, 4, 0}};
        t_assert(_faceIjkToH3(&fijk0J, 0) == 0, "j out of bounds at res 0");
        FaceIJK fijk0K = {2, {2, 0, 5}};
        t_assert(_faceIjkToH3(&fijk0K, 0) == 0, "k out of bounds at res 0");

        FaceIJK fijk1I = {3, {6, 0, 0}};
        t_assert(_faceIjkToH3(&fijk1I, 1) == 0, "i out of bounds at res 1");
        FaceIJK fijk1J = {4, {0, 7, 1}};
        t_assert(_faceIjkToH3(&fijk1J, 1) == 0, "j out of bounds at res 1");
        FaceIJK fijk1K = {5, {2, 0, 8}};
        t_assert(_faceIjkToH3(&fijk1K, 1) == 0, "k out of bounds at res 1");

        FaceIJK fijk2I = {6, {18, 0, 0}};
        t_assert(_faceIjkToH3(&fijk2I, 2) == 0, "i out of bounds at res 2");
        FaceIJK fijk2J = {7, {0, 19, 1}};
        t_assert(_faceIjkToH3(&fijk2J, 2) == 0, "j out of bounds at res 2");
        FaceIJK fijk2K = {8, {2, 0, 20}};
        t_assert(_faceIjkToH3(&fijk2K, 2) == 0, "k out of bounds at res 2");
    }

    TEST(vec3ToCell_invalidInputs) {
        Vec3d v = {1.0, 0.0, 0.0};
        H3Index out;

        // Test invalid resolution (negative)
        t_assert(vec3ToCell(&v, -1, &out) == E_RES_DOMAIN,
                 "negative resolution rejected");

        // Test invalid resolution (too high)
        t_assert(vec3ToCell(&v, MAX_H3_RES + 1, &out) == E_RES_DOMAIN,
                 "resolution above MAX_H3_RES rejected");

        // Test non-finite coordinates
        Vec3d nanVec = {NAN, 0.0, 0.0};
        t_assert(vec3ToCell(&nanVec, 5, &out) == E_DOMAIN,
                 "NaN x coordinate rejected");

        Vec3d infVec = {INFINITY, 0.0, 0.0};
        t_assert(vec3ToCell(&infVec, 5, &out) == E_DOMAIN,
                 "Infinity x coordinate rejected");

        Vec3d nanY = {0.0, NAN, 0.0};
        t_assert(vec3ToCell(&nanY, 5, &out) == E_DOMAIN,
                 "NaN y coordinate rejected");

        Vec3d nanZ = {0.0, 0.0, NAN};
        t_assert(vec3ToCell(&nanZ, 5, &out) == E_DOMAIN,
                 "NaN z coordinate rejected");
    }

    TEST(cellToVec3_invalidCell) {
        Vec3d v;
        H3Index invalid = 0xFFFFFFFFFFFFFFFF;
        t_assert(cellToVec3(invalid, &v) != E_SUCCESS,
                 "invalid H3 index rejected in cellToVec3");
    }

    TEST(cellToGeodesicBoundary_invalidCell) {
        GeodesicCellBoundary cb;
        H3Index invalid = 0xFFFFFFFFFFFFFFFF;
        t_assert(cellToGeodesicBoundary(invalid, &cb) != E_SUCCESS,
                 "invalid H3 index rejected in cellToGeodesicBoundary");
    }
}
