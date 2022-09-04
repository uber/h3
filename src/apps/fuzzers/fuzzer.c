/*
 * Copyright 2022 Uber Technologies, Inc.
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
/** @file
 * @brief Fuzzer program for the entire H3 API
 */

#ifndef FUZZER_COMBINED
#error \
    "fuzzer.c (combined fuzzer program) must be compiled with FUZZER_COMBINED defined"
#endif

#define FUZZER_COMBINED_INCLUDE_MAIN
#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

int fuzzerCellArea(const uint8_t *data, size_t size);
int fuzzerCellProperties(const uint8_t *data, size_t size);
int fuzzerCellsToLinkedMultiPolygon(const uint8_t *data, size_t size);
int fuzzerCellToLatLng(const uint8_t *data, size_t size);
int fuzzerCompact(const uint8_t *data, size_t size);
int fuzzerDirectedEdge(const uint8_t *data, size_t size);
int fuzzerDistances(const uint8_t *data, size_t size);
int fuzzerEdgeLength(const uint8_t *data, size_t size);
int fuzzerGridDisk(const uint8_t *data, size_t size);
int fuzzerHierarchy(const uint8_t *data, size_t size);
int fuzzerIndexIO(const uint8_t *data, size_t size);
int fuzzerLatLngToCell(const uint8_t *data, size_t size);
int fuzzerLocalIj(const uint8_t *data, size_t size);
int fuzzerPolygonToCells(const uint8_t *data, size_t size);
int fuzzerPolygonToCellsNoHoles(const uint8_t *data, size_t size);
int fuzzerResolutions(const uint8_t *data, size_t size);
int fuzzerVertexes(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // All return codes are ignored.
    fuzzerCellArea(data, size);
    fuzzerCellProperties(data, size);
    fuzzerCellsToLinkedMultiPolygon(data, size);
    fuzzerCellToLatLng(data, size);
    fuzzerCompact(data, size);
    fuzzerDirectedEdge(data, size);
    fuzzerDistances(data, size);
    fuzzerEdgeLength(data, size);
    fuzzerGridDisk(data, size);
    fuzzerHierarchy(data, size);
    fuzzerIndexIO(data, size);
    fuzzerLatLngToCell(data, size);
    fuzzerLocalIj(data, size);
    fuzzerPolygonToCells(data, size);
    fuzzerPolygonToCellsNoHoles(data, size);
    fuzzerResolutions(data, size);
    fuzzerVertexes(data, size);
    return 0;
}

AFL_HARNESS_MAIN(4096);
