/*
 * Copyright 2017-2018, 2020-2021 Uber Technologies, Inc.
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

#include "alloc.h"
#include "h3api.h"
#include "latLng.h"

/**
 * Add a linked polygon to the current polygon
 * @param  polygon Polygon to add link to
 * @return         Pointer to new polygon
 */
LinkedGeoPolygon *addNewLinkedPolygon(LinkedGeoPolygon *polygon) {
    assert(polygon->next == NULL);
    LinkedGeoPolygon *next = H3_MEMORY(calloc)(1, sizeof(*next));
    assert(next != NULL);
    polygon->next = next;
    return next;
}

/**
 * Add a new linked loop to the current polygon
 * @param  polygon Polygon to add loop to
 * @return         Pointer to loop
 */
LinkedGeoLoop *addNewLinkedLoop(LinkedGeoPolygon *polygon) {
    LinkedGeoLoop *loop = H3_MEMORY(calloc)(1, sizeof(*loop));
    assert(loop != NULL);
    return addLinkedLoop(polygon, loop);
}

/**
 * Add an existing linked loop to the current polygon
 * @param  polygon Polygon to add loop to
 * @return         Pointer to loop
 */
LinkedGeoLoop *addLinkedLoop(LinkedGeoPolygon *polygon, LinkedGeoLoop *loop) {
    LinkedGeoLoop *last = polygon->last;
    if (last == NULL) {
        assert(polygon->first == NULL);
        polygon->first = loop;
    } else {
        last->next = loop;
    }
    polygon->last = loop;
    return loop;
}

/**
 * Add a new linked coordinate to the current loop
 * @param  loop   Loop to add coordinate to
 * @param  vertex Coordinate to add
 * @return        Pointer to the coordinate
 */
LinkedLatLng *addLinkedCoord(LinkedGeoLoop *loop, const LatLng *vertex) {
    LinkedLatLng *coord = H3_MEMORY(malloc)(sizeof(*coord));
    assert(coord != NULL);
    *coord = (LinkedLatLng){.vertex = *vertex, .next = NULL};
    LinkedLatLng *last = loop->last;
    if (last == NULL) {
        assert(loop->first == NULL);
        loop->first = coord;
    } else {
        last->next = coord;
    }
    loop->last = coord;
    return coord;
}

/**
 * Free all allocated memory for a linked geo loop. The caller is
 * responsible for freeing memory allocated to input loop struct.
 * @param loop Loop to free
 */
void destroyLinkedGeoLoop(LinkedGeoLoop *loop) {
    LinkedLatLng *nextCoord;
    for (LinkedLatLng *currentCoord = loop->first; currentCoord != NULL;
         currentCoord = nextCoord) {
        nextCoord = currentCoord->next;
        H3_MEMORY(free)(currentCoord);
    }
}

/**
 * Free all allocated memory for a linked geo structure. The caller is
 * responsible for freeing memory allocated to input polygon struct.
 * @param polygon Pointer to the first polygon in the structure
 */
void H3_EXPORT(destroyLinkedMultiPolygon)(LinkedGeoPolygon *polygon) {
    // flag to skip the input polygon
    bool skip = true;
    LinkedGeoPolygon *nextPolygon;
    LinkedGeoLoop *nextLoop;
    for (LinkedGeoPolygon *currentPolygon = polygon; currentPolygon != NULL;
         currentPolygon = nextPolygon) {
        for (LinkedGeoLoop *currentLoop = currentPolygon->first;
             currentLoop != NULL; currentLoop = nextLoop) {
            destroyLinkedGeoLoop(currentLoop);
            nextLoop = currentLoop->next;
            H3_MEMORY(free)(currentLoop);
        }
        nextPolygon = currentPolygon->next;
        if (skip) {
            // do not free the input polygon
            skip = false;
        } else {
            H3_MEMORY(free)(currentPolygon);
        }
    }
}

/**
 * Count the number of polygons in a linked list
 * @param  polygon Starting polygon
 * @return         Count
 */
