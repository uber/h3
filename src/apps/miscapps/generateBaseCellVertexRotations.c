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
/** @file generateFaceCenterPoint.c
 * @brief Generates the faceCenterPoint table
 *
 *  usage: `generateFaceCenterPoint`
 */

#include <stdlib.h>

#include "algos.h"
#include "baseCells.h"
#include "faceijk.h"
#include "h3Index.h"
#include "vertex.h"

/**
 * Generates and prints the baseCellVertexRotations table.
 *
 * - For non-pentagons:
 *     - Reverse index from faceIjkBaseCells
 * - For pentagons:
 *     - Reverse index from faceIjkBaseCells
 *     - Polar pentagon:
 *         - If face isn't base cell home face, rotate -1
 *     - Non-polar pentagon:
 *         - If face is in IK direction, rotate -1
 *
 *  Then, to check for additional rotation:
 *     - If pentagon and
 *         (cellLeadingDigit === JK && face === IK) or
 *         (cellLeadingDigit === IK && face === JK):
 *             Rotate -1 (TODO: Check whether IK to JK is the other direction)
 */
static void generate() {
    BaseCellRotation rotations[NUM_BASE_CELLS][MAX_BASE_CELL_FACES] = {0};

    // Make a reverse index from faceIjkBaseCells
    for (int face = 0; face < NUM_ICOSA_FACES; face++) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                for (int k = 0; k < 3; k++) {
                    BaseCellOrient orient = faceIjkBaseCells[face][i][j][k];
                    int baseCell = orient.baseCell;
                    // Initialize the array if needed - {0, 0} is a valid
                    // rotation, se we can't use that as the "missing rotation"
                    if (rotations[baseCell][0].face == 0 &&
                        rotations[baseCell][1].face == 0) {
                        for (int r = 0; r < MAX_BASE_CELL_FACES; r++)
                            rotations[baseCell][r].face = INVALID_FACE;
                    }
                    // Now, look for either this base cell or an empty spot
                    for (int r = 0; r < MAX_BASE_CELL_FACES; r++) {
                        if (rotations[baseCell][r].face == face) break;
                        if (rotations[baseCell][r].face == INVALID_FACE) {
                            rotations[baseCell][r].face = face;
                            rotations[baseCell][r].ccwRot60 = orient.ccwRot60;
                            break;
                        }
                    }
                }
            }
        }
    }

    printf(
        "static const BaseCellRotation "
        "baseCellVertexRotations[NUM_BASE_CELLS][MAX_BASE_CELL_FACES] = "
        "{\n");
    for (int bc = 0; bc < NUM_BASE_CELLS; bc++) {
        printf("    {");
        if (!_isBaseCellPentagon(bc)) {
            for (int i = 0; i < MAX_BASE_CELL_FACES; i++) {
                BaseCellRotation rot = rotations[bc][i];
                if (i > 0) printf(", ");
                printf("{%d, %d}", rot.face, rot.ccwRot60);
            }
        } else {
            // Get the res 2 pentagon, whose neighbors have the same base cell
            // and are unambiguously on the correct faces
            H3Index pentRes2 = H3_INIT;
            setH3Index(&pentRes2, 2, bc, 0);

            // get the base cell face
            FaceIJK baseFijk;
            _baseCellToFaceIjk(bc, &baseFijk);

            // Get the neighbors in each direction, in order
            FaceIJK fijk;
            for (Direction dir = J_AXES_DIGIT; dir < NUM_DIGITS; dir++) {
                int r = 0;
                H3Index neighbor = h3NeighborRotations(pentRes2, dir, &r);
                _h3ToFaceIjk(neighbor, &fijk);

                // Find the base rotation
                int ccwRot60;
                for (int i = 0; i < MAX_BASE_CELL_FACES; i++) {
                    if (rotations[bc][i].face == fijk.face) {
                        ccwRot60 = rotations[bc][i].ccwRot60;
                        break;
                    }
                }

                // additional CCW rotation for polar neighbors or IK neighbors
                if (fijk.face != baseFijk.face &&
                    (_isBaseCellPolarPentagon(bc) || dir == IK_AXES_DIGIT)) {
                    // TBH, not sure why this is mod 6 and not mod 5
                    ccwRot60 = (ccwRot60 + 1) % NUM_HEX_VERTS;
                }

                if (dir > J_AXES_DIGIT) printf(", ");
                printf("{%d, %d}", fijk.face, ccwRot60);
            }
        }
        printf("}, // base cell %d%s\n", bc,
               _isBaseCellPentagon(bc) ? " (pent)" : "");
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
