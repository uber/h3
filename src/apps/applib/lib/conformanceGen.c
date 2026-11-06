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
/** @file conformanceGen.c
 * @brief Shared generator code for the conformance suite; see
 * conformanceGen.h.
 */

#include "conformanceGen.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

static const char *const H3_ERROR_NAMES[] = {
    "E_SUCCESS",          "E_FAILED",
    "E_DOMAIN",           "E_LATLNG_DOMAIN",
    "E_RES_DOMAIN",       "E_CELL_INVALID",
    "E_DIR_EDGE_INVALID", "E_UNDIR_EDGE_INVALID",
    "E_VERTEX_INVALID",   "E_PENTAGON",
    "E_DUPLICATE_INPUT",  "E_NOT_NEIGHBORS",
    "E_RES_MISMATCH",     "E_MEMORY_ALLOC",
    "E_MEMORY_BOUNDS",    "E_OPTION_INVALID",
    "E_INDEX_INVALID",    "E_BASE_CELL_DOMAIN",
    "E_DIGIT_DOMAIN",     "E_DELETED_DIGIT",
};

const char *conformanceErrorName(H3Error err) {
    if (err < sizeof(H3_ERROR_NAMES) / sizeof(H3_ERROR_NAMES[0])) {
        return H3_ERROR_NAMES[err];
    }
    return "E_UNKNOWN";
}

static void fail(const char *message) {
    fprintf(stderr, "conformance: %s\n", message);
    exit(1);
}

static void must(H3Error err) {
    if (err) {
        fprintf(stderr, "conformance: unexpected H3 error %s\n",
                conformanceErrorName(err));
        exit(1);
    }
}

/* ---- pseudo-random source (conformance.md, "Pseudo-random source") ---- */

uint64_t draw(Splitmix64 *rng) {
    rng->state += UINT64_C(0x9e3779b97f4a7c15);
    uint64_t z = rng->state;
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}

int drawN(Splitmix64 *rng, int n) { return (int)(draw(rng) % (uint64_t)n); }

/* ---- index construction (conformance.md, "Index construction") ---- */

unsigned digitShift(int r) { return (unsigned)((MAX_RES - r) * DIGIT_BITS); }

uint64_t buildIndex(int mode, int res, int baseCell, const int *digits) {
    uint64_t index = (uint64_t)mode << MODE_OFFSET |
                     (uint64_t)res << RES_OFFSET |
                     (uint64_t)baseCell << BASE_CELL_OFFSET;
    for (int r = 1; r <= MAX_RES; r++) {
        uint64_t digit = r <= res ? (uint64_t)digits[r - 1] : DIGIT_MASK;
        index |= digit << digitShift(r);
    }
    return index;
}

uint64_t setField(uint64_t index, unsigned offset, unsigned width,
                  uint64_t value) {
    uint64_t mask = ((UINT64_C(1) << width) - 1) << offset;
    return (index & ~mask) | ((value << offset) & mask);
}

uint64_t drawCell(Splitmix64 *rng, int res) {
    int baseCell = drawN(rng, NUM_BASE_CELLS);
    int digits[MAX_RES];
    for (int i = 0; i < res; i++) {
        digits[i] = drawN(rng, NUM_DIGITS);
    }
    return buildIndex(CELL_MODE, res, baseCell, digits);
}

uint64_t drawValidCell(Splitmix64 *rng, int res) {
    for (;;) {
        uint64_t index = drawCell(rng, res);
        if (H3_EXPORT(isValidCell)(index)) {
            return index;
        }
    }
}

int compareIndexes(const void *a, const void *b) {
    uint64_t x = *(const uint64_t *)a, y = *(const uint64_t *)b;
    return x < y ? -1 : x > y;
}

int compareInts(const void *a, const void *b) {
    int left = *(const int *)a;
    int right = *(const int *)b;
    return (left > right) - (left < right);
}

/* diskSize is H3_EXPORT(maxGridDiskSize)(k) for small k. */
int diskSize(int k) { return 1 + 3 * k * (k + 1); }

