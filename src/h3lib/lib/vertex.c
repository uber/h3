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
/** @file  vertex.h
 *  @brief Functions for working with cell vertexes.
 */

#include "vertex.h"

#include "baseCells.h"
#include "faceijk.h"
#include "geoCoord.h"
#include "h3Index.h"

// No base cell crosses more than 5 faces
const int MAX_BASE_CELL_FACES = 5;

/** @brief Base cell vertex rotation table
 *
 * For each base cell, gives required CCW rotations to rotate
 * the vertexes on a given face to the orientation of the base
 * cell's home face.
 */
static const BaseCellRotation
    baseCellVertexRotations[NUM_BASE_CELLS][MAX_BASE_CELL_FACES] = {
        {{0, 5}, {1, 0}, {2, 1}, {0}, {0}},             // base cell 0
        {{1, 5}, {2, 0}, {0}, {0}, {0}},                // base cell 1
        {{0, 5}, {1, 0}, {2, 1}, {6, 3}, {0}},          // base cell 2
        {{1, 5}, {2, 0}, {3, 1}, {0}, {0}},             // base cell 3
        {{4, 5}, {0, 0}, {2, 3}, {1, 2}, {3, 4}},       // base cell 4
        {{0, 5}, {1, 0}, {0}, {0}, {0}},                // base cell 5
        {{1, 0}, {2, 1}, {6, 3}, {0}, {0}},             // base cell 6
        {{1, 5}, {2, 0}, {3, 1}, {7, 3}, {0}},          // base cell 7
        {{0, 0}, {1, 1}, {4, 5}, {0}, {0}},             // base cell 8
        {{1, 5}, {2, 0}, {7, 3}, {0}, {0}},             // base cell 9
        {{0, 5}, {1, 0}, {6, 3}, {0}, {0}},             // base cell 10
        {{1, 0}, {6, 3}, {0}, {0}, {0}},                // base cell 11
        {{2, 5}, {3, 0}, {4, 1}, {0}, {0}},             // base cell 12
        {{2, 5}, {3, 0}, {0}, {0}, {0}},                // base cell 13
        {{1, 0}, {6, 3}, {11, 0}, {2, 1}, {7, 4}},      // base cell 14
        {{0, 1}, {3, 5}, {4, 0}, {0}, {0}},             // base cell 15
        {{0, 0}, {1, 1}, {4, 5}, {5, 3}, {0}},          // base cell 16
        {{1, 3}, {6, 0}, {11, 3}, {0}, {0}},            // base cell 17
        {{0, 0}, {1, 1}, {5, 3}, {0}, {0}},             // base cell 18
        {{2, 0}, {7, 3}, {0}, {0}, {0}},                // base cell 19
        {{2, 3}, {7, 0}, {11, 3}, {0}, {0}},            // base cell 20
        {{2, 0}, {3, 1}, {7, 3}, {0}, {0}},             // base cell 21
        {{0, 0}, {4, 5}, {0}, {0}, {0}},                // base cell 22
        {{1, 3}, {6, 0}, {10, 3}, {0}, {0}},            // base cell 23
        {{10, 0}, {1, 1}, {6, 4}, {0, 0}, {5, 3}},      // base cell 24
        {{1, 3}, {6, 0}, {10, 3}, {11, 3}, {0}},        // base cell 25
        {{2, 5}, {3, 0}, {4, 1}, {8, 3}, {0}},          // base cell 26
        {{6, 3}, {7, 3}, {11, 0}, {0}, {0}},            // base cell 27
        {{3, 5}, {4, 0}, {0}, {0}, {0}},                // base cell 28
        {{2, 5}, {3, 0}, {8, 3}, {0}, {0}},             // base cell 29
        {{0, 0}, {5, 3}, {0}, {0}, {0}},                // base cell 30
        {{0, 1}, {3, 5}, {4, 0}, {9, 3}, {0}},          // base cell 31
        {{0, 3}, {5, 0}, {10, 3}, {0}, {0}},            // base cell 32
        {{0, 0}, {4, 5}, {5, 3}, {0}, {0}},             // base cell 33
        {{2, 3}, {7, 0}, {12, 3}, {0}, {0}},            // base cell 34
        {{6, 3}, {11, 0}, {0}, {0}, {0}},               // base cell 35
        {{2, 3}, {7, 0}, {11, 3}, {12, 3}, {0}},        // base cell 36
        {{5, 3}, {6, 3}, {10, 0}, {0}, {0}},            // base cell 37
        {{12, 0}, {3, 1}, {8, 4}, {2, 0}, {7, 3}},      // base cell 38
        {{6, 0}, {10, 3}, {0}, {0}, {0}},               // base cell 39
        {{7, 0}, {11, 3}, {0}, {0}, {0}},               // base cell 40
        {{0, 1}, {4, 0}, {9, 3}, {0}, {0}},             // base cell 41
        {{3, 0}, {4, 1}, {8, 3}, {0}, {0}},             // base cell 42
        {{3, 0}, {8, 3}, {0}, {0}, {0}},                // base cell 43
        {{3, 5}, {4, 0}, {9, 3}, {0}, {0}},             // base cell 44
        {{6, 0}, {10, 3}, {11, 3}, {0}, {0}},           // base cell 45
        {{6, 3}, {7, 3}, {11, 0}, {16, 3}, {0}},        // base cell 46
        {{3, 3}, {8, 0}, {12, 3}, {0}, {0}},            // base cell 47
        {{0, 3}, {5, 0}, {14, 3}, {0}, {0}},            // base cell 48
        {{4, 0}, {9, 3}, {14, 0}, {0, 1}, {5, 4}},      // base cell 49
        {{0, 3}, {5, 0}, {10, 3}, {14, 3}, {0}},        // base cell 50
        {{7, 3}, {8, 3}, {12, 0}, {0}, {0}},            // base cell 51
        {{5, 3}, {10, 0}, {0}, {0}, {0}},               // base cell 52
        {{4, 0}, {9, 3}, {0}, {0}, {0}},                // base cell 53
        {{7, 3}, {12, 0}, {0}, {0}, {0}},               // base cell 54
        {{7, 0}, {11, 3}, {12, 3}, {0}, {0}},           // base cell 55
        {{6, 3}, {11, 0}, {16, 3}, {0}, {0}},           // base cell 56
        {{5, 1}, {6, 3}, {10, 0}, {15, 3}, {0}},        // base cell 57
        {{4, 1}, {9, 4}, {3, 0}, {8, 3}, {13, 0}},      // base cell 58
        {{6, 3}, {10, 0}, {15, 3}, {0}, {0}},           // base cell 59
        {{7, 3}, {11, 0}, {16, 3}, {0}, {0}},           // base cell 60
        {{4, 3}, {9, 0}, {14, 3}, {0}, {0}},            // base cell 61
        {{3, 3}, {8, 0}, {13, 3}, {0}, {0}},            // base cell 62
        {{16, 0}, {11, 3}, {6, 0}, {15, 1}, {10, 4}},   // base cell 63
        {{3, 3}, {8, 0}, {12, 3}, {13, 3}, {0}},        // base cell 64
        {{4, 3}, {9, 0}, {13, 3}, {0}, {0}},            // base cell 65
        {{5, 3}, {9, 3}, {14, 0}, {0}, {0}},            // base cell 66
        {{5, 0}, {14, 3}, {0}, {0}, {0}},               // base cell 67
        {{11, 3}, {16, 0}, {0}, {0}, {0}},              // base cell 68
        {{8, 0}, {12, 3}, {0}, {0}, {0}},               // base cell 69
        {{5, 0}, {10, 3}, {14, 3}, {0}, {0}},           // base cell 70
        {{7, 3}, {8, 3}, {12, 0}, {17, 3}, {0}},        // base cell 71
        {{16, 1}, {11, 4}, {17, 0}, {12, 3}, {7, 0}},   // base cell 72
        {{7, 3}, {12, 0}, {17, 3}, {0}, {0}},           // base cell 73
        {{5, 3}, {10, 0}, {15, 3}, {0}, {0}},           // base cell 74
        {{4, 3}, {9, 0}, {13, 3}, {14, 3}, {0}},        // base cell 75
        {{8, 3}, {9, 3}, {13, 0}, {0}, {0}},            // base cell 76
        {{11, 3}, {15, 1}, {16, 0}, {0}, {0}},          // base cell 77
        {{10, 3}, {15, 0}, {0}, {0}, {0}},              // base cell 78
        {{10, 3}, {15, 0}, {16, 5}, {0}, {0}},          // base cell 79
        {{11, 3}, {16, 0}, {17, 5}, {0}, {0}},          // base cell 80
        {{9, 3}, {14, 0}, {0}, {0}, {0}},               // base cell 81
        {{8, 3}, {13, 0}, {0}, {0}, {0}},               // base cell 82
        {{10, 3}, {5, 0}, {19, 1}, {14, 4}, {15, 0}},   // base cell 83
        {{8, 0}, {12, 3}, {13, 3}, {0}, {0}},           // base cell 84
        {{5, 3}, {9, 3}, {14, 0}, {19, 3}, {0}},        // base cell 85
        {{9, 0}, {13, 3}, {0}, {0}, {0}},               // base cell 86
        {{5, 3}, {14, 0}, {19, 3}, {0}, {0}},           // base cell 87
        {{12, 3}, {16, 1}, {17, 0}, {0}, {0}},          // base cell 88
        {{8, 3}, {12, 0}, {17, 3}, {0}, {0}},           // base cell 89
        {{11, 3}, {15, 1}, {16, 0}, {17, 5}, {0}},      // base cell 90
        {{12, 3}, {17, 0}, {0}, {0}, {0}},              // base cell 91
        {{10, 3}, {15, 0}, {19, 1}, {0}, {0}},          // base cell 92
        {{15, 1}, {16, 0}, {0}, {0}, {0}},              // base cell 93
        {{9, 0}, {13, 3}, {14, 3}, {0}, {0}},           // base cell 94
        {{10, 3}, {15, 0}, {16, 5}, {19, 1}, {0}},      // base cell 95
        {{8, 3}, {9, 3}, {13, 0}, {18, 3}, {0}},        // base cell 96
        {{13, 3}, {8, 0}, {17, 1}, {12, 4}, {18, 0}},   // base cell 97
        {{8, 3}, {13, 0}, {18, 3}, {0}, {0}},           // base cell 98
        {{16, 1}, {17, 0}, {0}, {0}, {0}},              // base cell 99
        {{14, 3}, {15, 5}, {19, 0}, {0}, {0}},          // base cell 100
        {{9, 3}, {14, 0}, {19, 3}, {0}, {0}},           // base cell 101
        {{14, 3}, {19, 0}, {0}, {0}, {0}},              // base cell 102
        {{12, 3}, {17, 0}, {18, 5}, {0}, {0}},          // base cell 103
        {{9, 3}, {13, 0}, {18, 3}, {0}, {0}},           // base cell 104
        {{12, 3}, {16, 1}, {17, 0}, {18, 5}, {0}},      // base cell 105
        {{15, 1}, {16, 0}, {17, 5}, {0}, {0}},          // base cell 106
        {{18, 1}, {13, 4}, {19, 0}, {14, 3}, {9, 0}},   // base cell 107
        {{15, 0}, {19, 1}, {0}, {0}, {0}},              // base cell 108
        {{15, 0}, {16, 5}, {19, 1}, {0}, {0}},          // base cell 109
        {{13, 3}, {18, 0}, {0}, {0}, {0}},              // base cell 110
        {{13, 3}, {17, 1}, {18, 0}, {0}, {0}},          // base cell 111
        {{14, 3}, {18, 1}, {19, 0}, {0}, {0}},          // base cell 112
        {{16, 1}, {17, 0}, {18, 5}, {0}, {0}},          // base cell 113
        {{14, 3}, {15, 5}, {18, 1}, {19, 0}, {0}},      // base cell 114
        {{13, 3}, {18, 0}, {19, 5}, {0}, {0}},          // base cell 115
        {{17, 1}, {18, 0}, {0}, {0}, {0}},              // base cell 116
        {{15, 5}, {19, 0}, {17, 3}, {18, 2}, {16, 4}},  // base cell 117
        {{15, 5}, {18, 1}, {19, 0}, {0}, {0}},          // base cell 118
        {{13, 3}, {17, 1}, {18, 0}, {19, 5}, {0}},      // base cell 119
        {{18, 1}, {19, 0}, {0}, {0}, {0}},              // base cell 120
        {{17, 1}, {18, 0}, {19, 5}, {0}, {0}}           // base cell 121
};

