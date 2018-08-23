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

TEST(nestedDonut) {
    LinkedGeoPolygon polygon;
    // hollow 1-ring + hollow 3-ring around the same hex
    H3Index set[] = {
        0x89283082813ffffl, 0x8928308281bffffl, 0x8928308280bffffl,
        0x8928308280fffffl, 0x89283082807ffffl, 0x89283082817ffffl,
        0x8928308289bffffl, 0x892830828d7ffffl, 0x892830828c3ffffl,
        0x892830828cbffffl, 0x89283082853ffffl, 0x89283082843ffffl,
        0x8928308284fffffl, 0x8928308287bffffl, 0x89283082863ffffl,
        0x89283082867ffffl, 0x8928308282bffffl, 0x89283082823ffffl,
        0x89283082837ffffl, 0x892830828afffffl, 0x892830828a3ffffl,
        0x892830828b3ffffl, 0x89283082887ffffl, 0x89283082883ffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

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

    H3_EXPORT(destroyLinkedPolygon)(&polygon);
}

TEST(nestedDonutTransmeridian) {
    LinkedGeoPolygon polygon;
    // hollow 1-ring + hollow 3-ring around the hex at (0, -180)
    H3Index set[] = {
        0x897eb5722c7ffffl, 0x897eb5722cfffffl, 0x897eb572257ffffl,
        0x897eb57220bffffl, 0x897eb572203ffffl, 0x897eb572213ffffl,
        0x897eb57266fffffl, 0x897eb5722d3ffffl, 0x897eb5722dbffffl,
        0x897eb573537ffffl, 0x897eb573527ffffl, 0x897eb57225bffffl,
        0x897eb57224bffffl, 0x897eb57224fffffl, 0x897eb57227bffffl,
        0x897eb572263ffffl, 0x897eb572277ffffl, 0x897eb57223bffffl,
        0x897eb572233ffffl, 0x897eb5722abffffl, 0x897eb5722bbffffl,
        0x897eb572287ffffl, 0x897eb572283ffffl, 0x897eb57229bffffl};
    int numHexes = sizeof(set) / sizeof(set[0]);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, &polygon);

    printf("%d\n", countLinkedLoops(polygon.next));

    // Note that the polygon order here is arbitrary, making this test
    // somewhat brittle, but it's difficult to assert correctness otherwise
    t_assert(countLinkedPolygons(&polygon) == 2, "Polygon count correct");
    t_assert(countLinkedLoops(&polygon) == 2,
             "Loop count on first polygon correct");
    t_assert(countLinkedCoords(polygon.first) == 18, "Got expected outer loop");
    t_assert(countLinkedCoords(polygon.first->next) == 6,
             "Got expected inner loop");
    t_assert(countLinkedLoops(polygon.next) == 2,
             "Loop count on second polygon correct");
    t_assert(countLinkedCoords(polygon.next->first) == 42,
             "Got expected big outer loop");
    t_assert(countLinkedCoords(polygon.next->first->next) == 30,
             "Got expected big inner loop");

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
