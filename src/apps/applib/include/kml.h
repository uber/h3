/*
 * Copyright 2016-2017, 2020-2021 Uber Technologies, Inc.
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
 * @brief functions to generate simple KML files
 */

#ifndef KML_H
#define KML_H

#include "latLng.h"

// output the KML header file for points
void kmlPtsHeader(const char *name, const char *desc);
void kmlBoundaryHeader(const char *name, const char *desc);

// output the KML footer file
void kmlPtsFooter(void);
void kmlBoundaryFooter(void);

// output KML individual points or polygons
void outputLngLatKML(const LatLng *g);
void outputPointKML(const LatLng *g, const char *name);
void outputTriKML(const LatLng *v1, const LatLng *v2, const LatLng *v3,
                  const char *name);
void outputPolyKML(const LatLng geoVerts[], int numVerts, const char *name);
void outputBoundaryKML(const CellBoundary *b, const char *name);

#endif
