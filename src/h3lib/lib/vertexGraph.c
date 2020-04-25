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
/** @file vertexGraph.c
 * @brief   Data structure for storing a graph of vertices
 */

#include "vertexGraph.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "alloc.h"
#include "geoCoord.h"

/**
 * Initialize a new VertexGraph
 * @param graph       Graph to initialize
 * @param  numBuckets Number of buckets to include in the graph
 * @param  res        Resolution of the hexagons whose vertices we're storing
 */
void initVertexGraph(VertexGraph* graph, int numBuckets, int res) {
    if (numBuckets > 0) {
        graph->buckets = H3_MEMORY(calloc)(numBuckets, sizeof(VertexNode*));
        assert(graph->buckets != NULL);
    } else {
        graph->buckets = NULL;
    }
    graph->numBuckets = numBuckets;
    graph->size = 0;
    graph->res = res;
}

/**
 * Destroy a VertexGraph's sub-objects, freeing their memory. The caller is
 * responsible for freeing memory allocated to the VertexGraph struct itself.
 * @param graph Graph to destroy
 */
void destroyVertexGraph(VertexGraph* graph) {
    VertexNode* node;
    while ((node = firstVertexNode(graph)) != NULL) {
        removeVertexNode(graph, node);
    }
    H3_MEMORY(free)(graph->buckets);
}

/**
 * Get an integer hash for a lat/lon point, at a precision determined
 * by the current hexagon resolution.
 * TODO: Light testing suggests this might not be sufficient at resolutions
 * finer than 10. Design a better hash function if performance and collisions
 * seem to be an issue here.
 * @param  vertex     Lat/lon vertex to hash
 * @param  res        Resolution of the hexagon the vertex belongs to
 * @param  numBuckets Number of buckets in the graph
 * @return            Integer hash
 */
uint32_t _hashVertex(const GeoCoord* vertex, int res, int numBuckets) {
    // Simple hash: Take the sum of the lat and lon with a precision level
    // determined by the resolution, converted to int, modulo bucket count.
    return (uint32_t)fmod(fabs((vertex->lat + vertex->lon) * pow(10, 15 - res)),
                          numBuckets);
}

void _initVertexNode(VertexNode* node, const GeoCoord* fromVtx,
                     const GeoCoord* toVtx) {
    node->from = *fromVtx;
    node->to = *toVtx;
    node->next = NULL;
}

/**
 * Add a edge to the graph
 * @param graph   Graph to add node to
 * @param fromVtx Start vertex
 * @param toVtx   End vertex
 * @return        Pointer to the new node
 */
VertexNode* addVertexNode(VertexGraph* graph, const GeoCoord* fromVtx,
                          const GeoCoord* toVtx) {
    // Make the new node
    VertexNode* node = H3_MEMORY(malloc)(sizeof(VertexNode));
    assert(node != NULL);
    _initVertexNode(node, fromVtx, toVtx);
    // Determine location
    uint32_t index = _hashVertex(fromVtx, graph->res, graph->numBuckets);
    // Check whether there's an existing node in that spot
    VertexNode* currentNode = graph->buckets[index];
    if (currentNode == NULL) {
        // Set bucket to the new node
        graph->buckets[index] = node;
    } else {
        // Find the end of the list
        do {
            // Check the the edge we're adding doesn't already exist
            if (geoAlmostEqual(&currentNode->from, fromVtx) &&
                geoAlmostEqual(&currentNode->to, toVtx)) {
                // already exists, bail
                H3_MEMORY(free)(node);
                return currentNode;
            }
            if (currentNode->next != NULL) {
                currentNode = currentNode->next;
            }
        } while (currentNode->next != NULL);
        // Add the new node to the end of the list
        currentNode->next = node;
    }
    graph->size++;
    return node;
}

/**
 * Remove a node from the graph. The input node will be freed, and should
 * not be used after removal.
 * @param graph Graph to mutate
 * @param node  Node to remove
 * @return      0 on success, 1 on failure (node not found)
 */
int removeVertexNode(VertexGraph* graph, VertexNode* node) {
    // Determine location
    uint32_t index = _hashVertex(&node->from, graph->res, graph->numBuckets);
    VertexNode* currentNode = graph->buckets[index];
    int found = 0;
    if (currentNode != NULL) {
        if (currentNode == node) {
            graph->buckets[index] = node->next;
            found = 1;
        }
        // Look through the list
        while (!found && currentNode->next != NULL) {
            if (currentNode->next == node) {
                // splice the node out
                currentNode->next = node->next;
                found = 1;
            }
            currentNode = currentNode->next;
        }
    }
    if (found) {
        H3_MEMORY(free)(node);
        graph->size--;
        return 0;
    }
    // Failed to find the node
    return 1;
}

/**
 * Find the Vertex node for a given edge, if it exists
 * @param  graph   Graph to look in
 * @param  fromVtx Start vertex
 * @param  toVtx   End vertex, or NULL if we don't care
 * @return         Pointer to the vertex node, if found
 */
VertexNode* findNodeForEdge(const VertexGraph* graph, const GeoCoord* fromVtx,
                            const GeoCoord* toVtx) {
    // Determine location
    uint32_t index = _hashVertex(fromVtx, graph->res, graph->numBuckets);
    // Check whether there's an existing node in that spot
    VertexNode* node = graph->buckets[index];
    if (node != NULL) {
        // Look through the list and see if we find the edge
        do {
            if (geoAlmostEqual(&node->from, fromVtx) &&
                (toVtx == NULL || geoAlmostEqual(&node->to, toVtx))) {
                return node;
            }
            node = node->next;
        } while (node != NULL);
    }
    // Iteration lookup fail
    return NULL;
}

/**
 * Find a Vertex node starting at the given vertex
 * @param  graph   Graph to look in
 * @param  fromVtx Start vertex
 * @return         Pointer to the vertex node, if found
 */
VertexNode* findNodeForVertex(const VertexGraph* graph,
                              const GeoCoord* fromVtx) {
    return findNodeForEdge(graph, fromVtx, NULL);
}

/**
 * Get the next vertex node in the graph.
 * @param  graph Graph to iterate
 * @return       Vertex node, or NULL if at the end
 */
VertexNode* firstVertexNode(const VertexGraph* graph) {
    VertexNode* node = NULL;
    int currentIndex = 0;
    while (node == NULL) {
        if (currentIndex < graph->numBuckets) {
            // find the first node in the next bucket
            node = graph->buckets[currentIndex];
        } else {
            // end of iteration
            return NULL;
        }
        currentIndex++;
    }
    return node;
}