/* wrapDegrees brings a longitude difference or value into [-180, 180]. */
double wrapDegrees(double lng) {
    if (lng > 180.0) {
        return lng - 360.0;
    }
    if (lng < -180.0) {
        return lng + 360.0;
    }
    return lng;
}

/* scaledBoundary fills loop with the cell's boundary scaled about its center
 * by factor, computed in degrees so that a consumer parsing the written
 * degrees starts from the same values, then converted back to radians for
 * the reference. */
void scaledBoundary(H3Index h, double factor, CellBoundary *loop) {
    LatLng center;
    must(H3_EXPORT(cellToLatLng)(h, &center));
    double centerLat = H3_EXPORT(radsToDegs)(center.lat);
    double centerLng = H3_EXPORT(radsToDegs)(center.lng);

    must(H3_EXPORT(cellToBoundary)(h, loop));
    for (int i = 0; i < loop->numVerts; i++) {
        double lat =
            centerLat +
            factor * (H3_EXPORT(radsToDegs)(loop->verts[i].lat) - centerLat);
        double lng = wrapDegrees(
            centerLng +
            factor * wrapDegrees(H3_EXPORT(radsToDegs)(loop->verts[i].lng) -
                                 centerLng));
        loop->verts[i].lat = H3_EXPORT(degsToRads)(lat);
        loop->verts[i].lng = H3_EXPORT(degsToRads)(lng);
    }
}

void lineAppend(Line *line, const char *text) {
    size_t n = strlen(text);
    if (line->len + n + 1 > line->cap) {
        size_t cap = line->cap ? line->cap : 256;
        while (line->len + n + 1 > cap) {
            cap *= 2;
        }
        char *buf = realloc(line->buf, cap);
        if (!buf) {
            fail("out of memory");
        }
        line->buf = buf;
        line->cap = cap;
    }
    memcpy(line->buf + line->len, text, n + 1);
    line->len += n;
}

/* lineStart begins a line with the subject cell. */
void lineStart(Line *line, H3Index h) {
    char buf[MAX_INDEX_LEN];
    must(H3_EXPORT(h3ToString)(h, buf, sizeof buf));
    line->len = 0;
    lineAppend(line, buf);
}

void lineIndex(Line *line, H3Index h) {
    char buf[MAX_INDEX_LEN];
    must(H3_EXPORT(h3ToString)(h, buf, sizeof buf));
    lineAppend(line, " ");
    lineAppend(line, buf);
}

void lineInt(Line *line, int64_t value) {
    char buf[32];
    snprintf(buf, sizeof buf, " %" PRId64, value);
    lineAppend(line, buf);
}

void lineError(Line *line, H3Error err) {
    lineAppend(line, " ");
    lineAppend(line, conformanceErrorName(err));
}

/* lineIndexSet appends the distinct non-zero entries in ascending order,
 * sorting and compacting the array in place, and returns how many. */
int64_t lineIndexSet(Line *line, H3Index *indexes, int64_t count) {
    int64_t kept = 0;
    for (int64_t i = 0; i < count; i++) {
        if (indexes[i]) {
            indexes[kept++] = indexes[i];
        }
    }
    qsort(indexes, (size_t)kept, sizeof *indexes, compareIndexes);
    int64_t unique = 0;
    for (int64_t i = 0; i < kept; i++) {
        if (!unique || indexes[unique - 1] != indexes[i]) {
            indexes[unique++] = indexes[i];
        }
    }
    for (int64_t i = 0; i < unique; i++) {
        lineIndex(line, indexes[i]);
    }
    return unique;
}

const char *const STREAM_KEYS[NUM_STREAMS] = {
    "cells", "children",   "compact",  "disks",   "distances",
    "edges", "inspection", "localIj",  "parents", "polygons",
    "rings", "roundTrip",  "vertexes",
};

const int64_t PATTERN_COUNTS[PATTERN_TIERS] = {100000, 1000000, 10000000};

bool hasPolygons(int res) { return res <= POLYGONS_MAX_RES; }