/**
 * Get the number of CW rotations of the cell's vertex numbers
 * compared to the directional layout of its neighbors.
 * @return Number of CCW rotations for the base cell, or -1 if the base cell
 *         was not found on this face
 */
int vertexRotations(H3Index cell) {
    // Get the face and other info for the origin
    FaceIJK fijk;
    _h3ToFaceIjk(cell, &fijk);
    int baseCell = H3_EXPORT(h3GetBaseCell)(cell);
    int cellLeadingDigit = _h3LeadingNonZeroDigit(cell);
    int mayCrossDeletedSubsequence =
        _isBaseCellPentagon(baseCell) && cellLeadingDigit == JK_AXES_DIGIT;

    FaceIJK baseFijk;
    _baseCellToFaceIjk(baseCell, &baseFijk);
    int hasPentCwRot = mayCrossDeletedSubsequence && fijk.face != baseFijk.face;
    for (int i = 0; i < MAX_BASE_CELL_FACES; i++) {
        BaseCellRotation rot = baseCellVertexRotations[baseCell][i];
        if (rot.face == fijk.face) {
            int ccwRot60 = rot.ccwRot60;
            if (hasPentCwRot) {
                return ccwRot60 == 0 ? 5 : ccwRot60 - 1;
            }
            return ccwRot60;
        }
    }
    // Failure case, should not be reachable
    return -1;
}

