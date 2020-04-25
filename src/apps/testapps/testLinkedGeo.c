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
#include <stdlib.h>

#include "geoCoord.h"
#include "h3api.h"
#include "linkedGeo.h"
#include "test.h"

// Fixtures
static GeoCoord vertex1;
static GeoCoord vertex2;
static GeoCoord vertex3;
static GeoCoord vertex4;

SUITE(linkedGeo) {
    setGeoDegs(&vertex1, 87.372002166, 166.160981117);
    setGeoDegs(&vertex2, 87.370101364, 166.160184306);
    setGeoDegs(&vertex3, 87.369088356, 166.196239997);
    setGeoDegs(&vertex4, 87.369975080, 166.233115768);

    TEST(createLinkedGeo) {
        LinkedGeoPolygon* polygon = calloc(1, sizeof(LinkedGeoPolygon));
        LinkedGeoLoop* loop;
        LinkedGeoCoord* coord;

        loop = addNewLinkedLoop(polygon);
        t_assert(loop != NULL, "Loop created");
        coord = addLinkedCoord(loop, &vertex1);
        t_assert(coord != NULL, "Coord created");
        coord = addLinkedCoord(loop, &vertex2);
        t_assert(coord != NULL, "Coord created");
        coord = addLinkedCoord(loop, &vertex3);
        t_assert(coord != NULL, "Coord created");

        loop = addNewLinkedLoop(polygon);
        t_assert(loop != NULL, "Loop createed");
        coord = addLinkedCoord(loop, &vertex2);
        t_assert(coord != NULL, "Coord created");
        coord = addLinkedCoord(loop, &vertex4);
        t_assert(coord != NULL, "Coord created");

        t_assert(countLinkedPolygons(polygon) == 1, "Polygon count correct");
        t_assert(countLinkedLoops(polygon) == 2, "Loop count correct");
        t_assert(countLinkedCoords(polygon->first) == 3,
                 "Coord count 1 correct");
        t_assert(countLinkedCoords(polygon->last) == 2,
                 "Coord count 2 correct");

        LinkedGeoPolygon* nextPolygon = addNewLinkedPolygon(polygon);
        t_assert(nextPolygon != NULL, "polygon created");

        t_assert(countLinkedPolygons(polygon) == 2, "Polygon count correct");

        H3_EXPORT(destroyLinkedPolygon)(polygon);
        free(polygon);
    }
}
