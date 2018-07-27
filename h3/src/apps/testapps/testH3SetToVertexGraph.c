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

BEGIN_TESTS(h3SetToVertexGraph);

TEST(empty) {
    VertexGraph graph;
    int numHexes = 0;
    H3Index* set = makeSet(NULL, numHexes);

    h3SetToVertexGraph(set, numHexes, &graph);

    t_assert(graph.size == 0, "No edges added to graph");

    destroyVertexGraph(&graph);
    free(set);
}

TEST(singleHex) {
    VertexGraph graph;
    char* hexes[] = {"890dab6220bffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    h3SetToVertexGraph(set, numHexes, &graph);
    t_assert(graph.size == 6, "All edges of one hex added to graph");

    destroyVertexGraph(&graph);
    free(set);
}

TEST(nonContiguous2) {
    VertexGraph graph;
    char* hexes[] = {"8928308291bffff", "89283082943ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    h3SetToVertexGraph(set, numHexes, &graph);
    t_assert(graph.size == 12,
             "All edges of two non-contiguous hexes added to graph");

    destroyVertexGraph(&graph);
    free(set);
}

TEST(contiguous2) {
    VertexGraph graph;
    char* hexes[] = {"8928308291bffff", "89283082957ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    h3SetToVertexGraph(set, numHexes, &graph);
    t_assert(graph.size == 10, "All edges except 2 shared added to graph");

    destroyVertexGraph(&graph);
    free(set);
}

TEST(contiguous2distorted) {
    VertexGraph graph;
    char* hexes[] = {"894cc5365afffff", "894cc536537ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    h3SetToVertexGraph(set, numHexes, &graph);
    t_assert(graph.size == 12, "All edges except 2 shared added to graph");

    destroyVertexGraph(&graph);
    free(set);
}

TEST(contiguous3) {
    VertexGraph graph;
    char* hexes[] = {"8928308288bffff", "892830828d7ffff", "8928308289bffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    h3SetToVertexGraph(set, numHexes, &graph);
    t_assert(graph.size == 3 * 4, "All edges except 6 shared added to graph");

    destroyVertexGraph(&graph);
    free(set);
}

TEST(hole) {
    VertexGraph graph;
    char* hexes[] = {"892830828c7ffff", "892830828d7ffff", "8928308289bffff",
                     "89283082813ffff", "8928308288fffff", "89283082883ffff"};
    int numHexes = sizeof(hexes) / sizeof(hexes[0]);
    H3Index* set = makeSet(hexes, numHexes);

    h3SetToVertexGraph(set, numHexes, &graph);
    t_assert(graph.size == (6 * 3) + 6,
             "All outer edges and inner hole edges added to graph");

    destroyVertexGraph(&graph);
    free(set);
}

END_TESTS();
