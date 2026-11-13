/*
 * Copyright 2026 Uber Technologies, Inc.
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
 * @brief Conformance runner for `floats/distances.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceFloatsDistances <record-file>`
 *
 *  Each line holds two points and the great-circle distance between them
 *  in radians, kilometres and metres. The distance is measured between the
 *  points in the file, so only the distance functions are under test.
 */

#include "conformance.h"
#include "h3api.h"

static void checkDistance(const cJSON *record) {
    LatLng a, b;
    jsonPoint(jsonItem(record, "a"), &a);
    jsonPoint(jsonItem(record, "b"), &b);
    assertMeasure(record, "rads", E_SUCCESS,
                  H3_EXPORT(greatCircleDistanceRads)(&a, &b), UNIT_RADS);
    assertMeasure(record, "km", E_SUCCESS,
                  H3_EXPORT(greatCircleDistanceKm)(&a, &b), UNIT_KM);
    assertMeasure(record, "m", E_SUCCESS,
                  H3_EXPORT(greatCircleDistanceM)(&a, &b), UNIT_M);
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkDistance);
}