int countLinkedPolygons(LinkedGeoPolygon *polygon) {
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
int countLinkedLoops(LinkedGeoPolygon *polygon) {
    LinkedGeoLoop *loop = polygon->first;
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
int countLinkedCoords(LinkedGeoLoop *loop) {
    LinkedLatLng *coord = loop->first;
    int count = 0;
    while (coord != NULL) {
        count++;
        coord = coord->next;
    }
    return count;
}

/**
 * Count the number of polygons containing a given loop.
 * @param  loop         Loop to count containers for
 * @param  polygons     Polygons to test
 * @param  bboxes       Bounding boxes for polygons, used in point-in-poly check
 * @param  polygonCount Number of polygons in the test array
 * @return              Number of polygons containing the loop
 */
static int countContainers(const LinkedGeoLoop *loop,
                           const LinkedGeoPolygon **polygons,
                           const BBox **bboxes, const int polygonCount) {
    int containerCount = 0;
    for (int i = 0; i < polygonCount; i++) {
        if (loop != polygons[i]->first &&
            pointInsideLinkedGeoLoop(polygons[i]->first, bboxes[i],
                                     &loop->first->vertex)) {
            containerCount++;
        }
    }
    return containerCount;
}

/**
 * Given a list of nested containers, find the one most deeply nested.
 * @param  polygons     Polygon containers to check
 * @param  bboxes       Bounding boxes for polygons, used in point-in-poly check
 * @param  polygonCount Number of polygons in the list
 * @return              Deepest container, or null if list is empty
 */
static const LinkedGeoPolygon *findDeepestContainer(
    const LinkedGeoPolygon **polygons, const BBox **bboxes,
    const int polygonCount) {
    // Set the initial return value to the first candidate
    const LinkedGeoPolygon *parent = polygonCount > 0 ? polygons[0] : NULL;

    // If we have multiple polygons, they must be nested inside each other.
    // Find the innermost polygon by taking the one with the most containers
    // in the list.
    if (polygonCount > 1) {
        int max = -1;
        for (int i = 0; i < polygonCount; i++) {
            int count = countContainers(polygons[i]->first, polygons, bboxes,
                                        polygonCount);
            if (count > max) {
                parent = polygons[i];
                max = count;
            }
        }
    }

    return parent;
}

/**
 * Find the polygon to which a given hole should be allocated. Note that this
 * function will return null if no parent is found.
 * @param  loop         Inner loop describing a hole
 * @param  polygon      Head of a linked list of polygons to check
 * @param  bboxes       Bounding boxes for polygons, used in point-in-poly check
 * @param  polygonCount Number of polygons to check
 * @return              Pointer to parent polygon, or null if not found
 */
static const LinkedGeoPolygon *findPolygonForHole(
    const LinkedGeoLoop *loop, const LinkedGeoPolygon *polygon,
    const BBox *bboxes, const int polygonCount) {
    // Early exit with no polygons
    if (polygonCount == 0) {
        return NULL;
    }
    // Initialize arrays for candidate loops and their bounding boxes
    const LinkedGeoPolygon **candidates =
        H3_MEMORY(malloc)(polygonCount * sizeof(LinkedGeoPolygon *));
    assert(candidates != NULL);
    const BBox **candidateBBoxes =
        H3_MEMORY(malloc)(polygonCount * sizeof(BBox *));
    assert(candidateBBoxes != NULL);

    // Find all polygons that contain the loop
    int candidateCount = 0;
    int index = 0;
    while (polygon) {
        // We are guaranteed not to overlap, so just test the first point
        if (pointInsideLinkedGeoLoop(polygon->first, &bboxes[index],
                                     &loop->first->vertex)) {
            candidates[candidateCount] = polygon;
            candidateBBoxes[candidateCount] = &bboxes[index];
            candidateCount++;
        }
        polygon = polygon->next;
        index++;
    }

    // The most deeply nested container is the immediate parent
    const LinkedGeoPolygon *parent =
        findDeepestContainer(candidates, candidateBBoxes, candidateCount);

    // Free allocated memory
    H3_MEMORY(free)(candidates);
    H3_MEMORY(free)(candidateBBoxes);

    return parent;
}

/**
 * Normalize a LinkedGeoPolygon in-place into a structure following GeoJSON
 * MultiPolygon rules: Each polygon must have exactly one outer loop, which
 * must be first in the list, followed by any holes. Holes in this algorithm
 * are identified by winding order (holes are clockwise), which is guaranteed
 * by the h3SetToVertexGraph algorithm.
 *
 * Input to this function is assumed to be a single polygon including all
 * loops to normalize. It's assumed that a valid arrangement is possible.
 *
 * @param root Root polygon including all loops
 * @return     0 on success, or an error code > 0 for invalid input
 */
int normalizeMultiPolygon(LinkedGeoPolygon *root) {
    // We assume that the input is a single polygon with loops;
    // if it has multiple polygons, don't touch it
    if (root->next) {
        return NORMALIZATION_ERR_MULTIPLE_POLYGONS;
    }

    // Count loops, exiting early if there's only one
    int loopCount = countLinkedLoops(root);
    if (loopCount <= 1) {
        return NORMALIZATION_SUCCESS;
    }

    int resultCode = NORMALIZATION_SUCCESS;
    LinkedGeoPolygon *polygon = NULL;
    LinkedGeoLoop *next;
    int innerCount = 0;
    int outerCount = 0;

    // Create an array to hold all of the inner loops. Note that
    // this array will never be full, as there will always be fewer
    // inner loops than outer loops.
    LinkedGeoLoop **innerLoops =
        H3_MEMORY(malloc)(loopCount * sizeof(LinkedGeoLoop *));
    assert(innerLoops != NULL);

    // Create an array to hold the bounding boxes for the outer loops
    BBox *bboxes = H3_MEMORY(malloc)(loopCount * sizeof(BBox));
    assert(bboxes != NULL);

    // Get the first loop and unlink it from root
    LinkedGeoLoop *loop = root->first;
    *root = (LinkedGeoPolygon){0};

    // Iterate over all loops, moving inner loops into an array and
    // assigning outer loops to new polygons
    while (loop) {
        if (isClockwiseLinkedGeoLoop(loop)) {
            innerLoops[innerCount] = loop;
            innerCount++;
        } else {
            polygon = polygon == NULL ? root : addNewLinkedPolygon(polygon);
            addLinkedLoop(polygon, loop);
            bboxFromLinkedGeoLoop(loop, &bboxes[outerCount]);
            outerCount++;
        }
        // get the next loop and unlink it from this one
        next = loop->next;
        loop->next = NULL;
        loop = next;
    }

    // Find polygon for each inner loop and assign the hole to it
    for (int i = 0; i < innerCount; i++) {
        polygon = (LinkedGeoPolygon *)findPolygonForHole(innerLoops[i], root,
                                                         bboxes, outerCount);
        if (polygon) {
            addLinkedLoop(polygon, innerLoops[i]);
        } else {
            // If we can't find a polygon (possible with invalid input), then
            // we need to release the memory for the hole, because the loop has
            // been unlinked from the root and the caller will no longer have
            // a way to destroy it with destroyLinkedMultiPolygon.
            destroyLinkedGeoLoop(innerLoops[i]);
            H3_MEMORY(free)(innerLoops[i]);
            resultCode = NORMALIZATION_ERR_UNASSIGNED_HOLES;
        }
    }

    // Free allocated memory
    H3_MEMORY(free)(innerLoops);
    H3_MEMORY(free)(bboxes);

    return resultCode;
}

// Define macros used in polygon algos for LinkedGeoLoop
#define TYPE LinkedGeoLoop
#define INIT_ITERATION INIT_ITERATION_LINKED_LOOP
#define ITERATE ITERATE_LINKED_LOOP
#define IS_EMPTY IS_EMPTY_LINKED_LOOP

#include "polygonAlgos.h"

#undef TYPE
#undef IS_EMPTY
#undef INIT_ITERATION
#undef ITERATE
