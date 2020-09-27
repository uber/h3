/*
 * Copyright 2020 Uber Technologies, Inc.
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
/** @file generatePentagonDirectionFaces.c
 * @brief Generates the pentagonDirectionFaces table
 *
 *  usage: `generatePentagonDirectionFaces`
 */

#include <stdlib.h>

#include "algos.h"
#include "baseCells.h"
#include "h3Index.h"

static void generate() {
    H3Index pentagons[NUM_PENTAGONS] = {0};
    // Get the res 2 pentagons, whose neighbors have the same base cell
    // and are unambiguously on the correct faces
    H3_EXPORT(getPentagonIndexes)(2, pentagons);

    printf(
        "static const PentagonDirectionFaces "
        "pentagonDirectionFaces[NUM_PENTAGONS] = "
        "{\n");

    for (int i = 0; i < NUM_PENTAGONS; i++) {
        H3Index pentagon = pentagons[i];
        int baseCell = H3_EXPORT(h3GetBaseCell)(pentagon);
        printf("    {%d, {", baseCell);
        // Get the neighbors in each direction, in order
        FaceIJK fijk;
        for (Direction dir = J_AXES_DIGIT; dir < NUM_DIGITS; dir++) {
            int r = 0;
            H3Index neighbor = h3NeighborRotations(pentagon, dir, &r);
            _h3ToFaceIjk(neighbor, &fijk);

            if (dir > J_AXES_DIGIT) printf(", ");
            printf("%d", fijk.face);
        }
        printf("}},\n");
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
