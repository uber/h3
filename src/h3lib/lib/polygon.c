/*
 * Copyright 2018 Uber Technologies, Inc.
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
/** @file polygon.c
 * @brief Polygon algorithms
 */

#include "polygon.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include "bbox.h"
#include "constants.h"
#include "geoCoord.h"
#include "h3api.h"
#include "linkedGeo.h"

// Define macros used in polygon algos for Geofence
#define TYPE Geofence
#define INIT_ITERATION INIT_ITERATION_GEOFENCE
#define ITERATE ITERATE_GEOFENCE
#define IS_EMPTY IS_EMPTY_GEOFENCE

#include "polygonAlgos.h"

/**
 * Create a bounding box from a GeoPolygon
 * @param polygon Input GeoPolygon
 * @param bboxes  Output bboxes, one for the outer loop and one for each hole
 */
void bboxesFromGeoPolygon(const GeoPolygon* polygon, BBox* bboxes) {
    bboxFromGeofence(&polygon->geofence, &bboxes[0]);
    for (int i = 0; i < polygon->numHoles; i++) {
        bboxFromGeofence(&polygon->holes[i], &bboxes[i + 1]);
    }
}

/**
 * pointInsidePolygon takes a given GeoPolygon data structure and
 * checks if it contains a given geo coordinate.
 *
 * @param geoPolygon The geofence and holes defining the relevant area
 * @param bboxes     The bboxes for the main geofence and each of its holes
 * @param coord      The coordinate to check
 * @return           Whether the point is contained
 */
bool pointInsidePolygon(const GeoPolygon* geoPolygon, const BBox* bboxes,
                        const GeoCoord* coord) {
    // Start with contains state of primary geofence
    bool contains =
        pointInsideGeofence(&(geoPolygon->geofence), &bboxes[0], coord);

    // If the point is contained in the primary geofence, but there are holes in
    // the geofence iterate through all holes and return false if the point is
    // contained in any hole
    if (contains && geoPolygon->numHoles > 0) {
        for (int i = 0; i < geoPolygon->numHoles; i++) {
            if (pointInsideGeofence(&(geoPolygon->holes[i]), &bboxes[i + 1],
                                    coord)) {
                return false;
            }
        }
    }

    return contains;
}

#undef TYPE
#undef IS_EMPTY
#undef INIT_ITERATION
#undef ITERATE

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

/**
 * Count the number of polygons containing a given loop.
 * TODO: It would be really nice to memoize this.
 * @param  loop         Loop to count containers for
 * @param  polygons     Polygons to test
 * @param  polygonCount Number of polygons in the test array
 * @return              Number of polygons containing the loop
 */
static int countContainers(LinkedGeoLoop* loop, LinkedGeoPolygon** polygons,
                           BBox** bboxes, int polygonCount) {
    int containerCount = 0;
    for (int i = 0; i < polygonCount; i++) {
        if (pointInsideLinkedGeoLoop(polygons[i]->first, bboxes[i],
                                     &loop->first->vertex)) {
            containerCount++;
        }
    }
    return containerCount;
}

/**
 * Find the polygon to which a given hole should be allocated. Note that this
 * function will return null if no parent is found.
 * @param  loop         Inner loop describing a hole
 * @param  polygon      Head of a linked list of polygons to check
 * @param  polygonCount Number of polygons to check
 * @return              Pointer to parent polygon
 */
static LinkedGeoPolygon* findPolygonForHole(LinkedGeoLoop* loop,
                                            LinkedGeoPolygon* polygon,
                                            BBox* bboxes, int polygonCount) {
    // Initialize arrays for candidate loops and their bounding boxes
    LinkedGeoPolygon** candidates =
        calloc(polygonCount, sizeof(LinkedGeoPolygon*));
    assert(candidates != NULL);
    BBox** candidateBBoxes = calloc(polygonCount, sizeof(BBox*));
    assert(candidateBBoxes != NULL);
    // Find all polygons that contain the loop
    int containerCount = 0;
    int index = 0;
    while (polygon) {
        // We are guaranteed not to overlap, so just test the first point
        if (pointInsideLinkedGeoLoop(polygon->first, &bboxes[index],
                                     &loop->first->vertex)) {
            candidates[containerCount] = polygon;
            candidateBBoxes[containerCount] = &bboxes[index];
            containerCount++;
        }
        polygon = polygon->next;
        index++;
    }
    // Set the initial return value to the first candidate
    LinkedGeoPolygon* parent = candidates[0];
    // If we have multiple candidates, they must be nested inside each other.
    // Find the innermost polygon by taking the candidate with the most
    // containers in the candidate list.
    if (containerCount > 1) {
        int max = -1;
        for (int i = 0; i < containerCount; i++) {
            int count = countContainers(candidates[i]->first, candidates,
                                        candidateBBoxes, containerCount);
            if (count > max) {
                parent = candidates[i];
                max = count;
            }
        }
    }
    free(candidates);
    free(candidateBBoxes);
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
 */
void normalizeMultiPolygon(LinkedGeoPolygon* root) {
    // We assume that the input is a single polygon with loops;
    // if not, don't touch it
    if (root->next) {
        return;
    }
    // Count loops
    int loopCount = countLinkedLoops(root);
    // Early exit if there's only one loop
    if (loopCount <= 1) {
        return;
    }
    // Create an array to hold all of the inner loops. Note that
    // this array will never be full, as there will always be fewer
    // inner loops than outer loops.
    LinkedGeoLoop** innerLoops = calloc(loopCount, sizeof(LinkedGeoLoop*));
    assert(innerLoops != NULL);
    // Create an array to hold the bounding boxes for the outer loops
    BBox* bboxes = calloc(loopCount, sizeof(BBox));
    assert(bboxes != NULL);
    // Get the first loop and unlink it from root
    LinkedGeoLoop* loop = root->first;
    initLinkedPolygon(root);
    // Set up iteration variables
    LinkedGeoPolygon* polygon = NULL;
    LinkedGeoLoop* next;
    int innerCount = 0;
    int outerCount = 0;
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
    // If no outer loops were found, the polygon is invalid. We've already
    // unlinked the root, so returning here will leave root in an empty
    // state - this is assumed better than an invalid state.
    if (outerCount > 0) {
        // Find polygon for each inner loop and assign the hole to it
        for (int i = 0; i < innerCount; i++) {
            polygon =
                findPolygonForHole(innerLoops[i], root, bboxes, outerCount);
            if (polygon) {
                addLinkedLoop(polygon, innerLoops[i]);
            }
            // TODO: Evasive action/assertion if no parents are found?
        }
    }
    // TODO: Free any unallocated inner loops? Only possible with invalid input
    // Free allocated memory
    free(innerLoops);
    free(bboxes);
}
