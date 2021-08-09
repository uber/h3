/*
 * Copyright 2018, 2020-2021 Uber Technologies, Inc.
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
/** @file generateFaceCenterPoint.c
 * @brief Generates the faceCenterPoint table
 *
 *  usage: `generateFaceCenterPoint`
 */

#include <stdlib.h>

#include "faceijk.h"
#include "vec3d.h"

/** @brief icosahedron face centers in lat/lng radians. Copied from faceijk.c.
 */
const LatLng faceCenterGeoCopy[NUM_ICOSA_FACES] = {
    {0.803582649718989942, 1.248397419617396099},    // face  0
    {1.307747883455638156, 2.536945009877921159},    // face  1
    {1.054751253523952054, -1.347517358900396623},   // face  2
    {0.600191595538186799, -0.450603909469755746},   // face  3
    {0.491715428198773866, 0.401988202911306943},    // face  4
    {0.172745327415618701, 1.678146885280433686},    // face  5
    {0.605929321571350690, 2.953923329812411617},    // face  6
    {0.427370518328979641, -1.888876200336285401},   // face  7
    {-0.079066118549212831, -0.733429513380867741},  // face  8
    {-0.230961644455383637, 0.506495587332349035},   // face  9
    {0.079066118549212831, 2.408163140208925497},    // face 10
    {0.230961644455383637, -2.635097066257444203},   // face 11
    {-0.172745327415618701, -1.463445768309359553},  // face 12
    {-0.605929321571350690, -0.187669323777381622},  // face 13
    {-0.427370518328979641, 1.252716453253507838},   // face 14
    {-0.600191595538186799, 2.690988744120037492},   // face 15
    {-0.491715428198773866, -2.739604450678486295},  // face 16
    {-0.803582649718989942, -1.893195233972397139},  // face 17
    {-1.307747883455638156, -0.604647643711872080},  // face 18
    {-1.054751253523952054, 1.794075294689396615},   // face 19
};

/**
 * Generates and prints the faceCenterPoint table.
 */
static void generate() {
    printf("static const Vec3d faceCenterPoint[NUM_ICOSA_FACES] = {\n");
    for (int i = 0; i < NUM_ICOSA_FACES; i++) {
        LatLng centerCoords = faceCenterGeoCopy[i];
        Vec3d centerPoint;
        _geoToVec3d(&centerCoords, &centerPoint);
        printf("    {%.16f, %.16f, %.16f}, // face %2d\n", centerPoint.x,
               centerPoint.y, centerPoint.z, i);
    }
    printf("};\n");
}

int main(int argc, char *argv[]) {
    // check command line args
    if (argc > 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(1);
    }

    generate();
}
