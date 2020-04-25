/*
 * Copyright 2019 Uber Technologies, Inc.
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
 * @brief tests H3 index to local IJ and IJK+ grid functions using
 * tests over a large number of indexes.
 *
 *  usage: `testH3ToLocalIjExhaustive`
 */

#include <h3api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algos.h"
#include "baseCells.h"
#include "constants.h"
#include "h3Index.h"
#include "h3api.h"
#include "localij.h"
#include "test.h"
#include "utility.h"

static const int MAX_DISTANCES[] = {1, 2, 5, 12, 19, 26};

// The same traversal constants from algos.c (for hexRange) here reused as local
// IJ vectors.
static const CoordIJ DIRECTIONS[6] = {{0, 1},  {-1, 0}, {-1, -1},
                                      {0, -1}, {1, 0},  {1, 1}};

static const CoordIJ NEXT_RING_DIRECTION = {1, 0};

/**
 * Test that the local coordinates for an index map to itself.
 */
void localIjToH3_identity_assertions(H3Index h3) {
    CoordIJ ij;
    t_assert(H3_EXPORT(experimentalH3ToLocalIj)(h3, h3, &ij) == 0,
             "able to setup localIjToH3 test");

    H3Index retrieved;
    t_assert(H3_EXPORT(experimentalLocalIjToH3)(h3, &ij, &retrieved) == 0,
             "got an index back from localIjTOh3");
    t_assert(h3 == retrieved, "round trip through local IJ space works");
}

/**
 * Test that coordinates for an index match some simple rules about index
 * digits, when using the index as its own origin. That is, that the IJ
 * coordinates are in the coordinate space of the origin's base cell.
 */
void h3ToLocalIj_coordinates_assertions(H3Index h3) {
    int r = H3_GET_RESOLUTION(h3);

    CoordIJ ij;
    t_assert(H3_EXPORT(experimentalH3ToLocalIj)(h3, h3, &ij) == 0,
             "get ij for origin");
    CoordIJK ijk;
    ijToIjk(&ij, &ijk);
    if (r == 0) {
        t_assert(_ijkMatches(&ijk, &UNIT_VECS[0]) == 1, "res 0 cell at 0,0,0");
    } else if (r == 1) {
        t_assert(_ijkMatches(&ijk, &UNIT_VECS[H3_GET_INDEX_DIGIT(h3, 1)]) == 1,
                 "res 1 cell at expected coordinates");
    } else if (r == 2) {
        CoordIJK expected = UNIT_VECS[H3_GET_INDEX_DIGIT(h3, 1)];
        _downAp7r(&expected);
        _neighbor(&expected, H3_GET_INDEX_DIGIT(h3, 2));
        t_assert(_ijkMatches(&ijk, &expected) == 1,
                 "res 2 cell at expected coordinates");
    } else {
        t_assert(0, "resolution supported by test function (coordinates)");
    }
}

/**
 * Test the the immediate neighbors of an index are at the expected locations in
 * the local IJ coordinate space.
 */
void h3ToLocalIj_neighbors_assertions(H3Index h3) {
    CoordIJ origin = {0};
    t_assert(H3_EXPORT(experimentalH3ToLocalIj)(h3, h3, &origin) == 0,
             "got ij for origin");
    CoordIJK originIjk;
    ijToIjk(&origin, &originIjk);

    for (Direction d = K_AXES_DIGIT; d < INVALID_DIGIT; d++) {
        if (d == K_AXES_DIGIT && H3_EXPORT(h3IsPentagon)(h3)) {
            continue;
        }

        int rotations = 0;
        H3Index offset = h3NeighborRotations(h3, d, &rotations);

        CoordIJ ij = {0};
        t_assert(H3_EXPORT(experimentalH3ToLocalIj)(h3, offset, &ij) == 0,
                 "got ij for destination");
        CoordIJK ijk;
        ijToIjk(&ij, &ijk);
        CoordIJK invertedIjk = {0};
        _neighbor(&invertedIjk, d);
        for (int i = 0; i < 3; i++) {
            _ijkRotate60ccw(&invertedIjk);
        }
        _ijkAdd(&invertedIjk, &ijk, &ijk);
        _ijkNormalize(&ijk);

        t_assert(_ijkMatches(&ijk, &originIjk), "back to origin");
    }
}

/**
 * Test that the neighbors (k-ring), if they can be found in the local IJ
 * coordinate space, can be converted back to indexes.
 */
