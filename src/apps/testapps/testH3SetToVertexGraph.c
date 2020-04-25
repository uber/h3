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
#include "utility.h"

SUITE(h3SetToVertexGraph) {
    TEST(empty) {
        VertexGraph graph;

        h3SetToVertexGraph(NULL, 0, &graph);

        t_assert(graph.size == 0, "No edges added to graph");

        destroyVertexGraph(&graph);
    }

    TEST(singleHex) {
        VertexGraph graph;
        H3Index set[] = {0x890dab6220bffff};
        int numHexes = ARRAY_SIZE(set);

        h3SetToVertexGraph(set, numHexes, &graph);
        t_assert(graph.size == 6, "All edges of one hex added to graph");

        destroyVertexGraph(&graph);
    }

    TEST(nonContiguous2) {
        VertexGraph graph;
        H3Index set[] = {0x8928308291bffff, 0x89283082943ffff};
        int numHexes = ARRAY_SIZE(set);

        h3SetToVertexGraph(set, numHexes, &graph);
        t_assert(graph.size == 12,
                 "All edges of two non-contiguous hexes added to graph");

        destroyVertexGraph(&graph);
    }

    TEST(contiguous2) {
        VertexGraph graph;
        H3Index set[] = {0x8928308291bffff, 0x89283082957ffff};
        int numHexes = ARRAY_SIZE(set);

        h3SetToVertexGraph(set, numHexes, &graph);
        t_assert(graph.size == 10, "All edges except 2 shared added to graph");

        destroyVertexGraph(&graph);
    }

    TEST(contiguous2distorted) {
        VertexGraph graph;
        H3Index set[] = {0x894cc5365afffff, 0x894cc536537ffff};
        int numHexes = ARRAY_SIZE(set);

        h3SetToVertexGraph(set, numHexes, &graph);
        t_assert(graph.size == 12, "All edges except 2 shared added to graph");

        destroyVertexGraph(&graph);
    }

    TEST(contiguous3) {
        VertexGraph graph;
        H3Index set[] = {0x8928308288bffff, 0x892830828d7ffff,
                         0x8928308289bffff};
        int numHexes = ARRAY_SIZE(set);

        h3SetToVertexGraph(set, numHexes, &graph);
        t_assert(graph.size == 3 * 4,
                 "All edges except 6 shared added to graph");

        destroyVertexGraph(&graph);
    }

    TEST(hole) {
        VertexGraph graph;
        H3Index set[] = {0x892830828c7ffff, 0x892830828d7ffff,
                         0x8928308289bffff, 0x89283082813ffff,
                         0x8928308288fffff, 0x89283082883ffff};
        int numHexes = ARRAY_SIZE(set);

        h3SetToVertexGraph(set, numHexes, &graph);
        t_assert(graph.size == (6 * 3) + 6,
                 "All outer edges and inner hole edges added to graph");

        destroyVertexGraph(&graph);
    }
}
