/*
 * Copyright 2016-2020 Uber Technologies, Inc.
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
/** @file geoCoord.h
 * @brief   Geodetic (lat/lon) functions.
 */

#ifndef GEOCOORD_H
#define GEOCOORD_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "h3api.h"

/** epsilon of ~0.1mm in degrees */
#define EPSILON_DEG .000000001
/** epsilon of ~0.1mm in radians */
#define EPSILON_RAD (EPSILON_DEG * M_PI_180)

void setGeoDegs(GeoCoord* p, double latDegs, double lonDegs);
double constrainLat(double lat);
double constrainLng(double lng);

bool geoAlmostEqual(const GeoCoord* p1, const GeoCoord* p2);
bool geoAlmostEqualThreshold(const GeoCoord* p1, const GeoCoord* p2,
                             double threshold);

// Internal functions

double _posAngleRads(double rads);
void _setGeoRads(GeoCoord* p, double latRads, double lonRads);
double _geoAzimuthRads(const GeoCoord* p1, const GeoCoord* p2);
void _geoAzDistanceRads(const GeoCoord* p1, double az, double distance,
                        GeoCoord* p2);

#endif
