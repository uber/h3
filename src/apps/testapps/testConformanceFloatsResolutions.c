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
 * @brief Conformance runner for `floats/resolutions.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceFloatsResolutions <record-file>`
 *
 *  Each line holds one resolution with the average hexagon area and edge
 *  length functions, or their error for resolution 16.
 */

#include "conformance.h"
#include "h3api.h"

static void checkResolution(const cJSON *record) {
    int res = jsonInt(record, "res");
    double value = 0;
    H3Error err = H3_EXPORT(getHexagonAreaAvgKm2)(res, &value);
    assertMeasure(record, "hexagonAreaKm2", err, value, UNIT_KM2);
    err = H3_EXPORT(getHexagonAreaAvgM2)(res, &value);
    assertMeasure(record, "hexagonAreaM2", err, value, UNIT_M2);
    err = H3_EXPORT(getHexagonEdgeLengthAvgKm)(res, &value);
    assertMeasure(record, "edgeLengthKm", err, value, UNIT_KM);
    err = H3_EXPORT(getHexagonEdgeLengthAvgM)(res, &value);
    assertMeasure(record, "edgeLengthM", err, value, UNIT_M);
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkResolution);
}