/* lineFill appends polygonToCellsExperimental of the polygon as a set. */
void lineFill(Line *line, const GeoPolygon *polygon, int res, uint32_t flags) {
    int64_t size;
    must(H3_EXPORT(maxPolygonToCellsSizeExperimental)(polygon, res, flags,
                                                      &size));
    H3Index *cells = calloc(size ? (size_t)size : 1, sizeof *cells);
    if (!cells) {
        fail("out of memory");
    }
    must(H3_EXPORT(polygonToCellsExperimental)(polygon, res, flags, size,
                                               cells));
    lineIndexSet(line, cells, size);
    free(cells);
}

/* absorb terminates the line and feeds it to one stream's digest. */
void absorb(Sha256 *streams, int stream, Line *line) {
    lineAppend(line, "\n");
    sha256Update(&streams[stream], (const uint8_t *)line->buf, line->len);
}

/* digestCell appends one cell's line to every stream. */
void digestCell(Sha256 *streams, Line *line, H3Index h, int res) {
    lineStart(line, h);
    absorb(streams, S_CELLS, line);

    H3Index parent0;
    must(H3_EXPORT(cellToParent)(h, 0, &parent0));

    lineStart(line, h);
    for (int parentRes = res - 1; parentRes >= 0; parentRes--) {
        H3Index parent;
        must(H3_EXPORT(cellToParent)(h, parentRes, &parent));
        lineIndex(line, parent);
    }
    absorb(streams, S_PARENTS, line);

    lineStart(line, h);
    {
        H3Index center;
        must(H3_EXPORT(cellToCenterChild)(h, res + 1, &center));
        lineIndex(line, center);
        int64_t size;
        must(H3_EXPORT(cellToChildrenSize)(h, res + 1, &size));
        lineInt(line, size);
        int64_t pos;
        must(H3_EXPORT(cellToChildPos)(h, 0, &pos));
        lineInt(line, pos);
        H3Index back;
        must(H3_EXPORT(childPosToCell)(pos, parent0, res, &back));
        lineIndex(line, back);
    }
    absorb(streams, S_CHILDREN, line);

    lineStart(line, h);
    {
        lineInt(line, H3_EXPORT(getBaseCellNumber)(h));
        lineInt(line, H3_EXPORT(isPentagon)(h));
        lineInt(line, H3_EXPORT(isResClassIII)(h));
        int faceCount;
        must(H3_EXPORT(maxFaceCount)(h, &faceCount));
        int faces[NUM_ICOSA_FACES];
        for (int i = 0; i < NUM_ICOSA_FACES; i++) {
            faces[i] = -1;
        }
        must(H3_EXPORT(getIcosahedronFaces)(h, faces));
        qsort(faces, (size_t)faceCount, sizeof faces[0], compareInts);
        for (int i = 0; i < faceCount; i++) {
            if (faces[i] >= 0) {
                lineInt(line, faces[i]);
            }
        }
    }
    absorb(streams, S_INSPECTION, line);

    H3Index disk3[1 + 3 * MAX_DISK_K * (MAX_DISK_K + 1)] = {0};
    int64_t disk3Count = 0;
    lineStart(line, h);
    for (int k = 1; k <= MAX_DISK_K; k++) {
        H3Index disk[1 + 3 * MAX_DISK_K * (MAX_DISK_K + 1)] = {0};
        must(H3_EXPORT(gridDisk)(h, k, disk));
        if (k > 1) {
            lineAppend(line, " |");
        }
        int64_t kept = lineIndexSet(line, disk, diskSize(k));
        if (k == MAX_DISK_K) {
            memcpy(disk3, disk, sizeof disk3);
            disk3Count = kept;
        }
    }
    absorb(streams, S_DISKS, line);

    lineStart(line, h);
    for (int k = 1; k <= MAX_DISK_K; k++) {
        H3Index ring[1 + 3 * MAX_DISK_K * (MAX_DISK_K + 1)] = {0};
        must(H3_EXPORT(gridRing)(h, k, ring));
        if (k > 1) {
            lineAppend(line, " |");
        }
        lineIndexSet(line, ring, diskSize(k));
    }
    absorb(streams, S_RINGS, line);

    lineStart(line, h);
    for (int64_t i = 0; i < disk3Count; i++) {
        int64_t distance;
        H3Error err = H3_EXPORT(gridDistance)(h, disk3[i], &distance);
        if (err) {
            lineError(line, err);
        } else {
            lineInt(line, distance);
        }
    }
    absorb(streams, S_DISTANCES, line);

    lineStart(line, h);
    {
        H3Index edges[NUM_HEX_EDGES] = {0};
        must(H3_EXPORT(originToDirectedEdges)(h, edges));
        int64_t edgeCount = lineIndexSet(line, edges, NUM_HEX_EDGES);
        line->len = 0; /* rebuild: edges interleaved with destinations */
        lineStart(line, h);
        for (int64_t i = 0; i < edgeCount; i++) {
            H3Index destination;
            must(H3_EXPORT(getDirectedEdgeDestination)(edges[i], &destination));
            lineIndex(line, edges[i]);
            lineIndex(line, destination);
        }
        lineAppend(line, " |");
        H3Index disk1[7] = {0};
        must(H3_EXPORT(gridDisk)(h, 1, disk1));
        Line scratch = {0};
        int64_t diskCount = lineIndexSet(&scratch, disk1, 7);
        free(scratch.buf);
        for (int64_t i = 0; i < diskCount; i++) {
            int neighbor;
            H3Error err = H3_EXPORT(areNeighborCells)(h, disk1[i], &neighbor);
            if (err) {
                lineError(line, err);
            } else {
                lineInt(line, neighbor);
            }
        }
    }
    absorb(streams, S_EDGES, line);

    lineStart(line, h);
    {
        H3Index vertexes[NUM_VERTEX_NUMBERS] = {0};
        must(H3_EXPORT(cellToVertexes)(h, vertexes));
        lineIndexSet(line, vertexes, NUM_VERTEX_NUMBERS);
    }
    absorb(streams, S_VERTEXES, line);

    lineStart(line, h);
    {
        H3Index origin = parent0;
        if (res > 0) {
            must(H3_EXPORT(cellToCenterChild)(parent0, res, &origin));
        }
        CoordIJ ij;
        H3Error err = H3_EXPORT(cellToLocalIj)(origin, h, 0, &ij);
        if (err) {
            lineError(line, err);
        } else {
            lineInt(line, ij.i);
            lineInt(line, ij.j);
        }
    }
    absorb(streams, S_LOCAL_IJ, line);

    lineStart(line, h);
    {
        int64_t size;
        must(H3_EXPORT(cellToChildrenSize)(h, res + 1, &size));
        H3Index *children = calloc((size_t)size, sizeof *children);
        H3Index *compacted = calloc((size_t)size, sizeof *compacted);
        if (!children || !compacted) {
            fail("out of memory");
        }
        must(H3_EXPORT(cellToChildren)(h, res + 1, children));
        qsort(children, (size_t)size, sizeof *children, compareIndexes);
        must(H3_EXPORT(compactCells)(children, compacted, size - 1));
        lineIndexSet(line, compacted, size - 1);
        free(children);
        free(compacted);
    }
    absorb(streams, S_COMPACT, line);

    lineStart(line, h);
    {
        LatLng center;
        must(H3_EXPORT(cellToLatLng)(h, &center));
        H3Index back;
        must(H3_EXPORT(latLngToCell)(&center, res, &back));
        lineIndex(line, back);
    }
    absorb(streams, S_ROUND_TRIP, line);

    if (hasPolygons(res)) {
        lineStart(line, h);
        const double factors[] = {REGIONS_SHRINK, REGIONS_EXPAND};
        for (int f = 0; f < 2; f++) {
            CellBoundary loop;
            scaledBoundary(h, factors[f], &loop);
            GeoPolygon polygon = {{loop.numVerts, loop.verts}, 0, NULL};
            for (uint32_t flags = 0; flags < NUM_CONTAINMENT_MODES; flags++) {
                if (f || flags) {
                    lineAppend(line, " |");
                }
                lineFill(line, &polygon, res + POLYGONS_FILL_STEP, flags);
            }
        }
        absorb(streams, S_POLYGONS, line);
    }
}

