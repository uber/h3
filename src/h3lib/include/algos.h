/*
 * Copyright 2016-2018, 2020 Uber Technologies, Inc.
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
/** @file algos.h
 * @brief   Hexagon grid algorithms
 */

#ifndef ALGOS_H
#define ALGOS_H

#include "bbox.h"
#include "coordijk.h"
#include "h3api.h"
#include "linkedGeo.h"
#include "vertexGraph.h"

// neighbor along the ijk coordinate system of the current face, rotated
H3Error h3NeighborRotations(H3Index origin, Direction dir, int *rotations,
                            H3Index *out);

// IJK direction of neighbor
Direction directionForNeighbor(H3Index origin, H3Index destination);

// k-ring implementation
void _kRingInternal(H3Index origin, int k, H3Index *out, int *distances,
                    int maxIdx, int curK);

// Create a vertex graph from a set of hexagons
H3Error h3SetToVertexGraph(const H3Index *h3Set, const int numHexes,
                           VertexGraph *out);

// Create a LinkedGeoPolygon from a vertex graph
void _vertexGraphToLinkedGeo(VertexGraph *graph, LinkedGeoPolygon *out);

// Internal function for polygonToCells that traces a geoloop with hexagons of
// a specific size
H3Error _getEdgeHexagons(const GeoLoop *geoloop, int64_t numHexagons, int res,
                         int64_t *numSearchHexes, H3Index *search,
                         H3Index *found);

// The safe gridDiskDistances algorithm.
H3Error _gridDiskDistancesInternal(H3Index origin, int k, H3Index *out,
                                   int *distances, int64_t maxIdx, int curK);
#endif
