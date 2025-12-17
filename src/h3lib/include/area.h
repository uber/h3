/*
 * Copyright 2025 Uber Technologies, Inc.
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
/** @file area.h
 * @brief   Area computation functions
 */

#ifndef AREA_H
#define AREA_H

#include "h3api.h"

GeoMultiPolygon createGlobeMultiPolygon();
H3Error geoLoopAreaRads2(GeoLoop loop, double *out);
H3Error geoPolygonAreaRads2(GeoPolygon poly, double *out);
H3Error geoMultiPolygonAreaRads2(GeoMultiPolygon mpoly, double *out);

#endif
