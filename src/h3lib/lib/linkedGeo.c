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
int countLinkedPolygons(const LinkedGeoPolygon *polygon) {
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
int countLinkedLoops(const LinkedGeoPolygon *polygon) {
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
int countLinkedCoords(const LinkedGeoLoop *loop) {
    LinkedLatLng *coord = loop->first;
    int count = 0;
    while (coord != NULL) {
        count++;
        coord = coord->next;
    }
    return count;
}

/**
 * Convert a linked geo loop to a GeoLoop by counting coords and copying.
 * @param  linked  Source linked loop
 * @param  out     Output GeoLoop (verts will be allocated)
 * @return         E_SUCCESS, E_FAILED (< 3 verts), or E_MEMORY_ALLOC
 */
static H3Error linkedGeoLoopToGeoLoop(const LinkedGeoLoop *linked,
                                      GeoLoop *out) {
    int numVerts = countLinkedCoords(linked);
    if (numVerts < 3) {
        return E_FAILED;
    }
    LatLng *verts = H3_MEMORY(calloc)(numVerts, sizeof(LatLng));
    if (!verts) return E_MEMORY_ALLOC;

    LinkedLatLng *coord = linked->first;
    for (int i = 0; coord != NULL; i++) {
        verts[i] = coord->vertex;
        coord = coord->next;
    }
    out->numVerts = numVerts;
    out->verts = verts;
    return E_SUCCESS;
}

/**
 * Convert a single LinkedGeoPolygon (outer loop + holes) to a GeoPolygon.
 * The output's geoloop and holes are allocated; on failure, partially
 * filled fields are safe for destroyGeoPolygon (calloc-zeroed).
 * @param  linked  Source linked polygon
 * @param  out     Output GeoPolygon (caller-owned, will be populated)
 * @return         E_SUCCESS or E_MEMORY_ALLOC
 */
static H3Error linkedGeoPolygonToGeoPolygon(const LinkedGeoPolygon *linked,
                                            GeoPolygon *out) {
    // Convert outer loop
    const LinkedGeoLoop *firstLoop = linked->first;
    H3Error err = linkedGeoLoopToGeoLoop(firstLoop, &out->geoloop);
    if (err) return err;

    // Count and convert holes
    int numHoles = countLinkedLoops(linked) - 1;
    if (numHoles > 0) {
        GeoLoop *holes = H3_MEMORY(calloc)(numHoles, sizeof(GeoLoop));
        if (!holes) return E_MEMORY_ALLOC;
        out->holes = holes;
        out->numHoles = numHoles;

        LinkedGeoLoop *loop = firstLoop->next;
        for (int i = 0; loop != NULL; i++) {
            err = linkedGeoLoopToGeoLoop(loop, &holes[i]);
            if (err) return err;
            loop = loop->next;
        }
    }

    return E_SUCCESS;
}

/**
 * Convert a LinkedGeoPolygon to a GeoMultiPolygon.
 *
 * An empty chain (head node with no loops and no `next`) produces an empty
 * output (numPolygons=0) and returns E_SUCCESS. Every non-empty polygon
 * node must have an outer loop, and every loop must have >= 3 vertices;
 * otherwise, E_FAILED is returned.
 *
 * On error, any (partial) output is cleaned up via `destroyGeoMultiPolygon()`.
 * On success the caller owns the output and must free
 * it with `destroyGeoMultiPolygon()`.
 *
 * @param  linked  Head of linked polygon chain
 * @param  out     Output GeoMultiPolygon (caller-owned, will be populated)
 * @return         E_SUCCESS, E_FAILED (invalid geometry), or E_MEMORY_ALLOC
 */
H3Error linkedGeoPolygonToGeoMultiPolygon(const LinkedGeoPolygon *linked,
                                          GeoMultiPolygon *out) {
    out->numPolygons = 0;
    out->polygons = NULL;

    // Empty chain (head has no loops and no next) is valid: 0 polygons
    if (linked->first == NULL && linked->next == NULL) {
        return E_SUCCESS;
    }

    int numPolygons = countLinkedPolygons(linked);

    GeoPolygon *polygons = H3_MEMORY(calloc)(numPolygons, sizeof(GeoPolygon));
    if (!polygons) return E_MEMORY_ALLOC;

    out->polygons = polygons;
    out->numPolygons = numPolygons;

    const LinkedGeoPolygon *lpoly = linked;
    for (int i = 0; lpoly != NULL; i++) {
        if (lpoly->first == NULL) {
            H3_EXPORT(destroyGeoMultiPolygon)(out);
            return E_FAILED;
        }
        H3Error err = linkedGeoPolygonToGeoPolygon(lpoly, &polygons[i]);
        if (err) {
            H3_EXPORT(destroyGeoMultiPolygon)(out);
            return err;
        }
        lpoly = lpoly->next;
    }

    return E_SUCCESS;
}

/**
 * Populate a LinkedGeoLoop with vertices from a GeoLoop.
 * @param  src   Source GeoLoop
 * @param  loop  Target linked loop (must be calloc-zeroed)
 * @return       E_SUCCESS or E_MEMORY_ALLOC
 */
static H3Error geoLoopToLinkedGeoLoop(const GeoLoop *src, LinkedGeoLoop *loop) {
    if (src->numVerts < 3) return E_FAILED;
    for (int i = 0; i < src->numVerts; i++) {
        LinkedLatLng *coord = H3_MEMORY(malloc)(sizeof(LinkedLatLng));
        if (!coord) return E_MEMORY_ALLOC;

        *coord = (LinkedLatLng){.vertex = src->verts[i], .next = NULL};
        if (loop->last == NULL) {
            loop->first = coord;
        } else {
            loop->last->next = coord;
        }
        loop->last = coord;
    }
    return E_SUCCESS;
}

/**
 * Convert a single GeoPolygon to linked loops within a LinkedGeoPolygon.
 * @param  poly        Source GeoPolygon
 * @param  currentPoly Target LinkedGeoPolygon to populate
 * @return             E_SUCCESS or E_MEMORY_ALLOC
 */
static H3Error addLinkedGeoLoop(const GeoLoop *gl,
                                LinkedGeoPolygon *currentPoly) {
    LinkedGeoLoop *loop = H3_MEMORY(calloc)(1, sizeof(LinkedGeoLoop));
    if (!loop) return E_MEMORY_ALLOC;

    if (currentPoly->last) {
        currentPoly->last->next = loop;
    } else {
        currentPoly->first = loop;
    }
    currentPoly->last = loop;

    return geoLoopToLinkedGeoLoop(gl, loop);
}

static H3Error geoPolygonToLinkedGeoLoops(const GeoPolygon *poly,
                                          LinkedGeoPolygon *currentPoly) {
    H3Error err = addLinkedGeoLoop(&poly->geoloop, currentPoly);
    if (err) return err;

    for (int i = 0; i < poly->numHoles; i++) {
        err = addLinkedGeoLoop(&poly->holes[i], currentPoly);
        if (err) return err;
    }

    return E_SUCCESS;
}

/**
 * Convert a GeoMultiPolygon to a LinkedGeoPolygon.
 *
 * The first polygon is placed in the caller-owned `out` node.
 * Every loop must have >= 3 vertices; otherwise E_FAILED is returned.
 *
 * On error, the output is cleaned up via `destroyLinkedMultiPolygon()`.
 * On success, the caller owns the output and must free it with
 * `destroyLinkedMultiPolygon()`.
 *
 * @param  mpoly  Source GeoMultiPolygon
 * @param  out    Output LinkedGeoPolygon
 * @return        E_SUCCESS, E_FAILED (invalid geometry), or E_MEMORY_ALLOC
 */
H3Error geoMultiPolygonToLinkedGeoPolygon(const GeoMultiPolygon *mpoly,
                                          LinkedGeoPolygon *out) {
    *out = (LinkedGeoPolygon){0};

    LinkedGeoPolygon *currentPoly = out;
    for (int i = 0; i < mpoly->numPolygons; i++) {
        if (i > 0) {
            LinkedGeoPolygon *newPoly =
                H3_MEMORY(calloc)(1, sizeof(LinkedGeoPolygon));
            if (!newPoly) {
                H3_EXPORT(destroyLinkedMultiPolygon)(out);
                return E_MEMORY_ALLOC;
            }
            currentPoly->next = newPoly;
            currentPoly = newPoly;
        }

        H3Error err =
            geoPolygonToLinkedGeoLoops(&mpoly->polygons[i], currentPoly);
        if (err) {
            H3_EXPORT(destroyLinkedMultiPolygon)(out);
            return err;
        }
    }

    return E_SUCCESS;
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
H3Error normalizeMultiPolygon(LinkedGeoPolygon *root) {
    // We assume that the input is a single polygon with loops;
    // if it has multiple polygons, don't touch it
    if (root->next) {
        return E_FAILED;
    }

    // Count loops, exiting early if there's only one
    int loopCount = countLinkedLoops(root);
    if (loopCount <= 1) {
        return E_SUCCESS;
    }

    H3Error resultCode = E_SUCCESS;
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
            resultCode = E_FAILED;
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
