/*
 * Copyright 2017, 2020-2021 Uber Technologies, Inc.
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
/** @file vertexGraph.h
 * @brief   Data structure for storing a graph of vertices
 */

#ifndef VERTEX_GRAPH_H
#define VERTEX_GRAPH_H

#include <stdint.h>
#include <stdlib.h>

#include "latLng.h"

/** @struct VertexNode
 *  @brief A single node in a vertex graph, part of a linked list
 */
typedef struct VertexNode VertexNode;
struct VertexNode {
    H3Index from;
    H3Index to;
    VertexNode *next;
};

/** @struct VertexGraph
 *  @brief A data structure to store a graph of vertices
 */
typedef struct {
    VertexNode **buckets;
    int numBuckets;
    int size;
    int res;
} VertexGraph;

void initVertexGraph(VertexGraph *graph, int numBuckets, int res);

void destroyVertexGraph(VertexGraph *graph);

VertexNode *addVertexNode(VertexGraph *graph, H3Index fromVtx, H3Index toVtx);

int removeVertexNode(VertexGraph *graph, VertexNode *node);

VertexNode *findNodeForEdge(const VertexGraph *graph, H3Index fromVtx,
                            H3Index toVtx);

VertexNode *findNodeForVertex(const VertexGraph *graph, H3Index fromVtx);

VertexNode *firstVertexNode(const VertexGraph *graph);

// Internal functions
uint32_t _hashVertex(H3Index vertex, int numBuckets);
void _initVertexNode(VertexNode *node, H3Index fromVtx, H3Index toVtx);

#endif
