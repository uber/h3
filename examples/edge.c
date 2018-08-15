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
/**
 * Example program that finds the edge between two indexes and prints their
 * boundary vertices.
 */

#include <h3/h3api.h>
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    H3Index origin = 0x8a2a1072b59ffffL;
    H3Index destination = 0x8a2a1072b597fffL;  // north of the origin

    H3Index edge = getH3UnidirectionalEdge(origin, destination);
    printf("The edge is %" PRIx64 "\n", edge);

    GeoBoundary boundary;
    getH3UnidirectionalEdgeBoundary(edge, &boundary);
    for (int v = 0; v < boundary.numVerts; v++) {
        printf("Edge vertex #%d: %lf, %lf\n", v,
               radsToDegs(boundary.verts[v].lat),
               radsToDegs(boundary.verts[v].lon));
    }
    // Output:
    // The edge is 16a2a1072b59ffff
    // Edge vertex #0: 40.690059, -74.044152
    // Edge vertex #1: 40.689908, -74.045062
}
