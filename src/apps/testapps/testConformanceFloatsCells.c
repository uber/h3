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
 * @brief Conformance runner for `floats/cells.jsonl`
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceFloatsCells <record-file>`
 *
 *  Each line holds one valid cell with its center, boundary and areas, and
 *  the cell that `latLngToCell` returns for points inside it. Coordinates
 *  are compared by angular distance and areas by relative error, under the
 *  manifest tolerances.
 */

#include "conformance.h"
#include "h3api.h"

static void checkFloatCell(const cJSON *record) {
    H3Index h = jsonIndex(record, "index");

    LatLng center;
    c_assert(!H3_EXPORT(cellToLatLng)(h, &center), "cellToLatLng");
    assertPoint(jsonItem(record, "center"), &center, "center");

    CellBoundary boundary;
    c_assert(!H3_EXPORT(cellToBoundary)(h, &boundary), "cellToBoundary");
    assertPointSequence(jsonItem(record, "boundary"), boundary.verts,
                        boundary.numVerts, "boundary");

    double area;
    H3Error err = H3_EXPORT(cellAreaRads2)(h, &area);
    assertMeasure(record, "areaRads2", err, area, UNIT_RADS2);
    err = H3_EXPORT(cellAreaKm2)(h, &area);
    assertMeasure(record, "areaKm2", err, area, UNIT_KM2);
    err = H3_EXPORT(cellAreaM2)(h, &area);
    assertMeasure(record, "areaM2", err, area, UNIT_M2);

    int interiorRes = jsonInt(record, "interiorRes");
    const cJSON *interior = jsonArray(record, "interior");
    const cJSON *interiorCells = jsonArray(record, "interiorCells");
    c_assert(cJSON_GetArraySize(interior) == cJSON_GetArraySize(interiorCells),
             "interior and interiorCells differ in length");
    const cJSON *point = interior->child;
    const cJSON *cellItem = interiorCells->child;
    for (; point; point = point->next, cellItem = cellItem->next) {
        LatLng p;
        jsonPoint(point, &p);
        H3Index cell;
        c_assert(!H3_EXPORT(latLngToCell)(&p, interiorRes, &cell),
                 "latLngToCell");
        assertIndex(cellItem, cell, "interiorCells");
    }
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkFloatCell);
}
