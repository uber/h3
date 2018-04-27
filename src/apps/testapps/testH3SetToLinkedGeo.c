/*
 * Copyright 2017 Uber Technologies, Inc.
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

H3Index* makeSet(char** hexes, int numHexes) {
    H3Index* set = calloc(numHexes, sizeof(H3Index));
    for (int i = 0; i < numHexes; i++) {
        set[i] = H3_EXPORT(stringToH3)(hexes[i]);
    }
    return set;
}

BEGIN_TESTS(h3SetToLinkedGeo);

TEST(empty) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    int numHexes = 0;
    H3Index* set = makeSet(NULL, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 0, "No loops added to polygon");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(singleHex) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    char* hexes[] = {"890dab6220bffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon->first) == 6, "6 coords added to loop");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(contiguous2) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    char* hexes[] = {"8928308291bffff", "89283082957ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon->first) == 10,
             "All coords added to loop except 2 shared");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

// TODO: This test asserts incorrect behavior - we should be creating multiple
// polygons, each with their own single loop. Update when the algorithm is
// corrected.
TEST(nonContiguous2) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    char* hexes[] = {"8928308291bffff", "89283082943ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 2, "2 loops added to polygon");
    t_assert(countLinkedCoords(polygon->first) == 6,
             "All coords for one hex added to first loop");
    t_assert(countLinkedCoords(polygon->first->next) == 6,
             "All coords for one hex added to second loop");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(contiguous3) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    char* hexes[] = {"8928308288bffff", "892830828d7ffff", "8928308289bffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon->first) == 12,
             "All coords added to loop except 6 shared");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(hole) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    char* hexes[] = {"892830828c7ffff", "892830828d7ffff", "8928308289bffff",
                     "89283082813ffff", "8928308288fffff", "89283082883ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 2, "2 loops added to polygon");
    t_assert(countLinkedCoords(polygon->first) == 6 * 3,
             "All outer coords added to first loop");
    t_assert(countLinkedCoords(polygon->first->next) == 6,
             "All inner coords added to second loop");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(pentagon) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    char* hexes[] = {"851c0003fffffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon->first) == 10,
             "10 coords (distorted pentagon) added to loop");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(2Ring) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    // 2-ring, in order returned by k-ring algo
    char* hexes[] = {"8930062838bffff", "8930062838fffff", "89300628383ffff",
                     "8930062839bffff", "893006283d7ffff", "893006283c7ffff",
                     "89300628313ffff", "89300628317ffff", "893006283bbffff",
                     "89300628387ffff", "89300628397ffff", "89300628393ffff",
                     "89300628067ffff", "8930062806fffff", "893006283d3ffff",
                     "893006283c3ffff", "893006283cfffff", "8930062831bffff",
                     "89300628303ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon->first) == (6 * (2 * 2 + 1)),
             "Expected number of coords added to loop");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(2RingUnordered) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    // 2-ring in random order
    char* hexes[] = {"89300628393ffff", "89300628383ffff", "89300628397ffff",
                     "89300628067ffff", "89300628387ffff", "893006283bbffff",
                     "89300628313ffff", "893006283cfffff", "89300628303ffff",
                     "89300628317ffff", "8930062839bffff", "8930062838bffff",
                     "8930062806fffff", "8930062838fffff", "893006283d3ffff",
                     "893006283c3ffff", "8930062831bffff", "893006283d7ffff",
                     "893006283c7ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon->first) == (6 * (2 * 2 + 1)),
             "Expected number of coords added to loop");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

TEST(contiguous2distorted) {
    LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
    char* hexes[] = {"894cc5365afffff", "894cc536537ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    H3_EXPORT(h3SetToLinkedGeo)(set, numHexes, polygon);

    t_assert(countLinkedLoops(polygon) == 1, "1 loop added to polygon");
    t_assert(countLinkedCoords(polygon->first) == 12,
             "All coords added to loop except 2 shared");

    H3_EXPORT(destroyLinkedPolygon)(polygon);
    free(set);
    free(polygon);
}

END_TESTS();