/* finalizeStreams writes each stream's hex digest. */
void finalizeStreams(Sha256 *streams, char hex[NUM_STREAMS][65]) {
    for (int i = 0; i < NUM_STREAMS; i++) {
        sha256Final(&streams[i], hex[i]);
    }
}

/* digestBaseCell runs every stream over the cells of res descending from
 * baseCell, in ascending order, and returns their count. */
int64_t digestBaseCell(int res, int baseCell, Line *line,
                       char hex[NUM_STREAMS][65]) {
    H3Index base = buildIndex(CELL_MODE, 0, baseCell, NULL);
    int64_t count;
    must(H3_EXPORT(cellToChildrenSize)(base, res, &count));
    H3Index *cells = calloc((size_t)count, sizeof *cells);
    if (!cells) {
        fail("out of memory");
    }
    must(H3_EXPORT(cellToChildren)(base, res, cells));
    qsort(cells, (size_t)count, sizeof *cells, compareIndexes);

    Sha256 streams[NUM_STREAMS];
    for (int i = 0; i < NUM_STREAMS; i++) {
        sha256Init(&streams[i]);
    }
    for (int64_t i = 0; i < count; i++) {
        digestCell(streams, line, cells[i], res);
    }
    free(cells);
    finalizeStreams(streams, hex);
    return count;
}