/** @brief Hexagon direction to vertex number relationships (same face).
 *         Note that we don't use direction 0 (center).
 */
static const int directionToVertexHex[NUM_HEX_VERTS + 1] = {-1, 3, 1, 2,
                                                            5,  4, 0};

/** @brief Pentagon direction to vertex number relationships (same face).
 *         Note that we don't use directions 0 (center) or 1 (deleted K axis).
 */
static const int directionToVertexPent[NUM_PENT_VERTS + 2] = {-1, -1, 1, 2,
                                                              4,  3,  0};

/**
 * Get the first vertex number for a given direction. The neighbor in this
 * direction is located between this vertex number and the next number in
 * sequence.
 */
int vertexNumForDirection(const H3Index origin, const Direction direction) {
    // Determine the vertex number for the direction. If the origin and the base
    // cell are on the same face, we can use the constant relationships above;
    // if they are on different faces, we need to apply a rotation
    // TODO: Handle error if rotations < 0? Should be unreachable
    int rotations = vertexRotations(origin);
    // Find the appropriate vertex, rotating CCW if necessary
    return H3_EXPORT(h3IsPentagon)(origin)
               ? (directionToVertexPent[direction] + NUM_PENT_VERTS -
                  rotations) %
                     NUM_PENT_VERTS
               : (directionToVertexHex[direction] + NUM_HEX_VERTS - rotations) %
                     NUM_HEX_VERTS;
}
