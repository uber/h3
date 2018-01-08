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
/** @file linkedGeo.h
 * @brief   Linked data structure for geo data
 */

#ifndef LINKED_GEO_H
#define LINKED_GEO_H

#include <stdlib.h>
#include "geoCoord.h"
#include "h3api.h"

void initLinkedPolygon(LinkedGeoPolygon* polygon);
LinkedGeoPolygon* addLinkedPolygon(LinkedGeoPolygon* polygon);
LinkedGeoLoop* addLinkedLoop(LinkedGeoPolygon* polygon);
LinkedGeoCoord* addLinkedCoord(LinkedGeoLoop* loop, const GeoCoord* vertex);
int countLinkedPolygons(LinkedGeoPolygon* polygon);
int countLinkedLoops(LinkedGeoPolygon* polygon);
int countLinkedCoords(LinkedGeoLoop* loop);

#endif