void resolutionInit(ResolutionDigest *digest) {
    for (int i = 0; i < NUM_STREAMS; i++) {
        sha256Init(&digest->streams[i]);
    }
    digest->count = 0;
    digest->next = 0;
}

/* resolutionAdd absorbs one base cell's digests, which must arrive in
 * base-cell order. */
void resolutionAdd(ResolutionDigest *digest, int res, int baseCell,
                   int64_t count, char hex[NUM_STREAMS][65]) {
    if (baseCell != digest->next) {
        fail("base-cell rows out of order");
    }
    digest->next++;
    digest->count += count;
    for (int i = 0; i < NUM_STREAMS; i++) {
        if (i == S_POLYGONS && !hasPolygons(res)) {
            continue;
        }
        sha256Update(&digest->streams[i], (const uint8_t *)hex[i], 64);
    }
}

/* drawPattern draws one 64-bit word for the validity stream (conformance.md,
 * "digests/patterns.jsonl"): uniform, or a valid cell with one field
 * mutated. */
uint64_t drawPattern(Splitmix64 *rng) {
    int kind = drawN(rng, PATTERN_KINDS);
    if (kind == 0) {
        return draw(rng);
    }
    uint64_t cell = drawValidCell(rng, drawN(rng, MAX_RES + 1));
    switch (kind) {
        case 1:
            return setField(cell, MODE_OFFSET, 4,
                            (uint64_t)drawN(rng, NUM_MODES));
        case 2:
            return setField(cell, RESERVED_OFFSET, 3, (uint64_t)drawN(rng, 8));
        case 3:
            return setField(cell, RES_OFFSET, 4,
                            (uint64_t)drawN(rng, MAX_RES + 1));
        case 4:
            return setField(cell, BASE_CELL_OFFSET, 7,
                            (uint64_t)drawN(rng, 128));
        case 5: {
            int position = 1 + drawN(rng, MAX_RES);
            return setField(cell, digitShift(position), DIGIT_BITS,
                            (uint64_t)drawN(rng, NUM_DIGITS + 1));
        }
        case 6:
            return cell | HIGH_BIT;
        default: {
            uint64_t mode = drawN(rng, 2) ? 4 : 2;
            cell = setField(cell, MODE_OFFSET, 4, mode);
            return setField(cell, RESERVED_OFFSET, 3, (uint64_t)drawN(rng, 8));
        }
    }
}
