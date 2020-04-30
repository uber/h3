/*
 * Copyright 2018 Uber Technologies, Inc.
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

/**
 * Generates and prints the faceCenterPoint table.
 */
static void generate() {
    printf("static const Vec3d faceCenterPoint[NUM_ICOSA_FACES] = {\n");
    for (int i = 0; i < NUM_ICOSA_FACES; i++) {
        GeoCoord centerCoords = faceCenterGeo[i];
        Vec3d centerPoint;
        _geoToVec3d(&centerCoords, &centerPoint);
        printf("    {%.16f, %.16f, %.16f}, // face %2d\n", centerPoint.x,
               centerPoint.y, centerPoint.z, i);
    }
    printf("};\n");
}

int main(int argc, char* argv[]) {
    // check command line args
    if (argc > 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(1);
    }

    generate();
}
