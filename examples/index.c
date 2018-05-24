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
/**
 * Example program that converts coordinates to an H3 index (hexagon),
 * and then finds the vertices and center coordinates of the index.
 */

#include <h3/h3api.h>
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Get the H3 index of some location and print it.
    GeoCoord location;
    location.lat = degsToRads(40.689167);
    location.lon = degsToRads(-74.044444);
    int resolution = 10;
    H3Index indexed = geoToH3(&location, resolution);
    printf("The index is: %" PRIx64 "\n", indexed);

    // Get the vertices of the H3 index.
    GeoBoundary boundary;
    h3ToGeoBoundary(indexed, &boundary);
    // Indexes can have different number of vertices under some cases,
    // which is why boundary.numVerts is needed.
    for (int v = 0; v < boundary.numVerts; v++) {
        printf("Boundary vertex #%d: %lf, %lf\n", v,
               radsToDegs(boundary.verts[v].lat),
               radsToDegs(boundary.verts[v].lon));
    }

    // Get the center coordinates.
    GeoCoord center;
    h3ToGeo(indexed, &center);
    printf("Center coordinates: %lf, %lf\n", radsToDegs(center.lat),
           radsToDegs(center.lon));
}
