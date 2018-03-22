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
/** @file generateBaseCellNeighbors.c
 * @brief Generates the baseCellNeighbors and baseCellRotations tables
 *
 *  usage: `generateBaseCellNeighbors`
 *
 *  The program generates a table of neighbors of base cells, and the
 *  60 degree ccw rotations into the coordinate system of that base cell.
 *
 *  INVALID_BASE_CELL is generated for neighbors that are deleted (the deleted
 *  neighbor of a pentagon.)
 */

#include <baseCells.h>
#include <stdlib.h>
#include <time.h>

#ifdef H3_HAVE_REENTRANT_GMTIME
#ifdef _MSC_VER
#define reentrant_gmtime(tmStruct, epochTime) \
    do {                                      \
        gmtime_s((tmStruct), (epochTime));    \
    } while (0)
#else
#define reentrant_gmtime(tmStruct, epochTime) \
    do {                                      \
        gmtime_r((epochTime), (tmStruct));    \
    } while (0)
#endif
#else
#include <string.h>
/* Fallback on non-reentrant when nothing available. */
#define reentrant_gmtime(tmStruct, epochTime)                        \
    do {                                                             \
        memcpy(*(tmStruct), sizeof(*(tmStruct)), gmtime(epochTime)); \
    } while (0)
#endif

const int NUM_DIRS = 6;

/**
 * Performs some tests on the generated table to try to ensure correctness.
 *
 * @param baseCellNeighbors
 * @param baseCellRotations
 */
static void auditBaseCellNeighbors(int baseCellNeighbors[NUM_BASE_CELLS][7],
                                   int baseCellRotations[NUM_BASE_CELLS][7]) {
    for (int i = 0; i < NUM_BASE_CELLS; i++) {
        for (int j = 0; j <= NUM_DIRS; j++) {
            if (baseCellNeighbors[i][j] == INVALID_BASE_CELL) continue;

            CoordIJK ourDir = {0, 0, 0};
            _neighbor(&ourDir, j);

            int k = 0;
            for (; k <= NUM_DIRS; k++) {
                if (baseCellNeighbors[baseCellNeighbors[i][j]][k] == i) {
                    break;
                }
            }
            if (k == NUM_DIRS + 1) {
                printf("MISMATCH between %d and %d\n", i,
                       baseCellNeighbors[i][j]);
            }
            CoordIJK theirDir = {0, 0, 0};
            _neighbor(&theirDir, k);

            for (int reverse = 0; reverse < 3; reverse++) {
                _ijkRotate60ccw(&ourDir);
            }
            for (int rotate = 0; rotate < baseCellRotations[i][j]; rotate++) {
                _ijkRotate60ccw(&ourDir);
            }

            /* This is wrong for moving into pentagons. One neighbor for most
               pentagons, and four neighbors for the polar pentagons 4 and
               117. */
            if (!_isBaseCellPentagon(baseCellNeighbors[i][j])) {
                if (ourDir.i != theirDir.i || ourDir.j != theirDir.j ||
                    ourDir.k != theirDir.k) {
                    printf("WRONG DIRECTION between %d and %d\n", i,
                           baseCellNeighbors[i][j]);
                }
            }
        }
    }
}

/**
 * Generates and prints the baseCellNeighbors and baseCellRotations tables.
 */
