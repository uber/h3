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
/** @file linkedGeo.c
 * @brief   Linked data structure for geo data
 */

#include "linkedGeo.h"
#include <assert.h>
#include <stdlib.h>
#include "geoCoord.h"
#include "h3api.h"

/**
 * Initialize a linked polygon
 * @param  polygon Polygon to init
 */
void initLinkedPolygon(LinkedGeoPolygon* polygon) {
    polygon->first = NULL;
    polygon->last = NULL;
    polygon->next = NULL;
}

/**
 * Initialize a linked loop
 * @param  loop Loop to init
 */
void initLinkedLoop(LinkedGeoLoop* loop) {
    loop->first = NULL;
    loop->last = NULL;
    loop->next = NULL;
}

/**
 * Add a linked polygon to the current polygon
 * @param  polygon Polygon to add link to
 * @return         Pointer to new polygon
 */
LinkedGeoPolygon* addLinkedPolygon(LinkedGeoPolygon* polygon) {
    assert(polygon->next == NULL);
    LinkedGeoPolygon* next = calloc(1, sizeof(*next));
    assert(next != NULL);
    polygon->next = next;
    return next;
}

/**
 * Add a new linked loop to the current polygon
 * @param  polygon Polygon to add loop to
 * @return         Pointer to loop
 */
LinkedGeoLoop* addLinkedLoop(LinkedGeoPolygon* polygon) {
    LinkedGeoLoop* loop = calloc(1, sizeof(*loop));
    assert(loop != NULL);
    initLinkedLoop(loop);
    LinkedGeoLoop* last = polygon->last;
    if (last == NULL) {
        assert(polygon->first == NULL);
        polygon->first = loop;
        polygon->last = loop;
    } else {
        last->next = loop;
        polygon->last = loop;
    }
    return loop;
}

/**
 * Add a new linked coordinate to the current loop
 * @param  loop   Loop to add coordinate to
 * @param  vertex Coordinate to add
 * @return        Pointer to the coordinate
 */
LinkedGeoCoord* addLinkedCoord(LinkedGeoLoop* loop, const GeoCoord* vertex) {
    LinkedGeoCoord* coord = calloc(1, sizeof(*coord));
    assert(coord != NULL);
    coord->vertex = *vertex;
    coord->next = NULL;
    LinkedGeoCoord* last = loop->last;
    if (last == NULL) {
        assert(loop->first == NULL);
        loop->first = coord;
        loop->last = coord;
    } else {
        last->next = coord;
        loop->last = coord;
    }
    return coord;
}

/**
 * Free all allocated memory for a linked geo structure. The caller is
 * responsible for freeing memory allocated to input polygon struct.
 * @param polygon Pointer to the first polygon in the structure
 */
void H3_EXPORT(destroyLinkedPolygon)(LinkedGeoPolygon* polygon) {
    // flag to skip the input polygon
    bool skip = true;
    for (LinkedGeoPolygon *currentPolygon = polygon, *nextPolygon;
         currentPolygon != NULL; currentPolygon = nextPolygon) {
        for (LinkedGeoLoop *currentLoop = currentPolygon->first, *nextLoop;
             currentLoop != NULL; currentLoop = nextLoop) {
            for (LinkedGeoCoord *currentCoord = currentLoop->first, *nextCoord;
                 currentCoord != NULL; currentCoord = nextCoord) {
                nextCoord = currentCoord->next;
                free(currentCoord);
            }
            nextLoop = currentLoop->next;
            free(currentLoop);
        }
        nextPolygon = currentPolygon->next;
        if (skip) {
            // do not free the input polygon
            skip = false;
        } else {
            free(currentPolygon);
        }
    }
}

/**
 * Count the number of polygons in a linked list
 * @param  polygon Starting polygon
 * @return         Count
 */
int countLinkedPolygons(LinkedGeoPolygon* polygon) {
    int count = 0;
    while (polygon != NULL) {
        count++;
        polygon = polygon->next;
    }
    return count;
}

/**
 * Count the number of linked loops in a polygon
 * @param  polygon Polygon to count loops for
 * @return         Count
 */
int countLinkedLoops(LinkedGeoPolygon* polygon) {
    LinkedGeoLoop* loop = polygon->first;
    int count = 0;
    while (loop != NULL) {
        count++;
        loop = loop->next;
    }
    return count;
}

/**
 * Count the number of coordinates in a loop
 * @param  loop Loop to count coordinates for
 * @return      Count
 */
int countLinkedCoords(LinkedGeoLoop* loop) {
    LinkedGeoCoord* coord = loop->first;
    int count = 0;
    while (coord != NULL) {
        count++;
        coord = coord->next;
    }
    return count;
}
