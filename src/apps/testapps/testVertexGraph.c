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

#include <assert.h>
#include <stdio.h>

#include "geoCoord.h"
#include "h3api.h"
#include "test.h"
#include "vertexGraph.h"

// Fixtures
static GeoCoord center;
static GeoCoord vertex1;
static GeoCoord vertex2;
static GeoCoord vertex3;
static GeoCoord vertex4;
static GeoCoord vertex5;
static GeoCoord vertex6;

SUITE(vertexGraph) {
    setGeoDegs(&center, 37.77362016769341, -122.41673772517154);
    setGeoDegs(&vertex1, 87.372002166, 166.160981117);
    setGeoDegs(&vertex2, 87.370101364, 166.160184306);
    setGeoDegs(&vertex3, 87.369088356, 166.196239997);
    setGeoDegs(&vertex4, 87.369975080, 166.233115768);
    setGeoDegs(&vertex5, 0, 0);
    setGeoDegs(&vertex6, -10, -10);

    TEST(makeVertexGraph) {
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        t_assert(graph.numBuckets == 10, "numBuckets set");
        t_assert(graph.size == 0, "size set");
        destroyVertexGraph(&graph);
    }

    TEST(vertexHash) {
        H3Index centerIndex;
        GeoBoundary outline;
        uint32_t hash1;
        uint32_t hash2;
        int numBuckets = 1000;

        for (int res = 0; res < 11; res++) {
            centerIndex = H3_EXPORT(geoToH3)(&center, res);
            H3_EXPORT(h3ToGeoBoundary)(centerIndex, &outline);
            for (int i = 0; i < outline.numVerts; i++) {
                hash1 = _hashVertex(&outline.verts[i], res, numBuckets);
                hash2 = _hashVertex(&outline.verts[(i + 1) % outline.numVerts],
                                    res, numBuckets);
                t_assert(hash1 != hash2, "Hashes must not be equal");
            }
        }
    }

    TEST(vertexHashNegative) {
        int numBuckets = 10;
        t_assert(_hashVertex(&vertex5, 5, numBuckets) < numBuckets,
                 "zero vertex hashes correctly");
        t_assert(_hashVertex(&vertex6, 5, numBuckets) < numBuckets,
                 "negative coordinates vertex hashes correctly");
    }

    TEST(addVertexNode) {
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        VertexNode* node;
        VertexNode* addedNode;

        // Basic add
        addedNode = addVertexNode(&graph, &vertex1, &vertex2);
        node = findNodeForEdge(&graph, &vertex1, &vertex2);
        t_assert(node != NULL, "Node found");
        t_assert(node == addedNode, "Right node found");
        t_assert(graph.size == 1, "Graph size incremented");

        // Collision add
        addedNode = addVertexNode(&graph, &vertex1, &vertex3);
        node = findNodeForEdge(&graph, &vertex1, &vertex3);
        t_assert(node != NULL, "Node found after hash collision");
        t_assert(node == addedNode, "Right node found");
        t_assert(graph.size == 2, "Graph size incremented");

        // Collision add #2
        addedNode = addVertexNode(&graph, &vertex1, &vertex4);
        node = findNodeForEdge(&graph, &vertex1, &vertex4);
        t_assert(node != NULL, "Node found after 2nd hash collision");
        t_assert(node == addedNode, "Right node found");
        t_assert(graph.size == 3, "Graph size incremented");

        // Exact match no-op
        node = findNodeForEdge(&graph, &vertex1, &vertex2);
        addedNode = addVertexNode(&graph, &vertex1, &vertex2);
        t_assert(node == findNodeForEdge(&graph, &vertex1, &vertex2),
                 "Exact match did not change existing node");
        t_assert(node == addedNode, "Old node returned");
        t_assert(graph.size == 3, "Graph size was not changed");

        destroyVertexGraph(&graph);
    }

    TEST(addVertexNodeDupe) {
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        VertexNode* node;
        VertexNode* addedNode;

        // Basic add
        addedNode = addVertexNode(&graph, &vertex1, &vertex2);
        node = findNodeForEdge(&graph, &vertex1, &vertex2);
        t_assert(node != NULL, "Node found");
        t_assert(node == addedNode, "Right node found");
        t_assert(graph.size == 1, "Graph size incremented");

        // Dupe add
        addedNode = addVertexNode(&graph, &vertex1, &vertex2);
        t_assert(node == addedNode, "addVertexNode returned the original node");
        t_assert(graph.size == 1, "Graph size not incremented");

        destroyVertexGraph(&graph);
    }

    TEST(findNodeForEdge) {
        // Basic lookup tested in testAddVertexNode, only test failures here
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        VertexNode* node;

        // Empty graph
        node = findNodeForEdge(&graph, &vertex1, &vertex2);
        t_assert(node == NULL, "Node lookup failed correctly for empty graph");

        addVertexNode(&graph, &vertex1, &vertex2);

        // Different hash
        node = findNodeForEdge(&graph, &vertex3, &vertex2);
        t_assert(node == NULL,
                 "Node lookup failed correctly for different hash");

        // Hash collision
        node = findNodeForEdge(&graph, &vertex1, &vertex3);
        t_assert(node == NULL,
                 "Node lookup failed correctly for hash collision");

        addVertexNode(&graph, &vertex1, &vertex4);

        // Hash collision, list iteration
        node = findNodeForEdge(&graph, &vertex1, &vertex3);
        t_assert(node == NULL,
                 "Node lookup failed correctly for collision w/iteration");

        destroyVertexGraph(&graph);
    }

    TEST(findNodeForVertex) {
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        VertexNode* node;

        // Empty graph
        node = findNodeForVertex(&graph, &vertex1);
        t_assert(node == NULL, "Node lookup failed correctly for empty graph");

        addVertexNode(&graph, &vertex1, &vertex2);

        node = findNodeForVertex(&graph, &vertex1);
        t_assert(node != NULL, "Node lookup succeeded for correct node");

        node = findNodeForVertex(&graph, &vertex3);
        t_assert(node == NULL,
                 "Node lookup failed correctly for different node");

        destroyVertexGraph(&graph);
    }

    TEST(removeVertexNode) {
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        VertexNode* node;
        int success;

        // Straight removal
        node = addVertexNode(&graph, &vertex1, &vertex2);
        success = removeVertexNode(&graph, node) == 0;

        t_assert(success, "Removal successful");
        t_assert(findNodeForVertex(&graph, &vertex1) == NULL,
                 "Node lookup cannot find node");
        t_assert(graph.size == 0, "Graph size decremented");

        // Remove end of list
        addVertexNode(&graph, &vertex1, &vertex2);
        node = addVertexNode(&graph, &vertex1, &vertex3);
        success = removeVertexNode(&graph, node) == 0;

        t_assert(success, "Removal successful");
        t_assert(findNodeForEdge(&graph, &vertex1, &vertex3) == NULL,
                 "Node lookup cannot find node");
        t_assert(findNodeForEdge(&graph, &vertex1, &vertex2)->next == NULL,
                 "Base bucket node not pointing to node");
        t_assert(graph.size == 1, "Graph size decremented");

        // This removal is just cleanup
        node = findNodeForVertex(&graph, &vertex1);
        assert(removeVertexNode(&graph, node) == 0);

        // Remove beginning of list
        node = addVertexNode(&graph, &vertex1, &vertex2);
        addVertexNode(&graph, &vertex1, &vertex3);
        success = removeVertexNode(&graph, node) == 0;

        t_assert(success, "Removal successful");
        t_assert(findNodeForEdge(&graph, &vertex1, &vertex2) == NULL,
                 "Node lookup cannot find node");
        t_assert(findNodeForEdge(&graph, &vertex1, &vertex3) != NULL,
                 "Node lookup can find previous end of list");
        t_assert(findNodeForEdge(&graph, &vertex1, &vertex3)->next == NULL,
                 "Base bucket node not pointing to node");
        t_assert(graph.size == 1, "Graph size decremented");

        // This removal is just cleanup
        node = findNodeForVertex(&graph, &vertex1);
        assert(removeVertexNode(&graph, node) == 0);

        // Remove middle of list
        addVertexNode(&graph, &vertex1, &vertex2);
        node = addVertexNode(&graph, &vertex1, &vertex3);
        addVertexNode(&graph, &vertex1, &vertex4);
        success = removeVertexNode(&graph, node) == 0;

        t_assert(success, "Removal successful");
        t_assert(findNodeForEdge(&graph, &vertex1, &vertex3) == NULL,
                 "Node lookup cannot find node");
        t_assert(findNodeForEdge(&graph, &vertex1, &vertex4) != NULL,
                 "Node lookup can find previous end of list");
        t_assert(graph.size == 2, "Graph size decremented");

        // Remove non-existent node
        node = calloc(1, sizeof(VertexNode));
        success = removeVertexNode(&graph, node) == 0;

        t_assert(!success, "Removal of non-existent node fails");
        t_assert(graph.size == 2, "Graph size unchanged");

        free(node);

        destroyVertexGraph(&graph);
    }

    TEST(firstVertexNode) {
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        VertexNode* node;
        VertexNode* addedNode;

        node = firstVertexNode(&graph);
        t_assert(node == NULL, "No node found for empty graph");

        addedNode = addVertexNode(&graph, &vertex1, &vertex2);

        node = firstVertexNode(&graph);
        t_assert(node == addedNode, "Node found");

        destroyVertexGraph(&graph);
    }

    TEST(destroyEmptyVertexGraph) {
        VertexGraph graph;
        initVertexGraph(&graph, 10, 9);
        destroyVertexGraph(&graph);
    }

    TEST(singleBucketVertexGraph) {
        VertexGraph graph;
        initVertexGraph(&graph, 1, 9);
        VertexNode* node;

        t_assert(graph.numBuckets == 1, "1 bucket created");

        node = firstVertexNode(&graph);
        t_assert(node == NULL, "No node found for empty graph");

        node = addVertexNode(&graph, &vertex1, &vertex2);
        t_assert(node != NULL, "Node added");
        t_assert(firstVertexNode(&graph) == node, "First node is node");

        addVertexNode(&graph, &vertex2, &vertex3);
        addVertexNode(&graph, &vertex3, &vertex4);
        t_assert(firstVertexNode(&graph) == node, "First node is still node");
        t_assert(graph.size == 3, "Graph size updated");

        destroyVertexGraph(&graph);
    }
}
