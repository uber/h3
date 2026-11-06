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
 * @brief Conformance runner for the `regions` group
 * (dev-docs/conformance.md).
 *
 *  usage: `testConformanceRegions <record-file>`
 *
 *  Each line holds one polygon and fill resolution with `polygonToCells`,
 *  `polygonToCellsExperimental` in every containment mode, and
 *  `cellsToLinkedMultiPolygon` of the classic fill. Cell sets are compared
 *  exactly; the multipolygon by structure and by points under the angular
 *  tolerance, with each outer loop allowed to start at any vertex and holes
 *  matched in any order.
 */

#include <stdlib.h>
#include <string.h>

#include "conformance.h"
#include "h3api.h"

/** Decodes a sequence of `[lat, lng]` points into a malloc'd loop. */
static void parseLoop(const cJSON *array, GeoLoop *loop) {
    c_assert(cJSON_IsArray(array), "loop is not an array");
    loop->numVerts = cJSON_GetArraySize(array);
    loop->verts = calloc(loop->numVerts ? (size_t)loop->numVerts : 1,
                         sizeof *loop->verts);
    c_assert(loop->verts, "out of memory");
    int i = 0;
    const cJSON *point;
    cJSON_ArrayForEach(point, array) { jsonPoint(point, &loop->verts[i++]); }
}

static void parsePolygon(const cJSON *object, GeoPolygon *polygon) {
    c_assert(cJSON_IsObject(object), "polygon: not an object");
    parseLoop(jsonArray(object, "outer"), &polygon->geoloop);
    const cJSON *holes = jsonArray(object, "holes");
    polygon->numHoles = cJSON_GetArraySize(holes);
    polygon->holes = calloc(polygon->numHoles ? (size_t)polygon->numHoles : 1,
                            sizeof *polygon->holes);
    c_assert(polygon->holes, "out of memory");
    int i = 0;
    const cJSON *hole;
    cJSON_ArrayForEach(hole, holes) { parseLoop(hole, &polygon->holes[i++]); }
}

static void freePolygon(GeoPolygon *polygon) {
    free(polygon->geoloop.verts);
    for (int i = 0; i < polygon->numHoles; i++) {
        free(polygon->holes[i].verts);
    }
    free(polygon->holes);
}

/** Fills the polygon with the classic or experimental function and checks
 * the result, or its error, against the record's key. */
static void checkFill(const cJSON *record, const char *key,
                      const GeoPolygon *polygon, int res, uint32_t flags,
                      bool experimental) {
    const cJSON *item = jsonItem(record, key);
    int64_t size = 0;
    H3Error err = experimental ? H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                                     polygon, res, flags, &size)
                               : H3_EXPORT(maxPolygonToCellsSize)(polygon, res,
                                                                  flags, &size);
    H3Index *cells = NULL;
    if (!err) {
        cells = calloc(size ? (size_t)size : 1, sizeof *cells);
        c_assert(cells, "out of memory");
        err = experimental
                  ? H3_EXPORT(polygonToCellsExperimental)(polygon, res, flags,
                                                          size, cells)
                  : H3_EXPORT(polygonToCells)(polygon, res, flags, cells);
    }
    if (!expectResult(item, err, key)) {
        assertIndexSet(item, cells, size, key);
    }
    free(cells);
}

/** Counts the points of a linked loop. */
static int linkedLoopLength(const LinkedGeoLoop *loop) {
    int n = 0;
    for (const LinkedLatLng *p = loop->first; p; p = p->next) {
        n++;
    }
    return n;
}

/** Whether a linked loop visits the expected points in order, starting at
 * any vertex. */
static bool loopsMatch(const LinkedGeoLoop *got, const cJSON *want) {
    int n = cJSON_GetArraySize(want);
    if (linkedLoopLength(got) != n) {
        return false;
    }
    if (n == 0) {
        return true;
    }
    LatLng *wantPoints = calloc((size_t)n, sizeof *wantPoints);
    c_assert(wantPoints, "out of memory");
    LatLng *gotPoints = calloc((size_t)n, sizeof *gotPoints);
    c_assert(gotPoints, "out of memory");
    int i = 0;
    const cJSON *point;
    cJSON_ArrayForEach(point, want) { jsonPoint(point, &wantPoints[i++]); }
    i = 0;
    for (const LinkedLatLng *p = got->first; p; p = p->next) {
        gotPoints[i++] = p->vertex;
    }
    bool matched = false;
    for (int offset = 0; offset < n && !matched; offset++) {
        matched = true;
        for (int j = 0; j < n && matched; j++) {
            matched =
                withinAngular(&gotPoints[(offset + j) % n], &wantPoints[j]);
        }
    }
    free(wantPoints);
    free(gotPoints);
    return matched;
}

