/*
 * Copyright 2017-2018 Uber Technologies, Inc.
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

#include <stdio.h>

#include "algos.h"
#include "test.h"
#include "utility.h"

SUITE(cellsToLinkedMultiPolygon) {
    TEST(empty) {
        LinkedGeoPolygon polygon;

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(NULL, 0, &polygon));

        t_assert(countLinkedLoops(&polygon) == 0, "No loops added to polygon");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(singleHex) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x890dab6220bffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
        t_assert(countLinkedCoords(polygon.first) == 6,
                 "6 coords added to loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(invalid) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0xfffffffffffffff};
        int numHexes = ARRAY_SIZE(set);

        t_assert(H3_EXPORT(cellsToLinkedMultiPolygon)(
                     set, numHexes, &polygon) == E_CELL_INVALID,
                 "Invalid set fails");
    }

    TEST(contiguous2) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x8928308291bffff, 0x89283082957ffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
        t_assert(countLinkedCoords(polygon.first) == 10,
                 "All coords added to loop except 2 shared");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    // TODO: This test asserts incorrect behavior - we should be creating
    // multiple polygons, each with their own single loop. Update when the
    // algorithm is corrected.
    TEST(nonContiguous2) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x8928308291bffff, 0x89283082943ffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedPolygons(&polygon) == 2, "2 polygons added");
        t_assert(countLinkedLoops(&polygon) == 1,
                 "1 loop on the first polygon");
        t_assert(countLinkedCoords(polygon.first) == 6,
                 "All coords for one hex added to first loop");
        t_assert(countLinkedLoops(polygon.next) == 1,
                 "Loop count on second polygon correct");
        t_assert(countLinkedCoords(polygon.next->first) == 6,
                 "All coords for one hex added to second polygon");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(contiguous3) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x8928308288bffff, 0x892830828d7ffff,
                         0x8928308289bffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
        t_assert(countLinkedCoords(polygon.first) == 12,
                 "All coords added to loop except 6 shared");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(hole) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x892830828c7ffff, 0x892830828d7ffff,
                         0x8928308289bffff, 0x89283082813ffff,
                         0x8928308288fffff, 0x89283082883ffff};
        int numHexes = ARRAY_SIZE(set);

        H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon);

        t_assert(countLinkedLoops(&polygon) == 2, "2 loops added to polygon");
        t_assert(countLinkedCoords(polygon.first) == 6 * 3,
                 "All outer coords added to first loop");
        t_assert(countLinkedCoords(polygon.first->next) == 6,
                 "All inner coords added to second loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(pentagon) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x851c0003fffffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
        t_assert(countLinkedCoords(polygon.first) == 10,
                 "10 coords (distorted pentagon) added to loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(twoRing) {
        LinkedGeoPolygon polygon;
        // 2-ring, in order returned by k-ring algo
        H3Index set[] = {
            0x8930062838bffff, 0x8930062838fffff, 0x89300628383ffff,
            0x8930062839bffff, 0x893006283d7ffff, 0x893006283c7ffff,
            0x89300628313ffff, 0x89300628317ffff, 0x893006283bbffff,
            0x89300628387ffff, 0x89300628397ffff, 0x89300628393ffff,
            0x89300628067ffff, 0x8930062806fffff, 0x893006283d3ffff,
            0x893006283c3ffff, 0x893006283cfffff, 0x8930062831bffff,
            0x89300628303ffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
        t_assert(countLinkedCoords(polygon.first) == (6 * (2 * 2 + 1)),
                 "Expected number of coords added to loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(twoRingUnordered) {
        LinkedGeoPolygon polygon;
        // 2-ring in random order
        H3Index set[] = {
            0x89300628393ffff, 0x89300628383ffff, 0x89300628397ffff,
            0x89300628067ffff, 0x89300628387ffff, 0x893006283bbffff,
            0x89300628313ffff, 0x893006283cfffff, 0x89300628303ffff,
            0x89300628317ffff, 0x8930062839bffff, 0x8930062838bffff,
            0x8930062806fffff, 0x8930062838fffff, 0x893006283d3ffff,
            0x893006283c3ffff, 0x8930062831bffff, 0x893006283d7ffff,
            0x893006283c7ffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
        t_assert(countLinkedCoords(polygon.first) == (6 * (2 * 2 + 1)),
                 "Expected number of coords added to loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(nestedDonut) {
        LinkedGeoPolygon polygon;
        // hollow 1-ring + hollow 3-ring around the same hex
        H3Index set[] = {
            0x89283082813ffff, 0x8928308281bffff, 0x8928308280bffff,
            0x8928308280fffff, 0x89283082807ffff, 0x89283082817ffff,
            0x8928308289bffff, 0x892830828d7ffff, 0x892830828c3ffff,
            0x892830828cbffff, 0x89283082853ffff, 0x89283082843ffff,
            0x8928308284fffff, 0x8928308287bffff, 0x89283082863ffff,
            0x89283082867ffff, 0x8928308282bffff, 0x89283082823ffff,
            0x89283082837ffff, 0x892830828afffff, 0x892830828a3ffff,
            0x892830828b3ffff, 0x89283082887ffff, 0x89283082883ffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        // Note that the polygon order here is arbitrary, making this test
        // somewhat brittle, but it's difficult to assert correctness otherwise
        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 2,
                 "Loop count on first polygon correct");
        t_assert(countLinkedCoords(polygon.first) == 42,
                 "Got expected big outer loop");
        t_assert(countLinkedCoords(polygon.first->next) == 30,
                 "Got expected big inner loop");
        t_assert(countLinkedLoops(polygon.next) == 2,
                 "Loop count on second polygon correct");
        t_assert(countLinkedCoords(polygon.next->first) == 18,
                 "Got expected outer loop");
        t_assert(countLinkedCoords(polygon.next->first->next) == 6,
                 "Got expected inner loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(nestedDonutTransmeridian) {
        LinkedGeoPolygon polygon;
        // hollow 1-ring + hollow 3-ring around the hex at (0, -180)
        H3Index set[] = {
            0x897eb5722c7ffff, 0x897eb5722cfffff, 0x897eb572257ffff,
            0x897eb57220bffff, 0x897eb572203ffff, 0x897eb572213ffff,
            0x897eb57266fffff, 0x897eb5722d3ffff, 0x897eb5722dbffff,
            0x897eb573537ffff, 0x897eb573527ffff, 0x897eb57225bffff,
            0x897eb57224bffff, 0x897eb57224fffff, 0x897eb57227bffff,
            0x897eb572263ffff, 0x897eb572277ffff, 0x897eb57223bffff,
            0x897eb572233ffff, 0x897eb5722abffff, 0x897eb5722bbffff,
            0x897eb572287ffff, 0x897eb572283ffff, 0x897eb57229bffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        // Note that the polygon order here is arbitrary, making this test
        // somewhat brittle, but it's difficult to assert correctness otherwise
        t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
        t_assert(countLinkedLoops(&polygon) == 2,
                 "Loop count on first polygon correct");
        t_assert(countLinkedCoords(polygon.first) == 18,
                 "Got expected outer loop");
        t_assert(countLinkedCoords(polygon.first->next) == 6,
                 "Got expected inner loop");
        t_assert(countLinkedLoops(polygon.next) == 2,
                 "Loop count on second polygon correct");
        t_assert(countLinkedCoords(polygon.next->first) == 42,
                 "Got expected big outer loop");
        t_assert(countLinkedCoords(polygon.next->first->next) == 30,
                 "Got expected big inner loop");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(contiguous2distorted) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x894cc5365afffff, 0x894cc536537ffff};
        int numHexes = ARRAY_SIZE(set);

        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
        t_assert(countLinkedCoords(polygon.first) == 12,
                 "All coords added to loop except 2 shared");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(negativeHashedCoordinates) {
        LinkedGeoPolygon polygon;
        H3Index set[] = {0x88ad36c547fffff, 0x88ad36c467fffff};
        int numHexes = ARRAY_SIZE(set);
        t_assertSuccess(
            H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes, &polygon));

        t_assert(countLinkedPolygons(&polygon) == 2, "2 polygons added");
        t_assert(countLinkedLoops(&polygon) == 1,
                 "1 loop on the first polygon");
        t_assert(countLinkedCoords(polygon.first) == 6,
                 "All coords for one hex added to first loop");
        t_assert(countLinkedLoops(polygon.next) == 1,
                 "Loop count on second polygon correct");
        t_assert(countLinkedCoords(polygon.next->first) == 6,
                 "All coords for one hex added to second polygon");

        H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
    }

    TEST(specificLeak) {
        // Test for a case where a leak can occur, detected by fuzzer.
        // The leak detection part will be enforced here by valgrind.
        LinkedGeoPolygon polygon;
        H3Index set[] = {0xd60006d60000f100, 0x3c3c403c1300d668};
        int numHexes = ARRAY_SIZE(set);
        t_assert(H3_EXPORT(cellsToLinkedMultiPolygon)(set, numHexes,
                                                      &polygon) == E_FAILED,
                 "invalid cells fail");
    }

    TEST(gridDiskResolutions) {
        // This is a center-face base cell, no pentagon siblings
        H3Index baseCell = 0x8073fffffffffff;
        H3Index origin = baseCell;

        H3Index indexes[19] = {0};
        int numHexes = ARRAY_SIZE(indexes);

        for (int res = 1; res < 15; res++) {
            // Take the 2-disk of the center child at res
            t_assertSuccess(
                H3_EXPORT(cellToCenterChild)(baseCell, res, &origin));
            t_assertSuccess(H3_EXPORT(gridDisk)(origin, 2, indexes));

            // Test the polygon output
            LinkedGeoPolygon polygon;
            t_assertSuccess(H3_EXPORT(cellsToLinkedMultiPolygon)(
                indexes, numHexes, &polygon));
            t_assert(countLinkedPolygons(&polygon) == 1, "1 polygon added");
            t_assert(countLinkedLoops(&polygon) == 1,
                     "1 loop on the first polygon");
            t_assert(countLinkedCoords(polygon.first) == 30,
                     "All coords for all hexes added to first loop");

            H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
        }
    }

    TEST(gridDiskResolutionsPentagon) {
        // This is a pentagon base cell
        H3Index baseCell = 0x8031fffffffffff;
        H3Index origin = baseCell;

        H3Index diskIndexes[7] = {0};
        H3Index indexes[6] = {0};

        for (int res = 1; res < 15; res++) {
            // Take the 1-disk of the center child at res. Note: We can't take
            // the 2-disk here, as increased distortion around the pentagon will
            // still fail at res 1. TODO: Use a 2-ring, start at res 0
            // when output is correct.
            t_assertSuccess(
                H3_EXPORT(cellToCenterChild)(baseCell, res, &origin));
            t_assertSuccess(H3_EXPORT(gridDisk)(origin, 1, diskIndexes));

            int j = 0;
            for (int i = 0; i < 7; i++) {
                if (diskIndexes[i]) indexes[j++] = diskIndexes[i];
            }
            t_assert(j == 6, "Filled all 6 indexes");

            // Test the polygon output
            LinkedGeoPolygon polygon;
            t_assertSuccess(
                H3_EXPORT(cellsToLinkedMultiPolygon)(indexes, 6, &polygon));
            t_assert(countLinkedPolygons(&polygon) == 1, "1 polygon added");
            t_assert(countLinkedLoops(&polygon) == 1,
                     "1 loop on the first polygon");
            t_assert(countLinkedCoords(polygon.first) == 15,
                     "All coords for all hexes added to first loop");

            H3_EXPORT(destroyLinkedMultiPolygon)(&polygon);
        }
    }
}
