/*
 * Copyright 2016-2021 Uber Technologies, Inc.
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
/** @file latLng.h
 * @brief   Geodetic (lat/lng) functions.
 */

#ifndef GEOPOINT_H
#define GEOPOINT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "h3api.h"

/** epsilon of ~0.1mm in degrees */
#define EPSILON_DEG .000000001
/** epsilon of ~0.1mm in radians */
#define EPSILON_RAD (EPSILON_DEG * M_PI_180)

void setGeoDegs(LatLng *p, double latDegs, double lngDegs);
double constrainLat(double lat);
double constrainLng(double lng);

bool geoAlmostEqual(const LatLng *p1, const LatLng *p2);
bool geoAlmostEqualThreshold(const LatLng *p1, const LatLng *p2,
                             double threshold);

// Internal functions

double _posAngleRads(double rads);
void _setGeoRads(LatLng *p, double latRads, double lngRads);
double _geoAzimuthRads(const LatLng *p1, const LatLng *p2);
void _geoAzDistanceRads(const LatLng *p1, double az, double distance,
                        LatLng *p2);

#endif