void localIjToH3_kRing_assertions(H3Index h3) {
    int r = H3_GET_RESOLUTION(h3);
    t_assert(r <= 5, "resolution supported by test function (kRing)");
    int maxK = MAX_DISTANCES[r];

    int sz = H3_EXPORT(maxKringSize)(maxK);
    H3Index *neighbors = calloc(sz, sizeof(H3Index));
    int *distances = calloc(sz, sizeof(int));

    H3_EXPORT(kRingDistances)(h3, maxK, neighbors, distances);

    for (int i = 0; i < sz; i++) {
        if (neighbors[i] == 0) {
            continue;
        }

        CoordIJ ij;
        // Don't consider indexes which we can't unfold in the first place
        if (H3_EXPORT(experimentalH3ToLocalIj)(h3, neighbors[i], &ij) == 0) {
            H3Index retrieved;
            t_assert(
                H3_EXPORT(experimentalLocalIjToH3)(h3, &ij, &retrieved) == 0,
                "retrieved index for unfolded coordinates");
            t_assert(retrieved == neighbors[i],
                     "round trip neighboring index matches expected");
        }
    }

    free(distances);
    free(neighbors);
}

void localIjToH3_traverse_assertions(H3Index h3) {
    int r = H3_GET_RESOLUTION(h3);
    t_assert(r <= 5, "resolution supported by test function (traverse)");
    int k = MAX_DISTANCES[r];

    CoordIJ ij;
    t_assert(H3_EXPORT(experimentalH3ToLocalIj)(h3, h3, &ij) == 0,
             "Got origin coordinates");

    // This logic is from hexRangeDistances.
    // 0 < ring <= k, current ring
    int ring = 1;
    // 0 <= direction < 6, current side of the ring
    int direction = 0;
    // 0 <= i < ring, current position on the side of the ring
    int i = 0;

    while (ring <= k) {
        if (direction == 0 && i == 0) {
            ij.i += NEXT_RING_DIRECTION.i;
            ij.j += NEXT_RING_DIRECTION.j;
        }

        ij.i += DIRECTIONS[direction].i;
        ij.j += DIRECTIONS[direction].j;

        H3Index testH3;

        int failed = H3_EXPORT(experimentalLocalIjToH3)(h3, &ij, &testH3);
        if (!failed) {
            t_assert(H3_EXPORT(h3IsValid)(testH3),
                     "test coordinates result in valid index");

            CoordIJ expectedIj;
            int reverseFailed =
                H3_EXPORT(experimentalH3ToLocalIj)(h3, testH3, &expectedIj);
            // If it doesn't give a coordinate for this origin,index pair that's
            // OK.
            if (!reverseFailed) {
                if (expectedIj.i != ij.i || expectedIj.j != ij.j) {
                    // Multiple coordinates for the same index can happen due to
                    // pentagon distortion. In that case, the other coordinates
                    // should also belong to the same index.
                    H3Index testTestH3;
                    t_assert(H3_EXPORT(experimentalLocalIjToH3)(
                                 h3, &expectedIj, &testTestH3) == 0,
                             "converted coordinates again");
                    t_assert(testH3 == testTestH3,
                             "index has normalizable coordinates in "
                             "local IJ");
                }
            }
        }

        i++;
        // Check if end of this side of the k-ring
        if (i == ring) {
            i = 0;
            direction++;
            // Check if end of this ring.
            if (direction == 6) {
                direction = 0;
                ring++;
            }
        }
    }
}

SUITE(h3ToLocalIj) {
    TEST(localIjToH3_identity) {
        iterateAllIndexesAtRes(0, localIjToH3_identity_assertions);
        iterateAllIndexesAtRes(1, localIjToH3_identity_assertions);
        iterateAllIndexesAtRes(2, localIjToH3_identity_assertions);
    }

    TEST(h3ToLocalIj_coordinates) {
        iterateAllIndexesAtRes(0, h3ToLocalIj_coordinates_assertions);
        iterateAllIndexesAtRes(1, h3ToLocalIj_coordinates_assertions);
        iterateAllIndexesAtRes(2, h3ToLocalIj_coordinates_assertions);
    }

    TEST(h3ToLocalIj_neighbors) {
        iterateAllIndexesAtRes(0, h3ToLocalIj_neighbors_assertions);
        iterateAllIndexesAtRes(1, h3ToLocalIj_neighbors_assertions);
        iterateAllIndexesAtRes(2, h3ToLocalIj_neighbors_assertions);
    }

    TEST(localIjToH3_kRing) {
        iterateAllIndexesAtRes(0, localIjToH3_kRing_assertions);
        iterateAllIndexesAtRes(1, localIjToH3_kRing_assertions);
        iterateAllIndexesAtRes(2, localIjToH3_kRing_assertions);
        // Don't iterate all of res 3, to save time
        iterateAllIndexesAtResPartial(3, localIjToH3_kRing_assertions, 27);
        // Further resolutions aren't tested to save time.
    }

    TEST(localIjToH3_traverse) {
        iterateAllIndexesAtRes(0, localIjToH3_traverse_assertions);
        iterateAllIndexesAtRes(1, localIjToH3_traverse_assertions);
        iterateAllIndexesAtRes(2, localIjToH3_traverse_assertions);
        // Don't iterate all of res 3, to save time
        iterateAllIndexesAtResPartial(3, localIjToH3_traverse_assertions, 27);
        // Further resolutions aren't tested to save time.
    }
}