static void generate() {
    int baseCellNeighbors[NUM_BASE_CELLS][7];
    int baseCellRotations[NUM_BASE_CELLS][7];

    for (int i = 0; i < NUM_BASE_CELLS; i++) {
        if (!_isBaseCellPentagon(i)) {
            for (int dir = 0; dir <= NUM_DIRS; dir++) {
                FaceIJK fijk;
                _baseCellToFaceIjk(i, &fijk);
                _neighbor(&fijk.coord, dir);

                /* Should never happen, but just in case :) */
                if (fijk.coord.i < 3 && fijk.coord.j < 3 && fijk.coord.k < 3) {
                    baseCellNeighbors[i][dir] = _faceIjkToBaseCell(&fijk);
                    baseCellRotations[i][dir] =
                        _faceIjkToBaseCellCCWrot60(&fijk);
                } else {
                    printf("UH OH: Went out of bounds\n");
                }
            }
        } else {
            baseCellNeighbors[i][0] = i;
            baseCellRotations[i][0] = 0;

            for (int dir = 1; dir <= NUM_DIRS; dir++) {
                baseCellNeighbors[i][dir] = INVALID_BASE_CELL;
                baseCellRotations[i][dir] = -1;
            }

            for (int f = 0; f < NUM_ICOSA_FACES; f++) {
                for (int axis = 0; axis < 3; axis++) {
                    FaceIJK fijk = {f, {0, 0, 0}};
                    switch (axis) {
                        case 0:
                            fijk.coord.k = 2;
                            break;
                        case 1:
                            fijk.coord.j = 2;
                            break;
                        case 2:
                            fijk.coord.i = 2;
                            break;
                    }

                    /* Determine if we found a face that can traverse to the
                       pentagon */
                    if (_faceIjkToBaseCell(&fijk) == i) {
                        /* fijk of the neighboring base cell */
                        FaceIJK neighborFijk = {
                            fijk.face,
                            {fijk.coord.i / 2, fijk.coord.j / 2,
                             fijk.coord.k / 2}};

                        /* number of rotations from the neighboring base cell
                           into the pentagon */
                        int rotations = _faceIjkToBaseCellCCWrot60(&fijk);

                        /* direction from the neighboring base cell to the
                           pentagon */
                        CoordIJK ijk = neighborFijk.coord;
                        /* turn that into the direction within the pentagon
                           (direction for continuing straight inside the
                           pentagon) */
                        for (int currRot = 0; currRot < rotations; currRot++) {
                            _ijkRotate60ccw(&ijk);
                        }
                        /* invert that */
                        for (int currRot = 0; currRot < 3; currRot++) {
                            _ijkRotate60ccw(&ijk);
                        }
                        /* direction from the pentagon towards the neighboring
                           base cell */
                        int dir = _unitIjkToDigit(&ijk);

                        /* the direction was detected as being the i direction,
                           but this can't be because i is deleted from the
                           pentagon. We need to choose a different direction. */
                        if (dir == 1) {
                            /* 4 and 117 are 'polar' type pentagons, which have
                               some different behavior. */
                            if (i == 4 || i == 117) {
                                _ijkRotate60cw(&ijk);
                                _ijkRotate60cw(&ijk);
                            } else {
                                _ijkRotate60ccw(&ijk);
                            }
                            dir = _unitIjkToDigit(&ijk);
                        }

                        /* Adjust for the deleted k-subsequence distortion */
                        int rotAdj = 0;
                        if (i == 4 || i == 117) {
                            /* 'polar' type pentagon with all faces pointing
                               towards i */
                            if (dir == 5) {
                                rotAdj = 2;
                            } else if (dir == 6) {
                                rotAdj = 4;
                            }
                        } else {
                            /* the deleted k subsequence causes 4 and 5 to
                               'warp', need to adjust for that. */
                            if (dir == 4 || dir == 5) {
                                rotAdj = dir;
                            }
                        }
                        rotations = (rotations + rotAdj) % 6;

                        int neighborBc = _faceIjkToBaseCell(&neighborFijk);

                        /* The poles are totally different, although the
                           rotations are correctly generated, so only overwrite
                           the neighbor information. It was easier to manually
                           derive the neighbors than to write the generation
                           program. */
                        if (i == 4) {
                            int realNeighbors[] = {
                                4, INVALID_BASE_CELL, 15, 8, 3, 0, 12};
                            neighborBc = realNeighbors[dir];
                        } else if (i == 117) {
                            int realNeighbors[] = {
                                117, INVALID_BASE_CELL, 109, 118, 113, 121,
                                106};
                            neighborBc = realNeighbors[dir];
                        }

                        /* the actual neighboring base cell */
                        baseCellNeighbors[i][dir] = neighborBc;
                        /* rotations from the pentagon into the neighboring base
                           cell */
                        baseCellRotations[i][dir] = rotations;
                    }
                }
            }
        }
    }

    auditBaseCellNeighbors(baseCellNeighbors, baseCellRotations);

    time_t rawTime;
    time(&rawTime);
    struct tm tmStruct;
    reentrant_gmtime(&tmStruct, &rawTime);
    const char prologFormat[] =
        "/*\n"
        " * Copyright 2016-%d Uber Technologies, Inc.\n"
        " *\n"
        " * Licensed under the Apache License, Version 2.0 (the \"License\");\n"
        " * you may not use this file except in compliance with the License.\n"
        " * You may obtain a copy of the License at\n"
        " *\n"
        " *         http://www.apache.org/licenses/LICENSE-2.0\n"
        " *\n"
        " * Unless required by applicable law or agreed to in writing, "
        "software\n"
        " * distributed under the License is distributed on an \"AS IS\" "
        "BASIS,\n"
        " * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or "
        "implied.\n"
        " * See the License for the specific language governing permissions "
        "and\n"
        " * limitations under the License.\n"
        " */\n"
        "/** @file baseCells.c\n"
        " * Base cell related lookup tables and access functions.\n"
        " */\n\n"
        "#include \"baseCells.h\"\n\n"
        "/** base cell at a given ijk and required rotations into its "
        "system */\n"
        "typedef struct {\n"
        "    int baseCell;  /** base cell number */\n"
        "    int ccwRot60;  /** number of ccw 60 degree rotations relative to "
        "current */\n"
        "} BaseCellOrient;\n\n"
        "/** @brief Neighboring base cell ID in each IJK direction.\n"
        " *\n"
        " * For each base cell, for each direction, the neighboring base\n"
        " * cell ID is given. 127 indicates there is no neighbor in that "
        "direction.\n"
        " */\n";
    printf(prologFormat, 1900 + tmStruct.tm_year);

    printf("const int baseCellNeighbors[NUM_BASE_CELLS][7] = {\n");
    for (int i = 0; i < NUM_BASE_CELLS; i++) {
        printf("    {");
        for (int j = 0; j < 7; j++) {
            if (j > 0) {
                printf(", ");
            }
            if (baseCellNeighbors[i][j] != INVALID_BASE_CELL) {
                printf("%d", baseCellNeighbors[i][j]);
            } else {
                printf("INVALID_BASE_CELL");
            }
        }
        printf("}, /* base cell %d%s */\n", i,
               _isBaseCellPentagon(i) ? " (pentagon)" : "");
    }
    printf("};\n");
    printf("\n");
    printf("const int baseCellNeighbor60CCWRots[NUM_BASE_CELLS][7] = {\n");
    for (int i = 0; i < NUM_BASE_CELLS; i++) {
        printf("    {%d, %d, %d, %d, %d, %d, %d}, /* base cell %d */%s\n",
               baseCellRotations[i][0], baseCellRotations[i][1],
               baseCellRotations[i][2], baseCellRotations[i][3],
               baseCellRotations[i][4], baseCellRotations[i][5],
               baseCellRotations[i][6], i,
               _isBaseCellPentagon(i) ? " (pentagon)" : "");
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