/** Checks one linked polygon against `{"outer": loop, "holes": [loop...]}`:
 * the outer loop up to rotation, the holes in any order. */
static void checkLinkedPolygon(const LinkedGeoPolygon *got, const cJSON *want,
                               int index) {
    c_assert(cJSON_IsObject(want), "multiPolygon: entry is not an object");
    const cJSON *outer = jsonArray(want, "outer");
    const cJSON *holes = jsonArray(want, "holes");
    if (!got->first) {
        c_assert(
            cJSON_GetArraySize(outer) == 0 && cJSON_GetArraySize(holes) == 0,
            "multiPolygon: polygon has no loops, record has loops");
        return;
    }
    if (!loopsMatch(got->first, outer)) {
        conformanceFail("multiPolygon: polygon %d outer loop differs", index);
    }
    int numGot = 0;
    for (const LinkedGeoLoop *loop = got->first->next; loop;
         loop = loop->next) {
        numGot++;
    }
    int numWant = cJSON_GetArraySize(holes);
    if (numGot != numWant) {
        conformanceFail("multiPolygon: polygon %d has %d holes, want %d", index,
                        numGot, numWant);
    }
    bool *used = calloc(numGot ? (size_t)numGot : 1, sizeof *used);
    c_assert(used, "out of memory");
    const cJSON *hole;
    cJSON_ArrayForEach(hole, holes) {
        bool found = false;
        int i = 0;
        for (const LinkedGeoLoop *loop = got->first->next; loop && !found;
             loop = loop->next, i++) {
            if (!used[i] && loopsMatch(loop, hole)) {
                used[i] = true;
                found = true;
            }
        }
        if (!found) {
            conformanceFail("multiPolygon: polygon %d is missing a hole",
                            index);
        }
    }
    free(used);
}

static void checkRegions(const cJSON *record) {
    GeoPolygon polygon;
    parsePolygon(jsonItem(record, "polygon"), &polygon);
    int res = jsonInt(record, "res");

    checkFill(record, "cells", &polygon, res, 0, false);
    static const char *const MODE_KEYS[] = {"center", "full", "overlapping",
                                            "overlappingBbox"};
    for (uint32_t mode = 0; mode < 4; mode++) {
        checkFill(record, MODE_KEYS[mode], &polygon, res, mode, true);
    }
    freePolygon(&polygon);

    // multiPolygon is cellsToLinkedMultiPolygon of the classic fill, which
    // the record holds, in ascending order; or the fill's error.
    const cJSON *cellsItem = jsonItem(record, "cells");
    const cJSON *multiItem = jsonItem(record, "multiPolygon");
    if (isErrorObject(cellsItem)) {
        c_assert(
            isErrorObject(multiItem) && !strcmp(jsonString(multiItem, "err"),
                                                jsonString(cellsItem, "err")),
            "multiPolygon: expected the cells error");
        return;
    }
    int64_t count;
    H3Index *cells = parseIndexArray(cellsItem, &count);
    count = normalizeIndexSet(cells, count);
    if (count == 0) {
        c_assert(cJSON_IsArray(multiItem) && cJSON_GetArraySize(multiItem) == 0,
                 "multiPolygon: expected an empty sequence for no cells");
        free(cells);
        return;
    }
    LinkedGeoPolygon multi = {0};
    H3Error err =
        H3_EXPORT(cellsToLinkedMultiPolygon)(cells, (int)count, &multi);
    free(cells);
    if (expectResult(multiItem, err, "multiPolygon")) {
        return;
    }
    c_assert(cJSON_IsArray(multiItem), "multiPolygon: not an array");
    int numGot = 0;
    for (const LinkedGeoPolygon *poly = &multi; poly; poly = poly->next) {
        numGot++;
    }
    if (numGot != cJSON_GetArraySize(multiItem)) {
        conformanceFail("multiPolygon: got %d polygons, want %d", numGot,
                        cJSON_GetArraySize(multiItem));
    }
    int index = 0;
    const LinkedGeoPolygon *poly = &multi;
    const cJSON *want;
    cJSON_ArrayForEach(want, multiItem) {
        checkLinkedPolygon(poly, want, index++);
        poly = poly->next;
    }
    H3_EXPORT(destroyLinkedMultiPolygon)(&multi);
}

int main(int argc, char *argv[]) {
    return conformanceMain(argc, argv, checkRegions);
}
