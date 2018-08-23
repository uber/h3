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

BEGIN_TESTS(h3SetToLinkedGeo);

TEST(empty) {
    LinkedGeoPolygon polygon;

    H3_EXPORT(h3SetToLinkedGeo)(NULL, 0, &polygon);

    t_assert(countLinkedLoops(&polygon) == 0, "No loops added to polygon");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(singleHex) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x890dab6220bffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon.first) == 6, "6 coords added to loop");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(contiguous2) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x8928308291bffffl, 0x89283082957ffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon.first) == 10,
             "All coords added to loop except 2 shared");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

// TODO: This test asserts incorrect behavior - we should be creating multiple
// polygons, each with their own single loop. Update when the algorithm is
// corrected.
TEST(nonContiguous2) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x8928308291bffffl, 0x89283082943ffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedPolygons(&polygon) == 2, "2 polygons added");
    t_assert(countLinkedLoops(&polygon) == 1, "1 loop on the first polygon");
    t_assert(countLinkedCoords(polygon.first) == 6,
             "All coords for one hex added to first loop");
    t_assert(countLinkedLoops(polygon.next) == 1,
             "Loop count on second polygon correct");
    t_assert(countLinkedCoords(polygon.next->first) == 6,
             "All coords for one hex added to second polygon");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(contiguous3) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x8928308288bffffl, 0x892830828d7ffffl,
                     0x8928308289bffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon.first) == 12,
             "All coords added to loop except 6 shared");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(hole) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x892830828c7ffffl, 0x892830828d7ffffl,
                     0x8928308289bffffl, 0x89283082813ffffl,
                     0x8928308288fffffl, 0x89283082883ffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 2, "2 loops added to polygon");
    t_assert(countLinkedCoords(polygon.first) == 6 * 3,
             "All outer coords added to first loop");
    t_assert(countLinkedCoords(polygon.first->next) == 6,
             "All inner coords added to second loop");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(pentagon) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x851c0003fffffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon.first) == 10,
             "10 coords (distorted pentagon) added to loop");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(2Ring) {
    LinkedGeoPolygon polygon;
    // 2-ring, in order returned by k-ring algo
    H3Index set[] = {0x8930062838bffffl, 0x8930062838fffffl, 0x89300628383ffffl,
                     0x8930062839bffffl, 0x893006283d7ffffl, 0x893006283c7ffffl,
                     0x89300628313ffffl, 0x89300628317ffffl, 0x893006283bbffffl,
                     0x89300628387ffffl, 0x89300628397ffffl, 0x89300628393ffffl,
                     0x89300628067ffffl, 0x8930062806fffffl, 0x893006283d3ffffl,
                     0x893006283c3ffffl, 0x893006283cfffffl, 0x8930062831bffffl,
                     0x89300628303ffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon.first) == (6 * (2 * 2 + 1)),
             "Expected number of coords added to loop");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(2RingUnordered) {
    LinkedGeoPolygon polygon;
    // 2-ring in random order
    H3Index set[] = {0x89300628393ffffl, 0x89300628383ffffl, 0x89300628397ffffl,
                     0x89300628067ffffl, 0x89300628387ffffl, 0x893006283bbffffl,
                     0x89300628313ffffl, 0x893006283cfffffl, 0x89300628303ffffl,
                     0x89300628317ffffl, 0x8930062839bffffl, 0x8930062838bffffl,
                     0x8930062806fffffl, 0x8930062838fffffl, 0x893006283d3ffffl,
                     0x893006283c3ffffl, 0x8930062831bffffl, 0x893006283d7ffffl,
                     0x893006283c7ffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon.first) == (6 * (2 * 2 + 1)),
             "Expected number of coords added to loop");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(contiguous2distorted) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x894cc5365afffffl, 0x894cc536537ffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedLoops(&polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon.first) == 12,
             "All coords added to loop except 2 shared");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(negativeHashedCoordinates) {
    LinkedGeoPolygon polygon;
    H3Index set[] = {0x88ad36c547fffffl, 0x88ad36c467fffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);
    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    t_assert(countLinkedPolygons(&polygon) == 2, "2 polygons added");
    t_assert(countLinkedLoops(&polygon) == 1, "1 loop on the first polygon");
    t_assert(countLinkedCoords(polygon.first) == 6,
             "All coords for one hex added to first loop");
    t_assert(countLinkedLoops(polygon.next) == 1,
             "Loop count on second polygon correct");
    t_assert(countLinkedCoords(polygon.next->first) == 6,
             "All coords for one hex added to second polygon");

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

END_TESTS();
