/*
 * Copyright 2024 Uber Technologies, Inc.
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
 * start and end vertexes.
 */

#include <h3/h3api.h>
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    H3Index origin = 0x8a2a1072b59ffffL;
    H3Index destination = 0x8a2a1072b597fffL;  // north of the origin

    H3Index edge;
    cellsToDirectedEdge(origin, destination, &edge);
    printf("The edge is %" PRIx64 "\n", edge);

    // Get the vertexes for this edge
    H3Index vertexes[2];
    directedEdgeToVertexes(edge, vertexes);
    
    printf("Start vertex: %" PRIx64 "\n", vertexes[0]);
    printf("End vertex: %" PRIx64 "\n", vertexes[1]);

    // Get the coordinates of the vertexes
    LatLng v1Coord, v2Coord;
    vertexToLatLng(vertexes[0], &v1Coord);
    vertexToLatLng(vertexes[1], &v2Coord);

    printf("Start vertex coordinates: %lf, %lf\n",
           radsToDegs(v1Coord.lat), radsToDegs(v1Coord.lng));
    printf("End vertex coordinates: %lf, %lf\n",
           radsToDegs(v2Coord.lat), radsToDegs(v2Coord.lng));

    // Compare with edge boundary
    CellBoundary boundary;
    directedEdgeToBoundary(edge, &boundary);
    printf("\nEdge boundary has %d vertices:\n", boundary.numVerts);
    for (int v = 0; v < boundary.numVerts; v++) {
        printf("  Boundary vertex #%d: %lf, %lf\n", v,
               radsToDegs(boundary.verts[v].lat),
               radsToDegs(boundary.verts[v].lng));
    }

    return 0;
}
