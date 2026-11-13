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
 * @brief Generates the H3 conformance suite under tests/inputfiles.
 *
 *  usage: `mkConformance -o <out-dir> [-s <seed>] [-g <group>]...`
 *
 *  Writes `<out-dir>/manifest.json` and one newline-delimited JSON record file
 *  per group under `<out-dir>`; the group directories must already exist.
 *  Every expected value is the library's own answer. The format and the
 *  sampling procedure, including the pseudo-random source and the order of
 *  draws, are specified in dev-docs/conformance.md; two generators that
 *  follow it produce byte-identical files from the same seed.
 *
 *  `-g` restricts the run to one group (`inspection`) or one file
 *  (`floats/cells.jsonl`) and may be repeated; the manifest then lists only
 *  the selected files. Without `-g` every file is written, which takes
 *  about ten minutes because of the digests group.
 *
 *  The program depends only on the public H3 API, the C standard library and
 *  applib's SHA-256, so the files it writes can be reproduced from any build
 *  of the library.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conformanceGen.h"
#include "h3api.h"
#include "sha256.h"

#define FORMAT_VERSION "h3-conformance/1"
#define GENERATOR_NAME "src/apps/testapps/mkConformance.c"
#define DEFAULT_SEED UINT64_C(20260929)
#define ANGULAR_DEG_TOLERANCE "1e-12"
#define RELATIVE_TOLERANCE "1e-9"

/* Sample sizes; see conformance.md, "Sampling". */
#define RANDOM_CELLS_PER_RES 48
#define MUTATION_BASES_PER_RES 4
#define INDEX_BASES_PER_RES 4
#define DRAWS_PER_INDEX_BASE 2
#define RANDOM_PATTERNS 256
#define HIERARCHY_CELLS_PER_RES 32
#define TRAVERSAL_CELLS_PER_RES 16
#define UNSAFE_CELLS_PER_RES 16
#define SETS_CELLS_PER_RES 2
#define REGIONS_MAX_RES 13
#define REGIONS_PENTAGON_STEP 4
#define REGIONS_CELLS_PER_RES 3
#define FLOATS_INTERIOR 0.5
#define FLOATS_DRAWS_PER_CELL 2
#define FLOATS_ANTIPODE_MARGIN 1e-3
#define FLOATS_PI 3.14159265358979323846
#define MAX_HOLES 3

/* Hand-picked inputs included regardless of seed. */
static const uint64_t FIXED_INDEXES[] = {
    UINT64_C(0x0),
    UINT64_C(0xffffffffffffffff),
    UINT64_C(0x7fffffffffffffff),
    UINT64_C(0x0800000000000000),
    UINT64_C(0x08001fffffffffff),
    UINT64_C(0x081283ffffffffff),
    UINT64_C(0x0804dfffffffffff),
    UINT64_C(0x0fffffffffffffff),
};

static void fail(const char *message) {
    fprintf(stderr, "mkConformance: %s\n", message);
    exit(1);
}

static void must(H3Error err) {
    if (err) {
        fprintf(stderr, "mkConformance: unexpected H3 error %s\n",
                conformanceErrorName(err));
        exit(1);
    }
}

/* ---- growable list of indexes ---- */

typedef struct {
    uint64_t *items;
    size_t len;
    size_t cap;
} IndexList;

static void push(IndexList *list, uint64_t index) {
    if (list->len == list->cap) {
        list->cap = list->cap ? list->cap * 2 : 1024;
        list->items = realloc(list->items, list->cap * sizeof *list->items);
        if (!list->items) {
            fail("out of memory");
        }
    }
    list->items[list->len++] = index;
}

static void sortAndDedup(IndexList *list) {
    qsort(list->items, list->len, sizeof *list->items, compareIndexes);
    size_t out = 0;
    for (size_t i = 0; i < list->len; i++) {
        if (out == 0 || list->items[out - 1] != list->items[i]) {
            list->items[out++] = list->items[i];
        }
    }
    list->len = out;
}

/* ---- inspection group sampling (conformance.md, "inspection/cells.jsonl")
 * ---- */

static void pushMutations(IndexList *list, Splitmix64 *rng, int res,
                          uint64_t base) {
    push(list, setField(base, MODE_OFFSET, 4, (uint64_t)drawN(rng, NUM_MODES)));
    push(list, setField(base, RESERVED_OFFSET, 3,
                        (uint64_t)(1 + drawN(rng, (int)DIGIT_MASK))));
    push(list, base | HIGH_BIT);
    push(list, setField(base, BASE_CELL_OFFSET, 7,
                        (uint64_t)(NUM_BASE_CELLS +
                                   drawN(rng, 128 - NUM_BASE_CELLS))));
    if (res > 0) {
        push(list, setField(base, digitShift(1 + drawN(rng, res)), DIGIT_BITS,
                            DIGIT_MASK));
    }
    if (res < MAX_RES) {
        int position = res + 1 + drawN(rng, MAX_RES - res);
        push(list, setField(base, digitShift(position), DIGIT_BITS,
                            (uint64_t)drawN(rng, NUM_DIGITS)));
    }
}

static void sampleInspection(IndexList *list, Splitmix64 *rng) {
    for (size_t i = 0; i < sizeof FIXED_INDEXES / sizeof FIXED_INDEXES[0];
         i++) {
        push(list, FIXED_INDEXES[i]);
    }

    for (int baseCell = 0; baseCell < NUM_BASE_CELLS; baseCell++) {
        push(list, buildIndex(CELL_MODE, 0, baseCell, NULL));
    }

    for (int res = 1; res <= MAX_RES; res++) {
        H3Index pentagons[NUM_PENTAGONS];
        must(H3_EXPORT(getPentagons)(res, pentagons));
        for (int i = 0; i < NUM_PENTAGONS; i++) {
            push(list, pentagons[i]);
        }
    }

    for (int res = 1; res <= MAX_RES; res++) {
        for (int i = 0; i < RANDOM_CELLS_PER_RES; i++) {
            push(list, drawCell(rng, res));
        }
    }

    for (int res = 0; res <= MAX_RES; res++) {
        for (int i = 0; i < MUTATION_BASES_PER_RES; i++) {
            pushMutations(list, rng, res, drawCell(rng, res));
        }
    }

    for (int res = 0; res <= MAX_RES; res++) {
        for (int i = 0; i < INDEX_BASES_PER_RES; i++) {
            H3Index cell = drawValidCell(rng, res);
            H3Index edges[NUM_HEX_EDGES], vertexes[NUM_HEX_EDGES];
            must(H3_EXPORT(originToDirectedEdges)(cell, edges));
            must(H3_EXPORT(cellToVertexes)(cell, vertexes));
            for (int j = 0; j < DRAWS_PER_INDEX_BASE; j++) {
                H3Index edge = edges[drawN(rng, NUM_HEX_EDGES)];
                if (edge) {
                    push(list, edge);
                }
            }
            for (int j = 0; j < DRAWS_PER_INDEX_BASE; j++) {
                H3Index vertex = vertexes[drawN(rng, NUM_HEX_EDGES)];
                if (vertex) {
                    push(list, vertex);
                }
            }
        }
    }

    for (int i = 0; i < RANDOM_PATTERNS; i++) {
        push(list, draw(rng));
    }

    sortAndDedup(list);
}

/* ---- inspection group records ---- */

/* printIndexBare prints the canonical string form without quotes. */
static void printIndexBare(FILE *out, H3Index h) {
    char buf[MAX_INDEX_LEN];
    must(H3_EXPORT(h3ToString)(h, buf, sizeof buf));
    fputs(buf, out);
}

static void printIndex(FILE *out, H3Index h) {
    fputc('"', out);
    printIndexBare(out, h);
    fputc('"', out);
}

static void printError(FILE *out, H3Error err) {
    fprintf(out, "{\"err\":\"%s\"}", conformanceErrorName(err));
}

static void printBool(FILE *out, int value) {
    fputs(value ? "true" : "false", out);
}

static void writeInspectionRecord(FILE *out, H3Index h, Splitmix64 *rng) {
    (void)rng;
    int res = H3_EXPORT(getResolution)(h);
    int baseCell = H3_EXPORT(getBaseCellNumber)(h);

    fputs("{\"index\":", out);
    printIndex(out, h);
    fprintf(out, ",\"res\":%d,\"baseCell\":%d,\"validCell\":", res, baseCell);
    printBool(out, H3_EXPORT(isValidCell)(h));
    fputs(",\"validIndex\":", out);
    printBool(out, H3_EXPORT(isValidIndex)(h));
    fputs(",\"resClassIII\":", out);
    printBool(out, H3_EXPORT(isResClassIII)(h));
    fputs(",\"pentagon\":", out);
    printBool(out, H3_EXPORT(isPentagon)(h));

    fputs(",\"faces\":", out);
    int faceCount;
    must(H3_EXPORT(maxFaceCount)(h, &faceCount));
    int *faces = calloc((size_t)faceCount, sizeof *faces);
    if (!faces) {
        fail("out of memory");
    }
    H3Error faceErr = H3_EXPORT(getIcosahedronFaces)(h, faces);
    if (faceErr) {
        printError(out, faceErr);
    } else {
        /* Drop the -1 padding and sort ascending; the set is small. */
        int kept = 0;
        for (int i = 0; i < faceCount; i++) {
            if (faces[i] >= 0) {
                faces[kept++] = faces[i];
            }
        }
        for (int i = 1; i < kept; i++) {
            for (int j = i; j > 0 && faces[j - 1] > faces[j]; j--) {
                int tmp = faces[j];
                faces[j] = faces[j - 1];
                faces[j - 1] = tmp;
            }
        }
        fputc('[', out);
        for (int i = 0; i < kept; i++) {
            fprintf(out, i ? ",%d" : "%d", faces[i]);
        }
        fputc(']', out);
    }
    free(faces);

    int digits[MAX_RES];
    fputs(",\"digits\":[", out);
    for (int r = 1; r <= MAX_RES; r++) {
        must(H3_EXPORT(getIndexDigit)(h, r, &digits[r - 1]));
        fprintf(out, r > 1 ? ",%d" : "%d", digits[r - 1]);
    }
    fputs("],\"construct\":", out);

    H3Index constructed;
    H3Error constructErr =
        H3_EXPORT(constructCell)(res, baseCell, digits, &constructed);
    if (constructErr) {
        printError(out, constructErr);
    } else {
        printIndex(out, constructed);
    }
    fputs("}\n", out);
}

/* ---- valid-cell subjects (conformance.md, "Valid-cell subjects") ---- */

/* sampleCells emits every base cell, every pentagon at every resolution and
 * perRes seeded valid cells per resolution, sorted and deduplicated. */
static void sampleCells(IndexList *list, Splitmix64 *rng, int perRes) {
    for (int baseCell = 0; baseCell < NUM_BASE_CELLS; baseCell++) {
        push(list, buildIndex(CELL_MODE, 0, baseCell, NULL));
    }

    for (int res = 1; res <= MAX_RES; res++) {
        H3Index pentagons[NUM_PENTAGONS];
        must(H3_EXPORT(getPentagons)(res, pentagons));
        for (int i = 0; i < NUM_PENTAGONS; i++) {
            push(list, pentagons[i]);
        }
    }

    for (int res = 1; res <= MAX_RES; res++) {
        for (int i = 0; i < perRes; i++) {
            push(list, drawValidCell(rng, res));
        }
    }

    sortAndDedup(list);
}

static void sampleHierarchy(IndexList *list, Splitmix64 *rng) {
    sampleCells(list, rng, HIERARCHY_CELLS_PER_RES);
}

static void sampleTraversal(IndexList *list, Splitmix64 *rng) {
    sampleCells(list, rng, TRAVERSAL_CELLS_PER_RES);
}

/* drawDiskTarget draws slots of the origin's k-disk, as gridDisk fills it,
 * until a non-zero one comes up. */
static H3Index drawDiskTarget(Splitmix64 *rng, H3Index origin, int k) {
    H3Index disk[1 + 3 * MAX_DISK_K * (MAX_DISK_K + 1)] = {0};
    must(H3_EXPORT(gridDisk)(origin, k, disk));
    for (;;) {
        H3Index target = disk[drawN(rng, diskSize(k))];
        if (target) {
            return target;
        }
    }
}

/* printIndexSequence prints indexes in the order given. */
static void printIndexSequence(FILE *out, const H3Index *indexes,
                               int64_t count) {
    fputc('[', out);
    for (int64_t i = 0; i < count; i++) {
        if (i) {
            fputc(',', out);
        }
        printIndex(out, indexes[i]);
    }
    fputc(']', out);
}

/* normalizeSet compacts the non-zero entries of indexes to the front, sorts
 * them ascending and removes duplicates, returning how many remain. */
static int64_t normalizeSet(H3Index *indexes, int64_t count) {
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
    return unique;
}

/* printIndexSet prints the distinct non-zero entries of indexes in ascending
 * order, compacting and sorting them in place, and returns how many were
 * kept. */
static int64_t printIndexSet(FILE *out, H3Index *indexes, int64_t count) {
    int64_t kept = normalizeSet(indexes, count);
    fputc('[', out);
    for (int64_t i = 0; i < kept; i++) {
        if (i) {
            fputc(',', out);
        }
        printIndex(out, indexes[i]);
    }
    fputc(']', out);
    return kept;
}

static void writeHierarchyRecord(FILE *out, H3Index h, Splitmix64 *rng) {
    (void)rng;
    int res = H3_EXPORT(getResolution)(h);

    fputs("{\"index\":", out);
    printIndex(out, h);

    fputs(",\"parents\":[", out);
    for (int r = 0; r < res; r++) {
        H3Index parent;
        must(H3_EXPORT(cellToParent)(h, r, &parent));
        if (r) {
            fputc(',', out);
        }
        printIndex(out, parent);
    }
    fputs("],\"centerChild\":", out);

    H3Index centerChild;
    H3Error err = H3_EXPORT(cellToCenterChild)(h, res + 1, &centerChild);
    if (err) {
        printError(out, err);
    } else {
        printIndex(out, centerChild);
    }

    fputs(",\"children\":", out);
    int64_t childCount;
    err = H3_EXPORT(cellToChildrenSize)(h, res + 1, &childCount);
    if (err) {
        printError(out, err);
    } else {
        H3Index *children = calloc((size_t)childCount, sizeof *children);
        if (!children) {
            fail("out of memory");
        }
        must(H3_EXPORT(cellToChildren)(h, res + 1, children));
        printIndexSet(out, children, childCount);
        free(children);
    }

    fputs(",\"childPos\":", out);
    int64_t childPos;
    err = H3_EXPORT(cellToChildPos)(h, 0, &childPos);
    if (err) {
        printError(out, err);
    } else {
        fprintf(out, "%" PRId64, childPos);
    }
    fputs("}\n", out);
}

/* ---- traversal group (conformance.md, "traversal/cells.jsonl") ---- */

/* printDiskSet prints gridDisk or gridRing output for k as a set. */
static void printDiskSet(FILE *out, H3Index origin, int k,
                         H3Error (*fn)(H3Index, int, H3Index *)) {
    H3Index cells[1 + 3 * MAX_DISK_K * (MAX_DISK_K + 1)] = {0};
    must(fn(origin, k, cells));
    printIndexSet(out, cells, diskSize(k));
}

static void writeTraversalPair(FILE *out, H3Index h, H3Index target);

/* writeTraversalRecord draws the target from the subject's 3-disk and writes
 * the pair. */
static void writeTraversalRecord(FILE *out, H3Index h, Splitmix64 *rng) {
    writeTraversalPair(out, h, drawDiskTarget(rng, h, MAX_DISK_K));
}

/* writeTraversalPair writes the subject's rings, then its distance, path and
 * neighbour relation to the target. Disks and distance-grouped disks are not
 * written: a k-disk is the origin and the rings 1 to k, so a consumer derives
 * them from the rings. */
static void writeTraversalPair(FILE *out, H3Index h, H3Index target) {
    fputs("{\"index\":", out);
    printIndex(out, h);

    for (int k = 1; k <= MAX_DISK_K; k++) {
        fprintf(out, ",\"ring%d\":", k);
        printDiskSet(out, h, k, H3_EXPORT(gridRing));
    }

    fputs(",\"target\":", out);
    printIndex(out, target);

    fputs(",\"distance\":", out);
    int64_t distance;
    H3Error err = H3_EXPORT(gridDistance)(h, target, &distance);
    if (err) {
        printError(out, err);
    } else {
        fprintf(out, "%" PRId64, distance);
    }

    fputs(",\"path\":", out);
    int64_t pathSize;
    err = H3_EXPORT(gridPathCellsSize)(h, target, &pathSize);
    if (!err) {
        H3Index *path = calloc((size_t)pathSize, sizeof *path);
        if (!path) {
            fail("out of memory");
        }
        err = H3_EXPORT(gridPathCells)(h, target, path);
        if (!err) {
            printIndexSequence(out, path, pathSize);
        }
        free(path);
    }
    if (err) {
        printError(out, err);
    }

    fputs(",\"neighbor\":", out);
    int neighbor;
    err = H3_EXPORT(areNeighborCells)(h, target, &neighbor);
    if (err) {
        printError(out, err);
    } else {
        printBool(out, neighbor);
    }
    fputs("}\n", out);
}

/* ---- unsafe group (conformance.md, "unsafe/cells.jsonl") ---- */

static void pushDisk(IndexList *list, H3Index h, int k);

/* sampleUnsafe takes the valid-cell subjects and adds, at every resolution,
 * every cell within MAX_DISK_K + 1 of each pentagon, which is where the
 * unsafe traversals decide between a result and E_PENTAGON. */
static void sampleUnsafe(IndexList *list, Splitmix64 *rng) {
    sampleCells(list, rng, UNSAFE_CELLS_PER_RES);

    for (int res = 0; res <= MAX_RES; res++) {
        H3Index pentagons[NUM_PENTAGONS];
        must(H3_EXPORT(getPentagons)(res, pentagons));
        for (int i = 0; i < NUM_PENTAGONS; i++) {
            pushDisk(list, pentagons[i], MAX_DISK_K + 1);
        }
    }

    sortAndDedup(list);
}

/* printUnsafeOutcome prints true when an unsafe traversal of the origin at k
 * succeeds, and its error otherwise. On success the cells it returned are, by
 * construction of the reference, exactly the safe traversal's, so they are
 * not repeated here. */
static void printUnsafeOutcome(FILE *out, H3Index origin, int k,
                               H3Error (*fn)(H3Index, int, H3Index *)) {
    H3Index cells[1 + 3 * MAX_DISK_K * (MAX_DISK_K + 1)] = {0};
    H3Error err = fn(origin, k, cells);
    if (err) {
        printError(out, err);
    } else {
        printBool(out, 1);
    }
}

/* printUnsafeOutcomes prints the outcomes of one unsafe traversal for k 1 to
 * MAX_DISK_K as a sequence. */
static void printUnsafeOutcomes(FILE *out, H3Index origin,
                                H3Error (*fn)(H3Index, int, H3Index *)) {
    fputc('[', out);
    for (int k = 1; k <= MAX_DISK_K; k++) {
        if (k > 1) {
            fputc(',', out);
        }
        printUnsafeOutcome(out, origin, k, fn);
    }
    fputc(']', out);
}

/* writeUnsafeRecord writes whether gridDiskUnsafe and gridRingUnsafe succeed
 * for k 1 to MAX_DISK_K. gridDiskDistancesUnsafe is gridDiskUnsafe's
 * implementation and gridDisksUnsafe is a loop over it, so neither gets a key.
 */
static void writeUnsafeRecord(FILE *out, H3Index h, Splitmix64 *rng) {
    (void)rng;
    fputs("{\"index\":", out);
    printIndex(out, h);
    fputs(",\"diskUnsafe\":", out);
    printUnsafeOutcomes(out, h, H3_EXPORT(gridDiskUnsafe));
    fputs(",\"ringUnsafe\":", out);
    printUnsafeOutcomes(out, h, H3_EXPORT(gridRingUnsafe));
    fputs("}\n", out);
}

/* ---- edges group (conformance.md, "edges/cells.jsonl") ---- */

static void writeEdgesRecord(FILE *out, H3Index h, Splitmix64 *rng) {
    fputs("{\"index\":", out);
    printIndex(out, h);

    H3Index edges[NUM_HEX_EDGES] = {0};
    must(H3_EXPORT(originToDirectedEdges)(h, edges));
    fputs(",\"edges\":", out);
    int64_t count = printIndexSet(out, edges, NUM_HEX_EDGES);

    H3Index destinations[NUM_HEX_EDGES];
    for (int64_t i = 0; i < count; i++) {
        must(H3_EXPORT(getDirectedEdgeDestination)(edges[i], &destinations[i]));
    }
    fputs(",\"destinations\":", out);
    printIndexSequence(out, destinations, count);

    H3Index target = drawDiskTarget(rng, h, 1);
    fputs(",\"target\":", out);
    printIndex(out, target);

    fputs(",\"edge\":", out);
    H3Index edge;
    H3Error err = H3_EXPORT(cellsToDirectedEdge)(h, target, &edge);
    if (err) {
        printError(out, err);
    } else {
        printIndex(out, edge);
    }
    fputs("}\n", out);
}

/* ---- vertexes group (conformance.md, "vertexes/cells.jsonl") ---- */

static void writeVertexesRecord(FILE *out, H3Index h, Splitmix64 *rng) {
    (void)rng;
    fputs("{\"index\":", out);
    printIndex(out, h);

    H3Index vertexes[NUM_VERTEX_NUMBERS] = {0};
    must(H3_EXPORT(cellToVertexes)(h, vertexes));
    fputs(",\"vertexes\":", out);
    printIndexSet(out, vertexes, NUM_VERTEX_NUMBERS);

    fputs(",\"byNumber\":[", out);
    for (int n = 0; n < NUM_VERTEX_NUMBERS; n++) {
        if (n) {
            fputc(',', out);
        }
        H3Index vertex;
        H3Error err = H3_EXPORT(cellToVertex)(h, n, &vertex);
        if (err) {
            printError(out, err);
        } else {
            printIndex(out, vertex);
        }
    }
    fputs("]}\n", out);
}

/* ---- localij group (conformance.md, "localij/pairs.jsonl") ---- */

static void writeLocalIJRecord(FILE *out, H3Index origin, H3Index target) {
    fputs("{\"index\":", out);
    printIndex(out, origin);
    fputs(",\"target\":", out);
    printIndex(out, target);

    fputs(",\"ij\":", out);
    CoordIJ ij;
    H3Error err = H3_EXPORT(cellToLocalIj)(origin, target, 0, &ij);
    if (err) {
        printError(out, err);
        fputs(",\"cell\":", out);
        printError(out, err);
    } else {
        fprintf(out, "{\"i\":%d,\"j\":%d}", ij.i, ij.j);
        fputs(",\"cell\":", out);
        H3Index cell;
        err = H3_EXPORT(localIjToCell)(origin, &ij, 0, &cell);
        if (err) {
            printError(out, err);
        } else {
            printIndex(out, cell);
        }
    }
    fputs("}\n", out);
}

/* ---- files and manifest ---- */

typedef struct {
    const char *name; /* slash-separated path relative to the manifest */
    long records;
    char sha256[65];
} FileInfo;

static char *joinPath(const char *dir, const char *name) {
    size_t len = strlen(dir) + 1 + strlen(name) + 1;
    char *path = malloc(len);
    if (!path) {
        fail("out of memory");
    }
    snprintf(path, len, "%s/%s", dir, name);
    return path;
}

/* describeFile reads a written record file back to count lines and hash it. */
static void describeFile(const char *path, FileInfo *info) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fail("could not reopen record file");
    }
    if (fseek(f, 0, SEEK_END)) {
        fail("seek");
    }
    long size = ftell(f);
    if (size < 0 || fseek(f, 0, SEEK_SET)) {
        fail("tell");
    }
    uint8_t *data = malloc((size_t)size + 1);
    if (!data) {
        fail("out of memory");
    }
    if (fread(data, 1, (size_t)size, f) != (size_t)size) {
        fail("read");
    }
    fclose(f);

    info->records = 0;
    for (long i = 0; i < size; i++) {
        info->records += data[i] == '\n';
    }
    sha256Hex(data, (size_t)size, info->sha256);
    free(data);
}

typedef void (*Sampler)(IndexList *list, Splitmix64 *rng);
typedef void (*RecordWriter)(FILE *out, H3Index h, Splitmix64 *rng);

/* A group whose subjects are single indexes. Each file starts from a fresh
 * generator seeded with the manifest seed; the sampler draws first, then the
 * writer draws for each subject in file order. A writer may emit more than
 * one line per subject. */
/* writeLocalIJSubject writes the origin with a near target drawn from its
 * 3-disk, then with a far target drawn at its resolution unless they match. */
static void writeLocalIJSubject(FILE *out, H3Index origin, Splitmix64 *rng) {
    H3Index near = drawDiskTarget(rng, origin, MAX_DISK_K);
    H3Index far = drawValidCell(rng, H3_EXPORT(getResolution)(origin));
    writeLocalIJRecord(out, origin, near);
    if (far != near) {
        writeLocalIJRecord(out, origin, far);
    }
}

/* ---- sets group (conformance.md, "sets/sets.jsonl") ---- */

static void sampleSets(IndexList *list, Splitmix64 *rng) {
    for (int res = 0; res <= MAX_RES; res++) {
        H3Index pentagons[NUM_PENTAGONS];
        must(H3_EXPORT(getPentagons)(res, pentagons));
        for (int i = 0; i < NUM_PENTAGONS; i++) {
            push(list, pentagons[i]);
        }
    }
    for (int res = 0; res <= MAX_RES; res++) {
        for (int i = 0; i < SETS_CELLS_PER_RES; i++) {
            push(list, drawValidCell(rng, res));
        }
    }
    sortAndDedup(list);
}

/* pushChildren appends H3_EXPORT(cellToChildren)(h, childRes) without its
 * padding. */
static void pushChildren(IndexList *list, H3Index h, int childRes) {
    int64_t count;
    must(H3_EXPORT(cellToChildrenSize)(h, childRes, &count));
    H3Index *children = calloc((size_t)count, sizeof *children);
    if (!children) {
        fail("out of memory");
    }
    must(H3_EXPORT(cellToChildren)(h, childRes, children));
    for (int64_t i = 0; i < count; i++) {
        if (children[i]) {
            push(list, children[i]);
        }
    }
    free(children);
}

/* pushDisk appends H3_EXPORT(gridDisk)(h, k) without its padding, keeping slot
 * order. */
static void pushDisk(IndexList *list, H3Index h, int k) {
    /* Sized for MAX_DISK_K + 1, the widest disk any sampler pushes. */
    H3Index disk[1 + 3 * (MAX_DISK_K + 1) * (MAX_DISK_K + 2)] = {0};
    must(H3_EXPORT(gridDisk)(h, k, disk));
    for (int i = 0; i < diskSize(k); i++) {
        if (disk[i]) {
            push(list, disk[i]);
        }
    }
}

/* writeSetsRecordNamed writes one line for the input set under the given id. */
static void writeSetsRecordNamed(FILE *out, const char *id,
                                 const IndexList *input, int uncompactRes) {
    fprintf(out, "{\"id\":\"%s\",\"input\":", id);
    printIndexSequence(out, input->items, (int64_t)input->len);

    fputs(",\"compact\":", out);
    H3Index *compacted = calloc(input->len ? input->len : 1, sizeof *compacted);
    if (!compacted) {
        fail("out of memory");
    }
    H3Error err =
        H3_EXPORT(compactCells)(input->items, compacted, (int64_t)input->len);
    if (err) {
        printError(out, err);
    } else {
        printIndexSet(out, compacted, (int64_t)input->len);
    }
    free(compacted);

    fprintf(out, ",\"uncompactRes\":%d,\"uncompact\":", uncompactRes);
    int64_t count;
    err = H3_EXPORT(uncompactCellsSize)(input->items, (int64_t)input->len,
                                        uncompactRes, &count);
    if (!err) {
        H3Index *cells = calloc(count ? (size_t)count : 1, sizeof *cells);
        if (!cells) {
            fail("out of memory");
        }
        err = H3_EXPORT(uncompactCells)(input->items, (int64_t)input->len,
                                        cells, count, uncompactRes);
        if (!err) {
            /* When uncompactRes is the input's own resolution the output is
             * the input as a set; write true instead of repeating it. */
            H3Index *same = calloc(input->len ? input->len : 1, sizeof *same);
            if (!same) {
                fail("out of memory");
            }
            memcpy(same, input->items, input->len * sizeof *same);
            int64_t inputCount = normalizeSet(same, (int64_t)input->len);
            int64_t outputCount = normalizeSet(cells, count);
            bool equal = inputCount == outputCount;
            for (int64_t i = 0; equal && i < inputCount; i++) {
                equal = same[i] == cells[i];
            }
            free(same);
            if (equal) {
                printBool(out, 1);
            } else {
                printIndexSet(out, cells, outputCount);
            }
        }
        free(cells);
    }
    if (err) {
        printError(out, err);
    }
    fputs("}\n", out);
}

/* writeSetsRecord writes one line for the input set under the id kind:cell. */
static void writeSetsRecord(FILE *out, const char *kind, H3Index h,
                            const IndexList *input, int uncompactRes) {
    char cell[MAX_INDEX_LEN];
    must(H3_EXPORT(h3ToString)(h, cell, sizeof cell));
    char id[64];
    snprintf(id, sizeof id, "%s:%s", kind, cell);
    writeSetsRecordNamed(out, id, input, uncompactRes);
}

/* writeSetsSubject writes the six kinds of set built from one cell. */
static void writeSetsSubject(FILE *out, H3Index h, Splitmix64 *rng) {
    int res = H3_EXPORT(getResolution)(h);
    int childRes = res + 2 > MAX_RES ? MAX_RES : res + 2;
    IndexList input = {0};

    pushChildren(&input, h, childRes);
    writeSetsRecord(out, "children", h, &input, childRes);

    input.len = 0;
    pushDisk(&input, h, 2);
    writeSetsRecord(out, "disk", h, &input, res + 1);

    input.len = 0;
    H3Index disk[1 + 3 * MAX_DISK_K * (MAX_DISK_K + 1)] = {0};
    must(H3_EXPORT(gridDisk)(h, MAX_DISK_K, disk));
    int holes = 1 + (int)drawN(rng, MAX_HOLES);
    for (int i = 0; i < holes; i++) {
        disk[drawN(rng, diskSize(MAX_DISK_K))] = 0;
    }
    for (int i = 0; i < diskSize(MAX_DISK_K); i++) {
        if (disk[i]) {
            push(&input, disk[i]);
        }
    }
    writeSetsRecord(out, "holes", h, &input, res);

    input.len = 0;
    H3Index siblingParent = h;
    if (res == MAX_RES) {
        must(H3_EXPORT(cellToParent)(h, res - 1, &siblingParent));
    }
    pushChildren(&input, siblingParent,
                 H3_EXPORT(getResolution)(siblingParent) + 1);
    push(&input, input.items[0]);
    writeSetsRecord(out, "duplicate", h, &input,
                    H3_EXPORT(getResolution)(siblingParent) + 1);

    input.len = 0;
    pushDisk(&input, h, 1);
    push(&input, h | ((uint64_t)1 << RESERVED_OFFSET));
    writeSetsRecord(out, "invalid", h, &input, res);

    input.len = 0;
    pushDisk(&input, h, 1);
    writeSetsRecord(out, "coarse", h, &input, res - 1);

    free(input.items);
}

/* ---- regions group (conformance.md, "regions/polygons.jsonl") ---- */

static void sampleRegions(IndexList *list, Splitmix64 *rng) {
    for (int res = 0; res <= REGIONS_MAX_RES; res += REGIONS_PENTAGON_STEP) {
        H3Index pentagons[NUM_PENTAGONS];
        must(H3_EXPORT(getPentagons)(res, pentagons));
        for (int i = 0; i < NUM_PENTAGONS; i++) {
            push(list, pentagons[i]);
        }
    }
    for (int res = 0; res <= REGIONS_MAX_RES; res++) {
        for (int i = 0; i < REGIONS_CELLS_PER_RES; i++) {
            push(list, drawValidCell(rng, res));
        }
    }
    sortAndDedup(list);
}

/* printLatLng prints a point as a [lat, lng] pair in degrees. */
static void printLatLng(FILE *out, LatLng point) {
    fprintf(out, "[%.17g,%.17g]", H3_EXPORT(radsToDegs)(point.lat),
            H3_EXPORT(radsToDegs)(point.lng));
}

/* printLoop prints a sequence of points. */
static void printLoop(FILE *out, const LatLng *verts, int numVerts) {
    fputc('[', out);
    for (int i = 0; i < numVerts; i++) {
        if (i) {
            fputc(',', out);
        }
        printLatLng(out, verts[i]);
    }
    fputc(']', out);
}

/* printLinkedLoop prints a linked loop as a sequence of points. */
static void printLinkedLoop(FILE *out, const LinkedGeoLoop *loop) {
    fputc('[', out);
    for (const LinkedLatLng *point = loop->first; point; point = point->next) {
        if (point != loop->first) {
            fputc(',', out);
        }
        printLatLng(out, point->vertex);
    }
    fputc(']', out);
}

/* printFill prints the cells a polyfill function returns, or its error. */
static void printFill(FILE *out, const GeoPolygon *polygon, int res,
                      uint32_t flags, bool experimental) {
    int64_t size;
    H3Error err = experimental ? H3_EXPORT(maxPolygonToCellsSizeExperimental)(
                                     polygon, res, flags, &size)
                               : H3_EXPORT(maxPolygonToCellsSize)(polygon, res,
                                                                  flags, &size);
    if (err) {
        printError(out, err);
        return;
    }
    H3Index *cells = calloc(size ? (size_t)size : 1, sizeof *cells);
    if (!cells) {
        fail("out of memory");
    }
    err = experimental ? H3_EXPORT(polygonToCellsExperimental)(
                             polygon, res, flags, size, cells)
                       : H3_EXPORT(polygonToCells)(polygon, res, flags, cells);
    if (err) {
        printError(out, err);
    } else {
        printIndexSet(out, cells, size);
    }
    free(cells);
}

/* printMultiPolygon prints cellsToLinkedMultiPolygon of the classic fill of
 * the polygon, or the error of whichever step failed. */
static void printMultiPolygon(FILE *out, const GeoPolygon *polygon, int res) {
    int64_t size;
    H3Error err = H3_EXPORT(maxPolygonToCellsSize)(polygon, res, 0, &size);
    if (err) {
        printError(out, err);
        return;
    }
    H3Index *cells = calloc(size ? (size_t)size : 1, sizeof *cells);
    if (!cells) {
        fail("out of memory");
    }
    err = H3_EXPORT(polygonToCells)(polygon, res, 0, cells);
    if (err) {
        free(cells);
        printError(out, err);
        return;
    }
    int64_t count = 0;
    for (int64_t i = 0; i < size; i++) {
        if (cells[i]) {
            cells[count++] = cells[i];
        }
    }
    qsort(cells, (size_t)count, sizeof *cells, compareIndexes);
    if (count == 0) {
        /* The reference still returns one polygon structure with no loops;
         * the record's sequence of polygons is empty. */
        free(cells);
        fputs("[]", out);
        return;
    }

    LinkedGeoPolygon multi = {0};
    err = H3_EXPORT(cellsToLinkedMultiPolygon)(cells, (int)count, &multi);
    free(cells);
    if (err) {
        printError(out, err);
        return;
    }
    fputc('[', out);
    for (const LinkedGeoPolygon *poly = &multi; poly; poly = poly->next) {
        if (poly != &multi) {
            fputc(',', out);
        }
        if (!poly->first) {
            fputs("{\"outer\":[],\"holes\":[]}", out);
            continue;
        }
        fputs("{\"outer\":", out);
        printLinkedLoop(out, poly->first);
        fputs(",\"holes\":[", out);
        for (const LinkedGeoLoop *loop = poly->first->next; loop;
             loop = loop->next) {
            if (loop != poly->first->next) {
                fputc(',', out);
            }
            printLinkedLoop(out, loop);
        }
        fputs("]}", out);
    }
    fputc(']', out);
    H3_EXPORT(destroyLinkedMultiPolygon)(&multi);
}

/* writeRegionsRecordNamed writes one line for the polygon filled at res under
 * the given id. */
static void writeRegionsRecordNamed(FILE *out, const char *id,
                                    const GeoPolygon *polygon, int res) {
    fprintf(out, "{\"id\":\"%s\",\"polygon\":{\"outer\":", id);
    printLoop(out, polygon->geoloop.verts, polygon->geoloop.numVerts);
    fputs(",\"holes\":[", out);
    for (int i = 0; i < polygon->numHoles; i++) {
        if (i) {
            fputc(',', out);
        }
        printLoop(out, polygon->holes[i].verts, polygon->holes[i].numVerts);
    }
    fprintf(out, "]},\"res\":%d", res);

    fputs(",\"cells\":", out);
    printFill(out, polygon, res, 0, false);
    static const char *const MODE_KEYS[] = {"center", "full", "overlapping",
                                            "overlappingBbox"};
    for (uint32_t mode = 0; mode < 4; mode++) {
        fprintf(out, ",\"%s\":", MODE_KEYS[mode]);
        printFill(out, polygon, res, mode, true);
    }
    fputs(",\"multiPolygon\":", out);
    printMultiPolygon(out, polygon, res);
    fputs("}\n", out);
}

/* writeRegionsRecord writes one line for the polygon filled at res under the
 * id cell:res, with :hole appended when the polygon has a hole. */
static void writeRegionsRecord(FILE *out, H3Index h, const GeoPolygon *polygon,
                               int res) {
    char cell[MAX_INDEX_LEN];
    must(H3_EXPORT(h3ToString)(h, cell, sizeof cell));
    char id[64];
    snprintf(id, sizeof id, "%s:%d%s", cell, res,
             polygon->numHoles ? ":hole" : "");
    writeRegionsRecordNamed(out, id, polygon, res);
}

/* writeRegionsSubject writes the cell's boundary shrunk and filled one
 * resolution finer, expanded and filled two finer, the same with the expanded
 * center child cut out, then expanded at an invalid resolution. */
static void writeRegionsSubject(FILE *out, H3Index h, Splitmix64 *rng) {
    (void)rng;
    int res = H3_EXPORT(getResolution)(h);

    CellBoundary outer;
    scaledBoundary(h, REGIONS_SHRINK, &outer);
    GeoPolygon polygon = {{outer.numVerts, outer.verts}, 0, NULL};
    writeRegionsRecord(out, h, &polygon, res + 1);

    scaledBoundary(h, REGIONS_EXPAND, &outer);
    writeRegionsRecord(out, h, &polygon, res + 2);

    H3Index center;
    must(H3_EXPORT(cellToCenterChild)(h, res + 1, &center));
    CellBoundary inner;
    scaledBoundary(center, REGIONS_EXPAND, &inner);
    GeoLoop hole = {inner.numVerts, inner.verts};
    polygon.numHoles = 1;
    polygon.holes = &hole;
    writeRegionsRecord(out, h, &polygon, res + 2);

    polygon.numHoles = 0;
    polygon.holes = NULL;
    writeRegionsRecord(out, h, &polygon, MAX_RES + 1);
}

/* ---- digests group (conformance.md, "digests/") ---- */

static FILE *createFile(const char *path);

/* printDigests prints the "digests" object from hex digests and closes the
 * row. */
static void printDigests(FILE *out, char hex[NUM_STREAMS][65], int res) {
    fputs(",\"digests\":{", out);
    for (int i = 0; i < NUM_STREAMS; i++) {
        if (i == S_POLYGONS && !hasPolygons(res)) {
            continue;
        }
        fprintf(out, "%s\"%s\":\"%s\"", i ? "," : "", STREAM_KEYS[i], hex[i]);
    }
    fputs("}}\n", out);
}

/* printBaseCellRow prints one digests/baseCells.jsonl line. */
static void printBaseCellRow(FILE *out, int res, int baseCell, int64_t count,
                             char hex[NUM_STREAMS][65]) {
    fprintf(out, "{\"res\":%d,\"baseCell\":%d,\"count\":%" PRId64, res,
            baseCell, count);
    printDigests(out, hex, res);
}

/* printResolutionRow prints the accumulated line once every base cell has
 * been added. */
static void printResolutionRow(FILE *out, int res, ResolutionDigest *digest) {
    if (digest->next != NUM_BASE_CELLS) {
        fail("resolution row needs all 122 base cells");
    }
    char hex[NUM_STREAMS][65];
    finalizeStreams(digest->streams, hex);
    fprintf(out, "{\"res\":%d,\"count\":%" PRId64, res, digest->count);
    printDigests(out, hex, res);
}

/* writeDigestSlice prints the base-cell rows of one resolution for base
 * cells first..last to stdout, so that a resolution can be produced by
 * several processes and the rows concatenated in base-cell order. */
static void writeDigestSlice(int res, int first, int last) {
    Line line = {0};
    for (int baseCell = first; baseCell <= last; baseCell++) {
        char hex[NUM_STREAMS][65];
        int64_t count = digestBaseCell(res, baseCell, &line, hex);
        printBaseCellRow(stdout, res, baseCell, count, hex);
    }
    free(line.buf);
}

/* parseBaseCellRow reads the fields of one digests/baseCells.jsonl line as
 * this program writes them. */
static bool parseBaseCellRow(const char *row, int *res, int *baseCell,
                             int64_t *count, char hex[NUM_STREAMS][65]) {
    if (sscanf(row, "{\"res\":%d,\"baseCell\":%d,\"count\":%" SCNd64, res,
               baseCell, count) != 3) {
        return false;
    }
    for (int i = 0; i < NUM_STREAMS; i++) {
        char key[32];
        snprintf(key, sizeof key, "\"%s\":\"", STREAM_KEYS[i]);
        const char *value = strstr(row, key);
        if (!value) {
            if (i == S_POLYGONS && !hasPolygons(*res)) {
                hex[i][0] = '\0';
                continue;
            }
            return false;
        }
        value += strlen(key);
        if (strlen(value) < 64) {
            return false;
        }
        memcpy(hex[i], value, 64);
        hex[i][64] = '\0';
    }
    return true;
}

/* writeResolutionRow reads base-cell rows from stdin and prints the
 * resolution row they combine to. */
static void writeResolutionRow(int res) {
    ResolutionDigest digest;
    resolutionInit(&digest);
    char row[8192];
    while (fgets(row, sizeof row, stdin)) {
        int rowRes, baseCell;
        int64_t count;
        char hex[NUM_STREAMS][65];
        if (!parseBaseCellRow(row, &rowRes, &baseCell, &count, hex)) {
            fail("malformed base-cell row");
        }
        if (rowRes == res) {
            resolutionAdd(&digest, res, baseCell, count, hex);
        }
    }
    printResolutionRow(stdout, res, &digest);
}

/* writePatternsFile writes digests/patterns.jsonl: one validity digest per
 * tier, each over the first PATTERN_COUNTS[tier] patterns of one stream. */
static void writePatternsFile(const char *outDir, uint64_t seed,
                              FileInfo *info) {
    char *path = joinPath(outDir, info->name);
    FILE *out = createFile(path);
    Splitmix64 rng = {seed};
    Sha256 digest;
    sha256Init(&digest);
    Line line = {0};
    int tier = 0;

    for (int64_t n = 1; n <= PATTERN_COUNTS[PATTERN_TIERS - 1]; n++) {
        uint64_t word = drawPattern(&rng);
        char buf[32];
        snprintf(buf, sizeof buf, "%" PRIx64, word);
        line.len = 0;
        lineAppend(&line, buf);
        lineInt(&line, H3_EXPORT(isValidCell)(word));
        lineInt(&line, H3_EXPORT(isValidIndex)(word));
        lineInt(&line, H3_EXPORT(isValidDirectedEdge)(word));
        lineInt(&line, H3_EXPORT(isValidVertex)(word));
        lineAppend(&line, "\n");
        sha256Update(&digest, (const uint8_t *)line.buf, line.len);

        if (n == PATTERN_COUNTS[tier]) {
            Sha256 snapshot = digest;
            char hex[65];
            sha256Final(&snapshot, hex);
            fprintf(out,
                    "{\"count\":%" PRId64
                    ",\"digests\":{\"validity\":\"%s\"}}\n",
                    n, hex);
            tier++;
        }
    }

    free(line.buf);
    if (fclose(out)) {
        fail("write");
    }
    describeFile(path, info);
    free(path);
}

/* writeDigestsFiles writes, for every resolution 0..DIGESTS_MAX_RES, one
 * line of per-resolution digests and 122 lines of per-base-cell digests. */
static void writeDigestsFiles(const char *outDir, FileInfo *resInfo,
                              FileInfo *baseInfo) {
    char *resPath = joinPath(outDir, resInfo->name);
    char *basePath = joinPath(outDir, baseInfo->name);
    FILE *resOut = createFile(resPath);
    FILE *baseOut = createFile(basePath);
    Line line = {0};

    for (int res = 0; res <= DIGESTS_MAX_RES; res++) {
        ResolutionDigest resolution;
        resolutionInit(&resolution);
        for (int baseCell = 0; baseCell < NUM_BASE_CELLS; baseCell++) {
            char hex[NUM_STREAMS][65];
            int64_t count = digestBaseCell(res, baseCell, &line, hex);
            printBaseCellRow(baseOut, res, baseCell, count, hex);
            resolutionAdd(&resolution, res, baseCell, count, hex);
        }
        printResolutionRow(resOut, res, &resolution);
    }

    free(line.buf);
    if (fclose(resOut) || fclose(baseOut)) {
        fail("write");
    }
    describeFile(resPath, resInfo);
    describeFile(basePath, baseInfo);
    free(resPath);
    free(basePath);
}

/* ---- floats group (conformance.md, "floats/") ---- */

static FILE *createFile(const char *path);

/* printDouble prints a binary64 with enough digits to round-trip. */
static void printDouble(FILE *out, double value) {
    fprintf(out, "%.17g", value);
}

/* printDegrees prints a point already held in degrees. */
static void printDegrees(FILE *out, double lat, double lng) {
    fprintf(out, "[%.17g,%.17g]", lat, lng);
}

/* printMeasure prints a key and a measurement, or its error. */
static void printMeasure(FILE *out, const char *key, H3Error err,
                         double value) {
    fprintf(out, ",\"%s\":", key);
    if (err) {
        printError(out, err);
    } else {
        printDouble(out, value);
    }
}

static void writeFloatCellRecord(FILE *out, H3Index h, Splitmix64 *rng) {
    (void)rng;
    int res = H3_EXPORT(getResolution)(h);

    LatLng center;
    must(H3_EXPORT(cellToLatLng)(h, &center));
    CellBoundary boundary;
    must(H3_EXPORT(cellToBoundary)(h, &boundary));

    fputs("{\"index\":", out);
    printIndex(out, h);
    fputs(",\"center\":", out);
    printLatLng(out, center);
    fputs(",\"boundary\":", out);
    printLoop(out, boundary.verts, boundary.numVerts);

    double area;
    must(H3_EXPORT(cellAreaRads2)(h, &area));
    printMeasure(out, "areaRads2", E_SUCCESS, area);
    must(H3_EXPORT(cellAreaKm2)(h, &area));
    printMeasure(out, "areaKm2", E_SUCCESS, area);
    must(H3_EXPORT(cellAreaM2)(h, &area));
    printMeasure(out, "areaM2", E_SUCCESS, area);

    /* Interior points halfway from the center to each boundary vertex,
     * computed in degrees so the file holds exactly what was used. */
    int interiorRes = res + 2 > MAX_RES ? MAX_RES : res + 2;
    double centerLat = H3_EXPORT(radsToDegs)(center.lat);
    double centerLng = H3_EXPORT(radsToDegs)(center.lng);
    double lats[MAX_CELL_BNDRY_VERTS];
    double lngs[MAX_CELL_BNDRY_VERTS];
    for (int i = 0; i < boundary.numVerts; i++) {
        lats[i] =
            centerLat +
            FLOATS_INTERIOR *
                (H3_EXPORT(radsToDegs)(boundary.verts[i].lat) - centerLat);
        lngs[i] = wrapDegrees(
            centerLng +
            FLOATS_INTERIOR *
                wrapDegrees(H3_EXPORT(radsToDegs)(boundary.verts[i].lng) -
                            centerLng));
    }

    fprintf(out, ",\"interiorRes\":%d,\"interior\":[", interiorRes);
    for (int i = 0; i < boundary.numVerts; i++) {
        if (i) {
            fputc(',', out);
        }
        printDegrees(out, lats[i], lngs[i]);
    }
    fputs("],\"interiorCells\":[", out);
    for (int i = 0; i < boundary.numVerts; i++) {
        if (i) {
            fputc(',', out);
        }
        LatLng point = {H3_EXPORT(degsToRads)(lats[i]),
                        H3_EXPORT(degsToRads)(lngs[i])};
        H3Index cell;
        must(H3_EXPORT(latLngToCell)(&point, interiorRes, &cell));
        printIndex(out, cell);
    }
    fputs("]}\n", out);
}

static void writeFloatEdgeSubject(FILE *out, H3Index h, Splitmix64 *rng) {
    H3Index edges[NUM_HEX_EDGES] = {0};
    must(H3_EXPORT(originToDirectedEdges)(h, edges));
    bool emitted[NUM_HEX_EDGES] = {false};
    for (int draw = 0; draw < FLOATS_DRAWS_PER_CELL; draw++) {
        int slot = (int)drawN(rng, NUM_HEX_EDGES);
        if (!edges[slot] || emitted[slot]) {
            continue;
        }
        emitted[slot] = true;
        H3Index edge = edges[slot];

        fputs("{\"index\":", out);
        printIndex(out, edge);
        double length;
        must(H3_EXPORT(edgeLengthRads)(edge, &length));
        printMeasure(out, "lengthRads", E_SUCCESS, length);
        must(H3_EXPORT(edgeLengthKm)(edge, &length));
        printMeasure(out, "lengthKm", E_SUCCESS, length);
        must(H3_EXPORT(edgeLengthM)(edge, &length));
        printMeasure(out, "lengthM", E_SUCCESS, length);
        CellBoundary boundary;
        must(H3_EXPORT(directedEdgeToBoundary)(edge, &boundary));
        fputs(",\"boundary\":", out);
        printLoop(out, boundary.verts, boundary.numVerts);
        fputs("}\n", out);
    }
}

static void writeFloatVertexSubject(FILE *out, H3Index h, Splitmix64 *rng) {
    H3Index vertexes[NUM_VERTEX_NUMBERS] = {0};
    must(H3_EXPORT(cellToVertexes)(h, vertexes));
    bool emitted[NUM_VERTEX_NUMBERS] = {false};
    for (int draw = 0; draw < FLOATS_DRAWS_PER_CELL; draw++) {
        int slot = (int)drawN(rng, NUM_VERTEX_NUMBERS);
        if (!vertexes[slot] || emitted[slot]) {
            continue;
        }
        emitted[slot] = true;
        LatLng point;
        must(H3_EXPORT(vertexToLatLng)(vertexes[slot], &point));
        fputs("{\"index\":", out);
        printIndex(out, vertexes[slot]);
        fputs(",\"latLng\":", out);
        printLatLng(out, point);
        fputs("}\n", out);
    }
}

static void writeDistanceRecord(FILE *out, H3Index origin, H3Index target) {
    LatLng a;
    LatLng b;
    must(H3_EXPORT(cellToLatLng)(origin, &a));
    must(H3_EXPORT(cellToLatLng)(target, &b));
    fputs("{\"index\":", out);
    printIndex(out, origin);
    fputs(",\"target\":", out);
    printIndex(out, target);
    fputs(",\"a\":", out);
    printLatLng(out, a);
    fputs(",\"b\":", out);
    printLatLng(out, b);
    printMeasure(out, "rads", E_SUCCESS,
                 H3_EXPORT(greatCircleDistanceRads)(&a, &b));
    printMeasure(out, "km", E_SUCCESS,
                 H3_EXPORT(greatCircleDistanceKm)(&a, &b));
    printMeasure(out, "m", E_SUCCESS, H3_EXPORT(greatCircleDistanceM)(&a, &b));
    fputs("}\n", out);
}

/* nearlyAntipodal reports whether the centers of two cells are within
 * FLOATS_ANTIPODE_MARGIN radians of being antipodal. The haversine formula
 * is ill-conditioned there, since asin has infinite slope at 1, so a last-bit
 * difference in the inputs moves the result by far more than the relative
 * tolerance; such pairs are not sampled. */
static bool nearlyAntipodal(H3Index origin, H3Index target) {
    LatLng a;
    LatLng b;
    must(H3_EXPORT(cellToLatLng)(origin, &a));
    must(H3_EXPORT(cellToLatLng)(target, &b));
    return H3_EXPORT(greatCircleDistanceRads)(&a, &b) >
           FLOATS_PI - FLOATS_ANTIPODE_MARGIN;
}

static void writeDistanceSubject(FILE *out, H3Index origin, Splitmix64 *rng) {
    H3Index near = drawDiskTarget(rng, origin, MAX_DISK_K);
    H3Index far = drawValidCell(rng, H3_EXPORT(getResolution)(origin));
    while (nearlyAntipodal(origin, far)) {
        far = drawValidCell(rng, H3_EXPORT(getResolution)(origin));
    }
    writeDistanceRecord(out, origin, near);
    if (far != near) {
        writeDistanceRecord(out, origin, far);
    }
}

/* writeFloatResolutionsFile writes the per-resolution averages for
 * resolutions 0..15 and the error row for 16. */
static void writeFloatResolutionsFile(const char *outDir, FileInfo *info) {
    char *path = joinPath(outDir, info->name);
    FILE *out = createFile(path);
    for (int res = 0; res <= MAX_RES + 1; res++) {
        fprintf(out, "{\"res\":%d", res);
        double value = 0;
        H3Error err = H3_EXPORT(getHexagonAreaAvgKm2)(res, &value);
        printMeasure(out, "hexagonAreaKm2", err, value);
        err = H3_EXPORT(getHexagonAreaAvgM2)(res, &value);
        printMeasure(out, "hexagonAreaM2", err, value);
        err = H3_EXPORT(getHexagonEdgeLengthAvgKm)(res, &value);
        printMeasure(out, "edgeLengthKm", err, value);
        err = H3_EXPORT(getHexagonEdgeLengthAvgM)(res, &value);
        printMeasure(out, "edgeLengthM", err, value);
        fputs("}\n", out);
    }
    if (fclose(out)) {
        fail("write");
    }
    describeFile(path, info);
    free(path);
}

typedef struct {
    FileInfo info;
    Sampler sample;
    RecordWriter write;
} IndexGroup;

static FILE *createFile(const char *path) {
    FILE *out = fopen(path, "wb");
    if (!out) {
        fprintf(
            stderr,
            "mkConformance: could not create %s (does its directory exist?)\n",
            path);
        exit(1);
    }
    return out;
}

static void writeIndexGroup(const char *outDir, uint64_t seed,
                            IndexGroup *group) {
    Splitmix64 rng = {seed};
    IndexList list = {0};
    group->sample(&list, &rng);

    char *path = joinPath(outDir, group->info.name);
    FILE *out = createFile(path);
    for (size_t i = 0; i < list.len; i++) {
        group->write(out, list.items[i], &rng);
    }
    if (fclose(out)) {
        fail("write");
    }
    free(list.items);

    describeFile(path, &group->info);
    free(path);
}

static void writeManifest(const char *outDir, const char *version,
                          uint64_t seed, const FileInfo *files,
                          size_t numFiles) {
    char *path = joinPath(outDir, "manifest.json");
    FILE *out = fopen(path, "wb");
    if (!out) {
        fail("could not create manifest.json");
    }
    fprintf(out,
            "{\n"
            "  \"format\": \"" FORMAT_VERSION
            "\",\n"
            "  \"h3Version\": \"%s\",\n"
            "  \"generator\": {\n"
            "    \"name\": \"" GENERATOR_NAME
            "\",\n"
            "    \"seed\": %" PRIu64
            "\n"
            "  },\n"
            "  \"tolerances\": {\n"
            "    \"angularDeg\": " ANGULAR_DEG_TOLERANCE
            ",\n"
            "    \"relative\": " RELATIVE_TOLERANCE
            "\n"
            "  },\n"
            "  \"files\": {\n",
            version, seed);
    for (size_t i = 0; i < numFiles; i++) {
        fprintf(out,
                "    \"%s\": {\n"
                "      \"records\": %ld,\n"
                "      \"sha256\": \"%s\"\n"
                "    }%s\n",
                files[i].name, files[i].records, files[i].sha256,
                i + 1 < numFiles ? "," : "");
    }
    fputs("  }\n}\n", out);
    if (fclose(out)) {
        fail("write");
    }
    free(path);
}

/* ---- group selection (-g) ---- */

#define MAX_SELECTED 32

static const char *selectedNames[MAX_SELECTED];
static int numSelected = 0;

/* selected reports whether a record file is part of this run: every file
 * when -g was not given, otherwise the files named by -g or whose directory
 * -g named. */
static bool selected(const char *file) {
    if (numSelected == 0) {
        return true;
    }
    const char *slash = strchr(file, '/');
    size_t dirLen = slash ? (size_t)(slash - file) : strlen(file);
    for (int i = 0; i < numSelected; i++) {
        const char *name = selectedNames[i];
        if (!strcmp(name, file)) {
            return true;
        }
        if (strlen(name) == dirLen && !strncmp(name, file, dirLen)) {
            return true;
        }
    }
    return false;
}

static void selectGroup(const char *name) {
    if (numSelected == MAX_SELECTED) {
        fail("too many -g options");
    }
    selectedNames[numSelected++] = name;
}

/* ---- curated files (conformance.md, "Curated files") ---- */

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define NUM_CURATED_FILES 6
#define MAX_CURATED_VERTS 16

/* Indexes from the reference's own tests and command-line fixtures, plus
 * hand-built traps, in the order README.md lists them. */
static const uint64_t CURATED_INSPECTION[] = {
    UINT64_C(0x85283473fffffff),  UINT64_C(0x85283473ffff),
    UINT64_C(0x115283473fffffff), UINT64_C(0x22528340bfffffff),
    UINT64_C(0x8f754e64992d6d6),  UINT64_C(0x81743ffffffffff),
    UINT64_C(0x8928342e20fffff),  UINT64_C(0x5),
    UINT64_C(0x200f202020202020), UINT64_C(0x80c3fffffffffff),
    UINT64_C(0x8f283080dcb0ae2),  UINT64_C(0x88283080ddfffff),
    UINT64_C(0x81083ffffffffff),  UINT64_C(0x81087ffffffffff),
    UINT64_C(0x82080ffffffffff),  UINT64_C(0x8029fffffffffff),
    UINT64_C(0x85283472fffffff),
};

static const uint64_t CURATED_HIERARCHY[] = {
    UINT64_C(0x88283080ddfffff), UINT64_C(0x8f283080dcb0ae2),
    UINT64_C(0x81083ffffffffff), UINT64_C(0x820807fffffffff),
    UINT64_C(0x85283473fffffff), UINT64_C(0x80c3fffffffffff),
    UINT64_C(0x8928342e20fffff), UINT64_C(0x8029fffffffffff),
};

typedef struct {
    uint64_t origin;
    uint64_t target;
} CuratedPair;

static const CuratedPair CURATED_TRAVERSAL[] = {
    {UINT64_C(0x85283473fffffff), UINT64_C(0x8528342bfffffff)},
    {UINT64_C(0x85283473fffffff), UINT64_C(0x85291ac7fffffff)},
    {UINT64_C(0x85283473fffffff), UINT64_C(0x85283477fffffff)},
    {UINT64_C(0x85283473fffffff), UINT64_C(0x85283472fffffff)},
    {UINT64_C(0x85285aa7fffffff), UINT64_C(0x851d9b1bfffffff)},
    {UINT64_C(0x820807fffffffff), UINT64_C(0x8208e7fffffffff)},
    {UINT64_C(0x8411b61ffffffff), UINT64_C(0x84016d3ffffffff)},
    {UINT64_C(0x820c4ffffffffff), UINT64_C(0x821ce7fffffffff)},
    {UINT64_C(0x832830fffffffff), UINT64_C(0x822837fffffffff)},
    {UINT64_C(0x832830fffffffff), UINT64_C(0x832834fffffffff)},
    {UINT64_C(0x8029fffffffffff), UINT64_C(0x8051fffffffffff)},
    {UINT64_C(0x80c3fffffffffff), UINT64_C(0x80c3fffffffffff)},
    {UINT64_C(0x81083ffffffffff), UINT64_C(0x8108bffffffffff)},
    {UINT64_C(0x8f283080dcb0ae2), UINT64_C(0x8f283080dcb0ae2)},
};

static const CuratedPair CURATED_LOCALIJ[] = {
    {UINT64_C(0x85283473fffffff), UINT64_C(0x8528342bfffffff)},
    {UINT64_C(0x8029fffffffffff), UINT64_C(0x8029fffffffffff)},
    {UINT64_C(0x8029fffffffffff), UINT64_C(0x8051fffffffffff)},
    {UINT64_C(0x820897fffffffff), UINT64_C(0x821f67fffffffff)},
    {UINT64_C(0x85283473fffffff), UINT64_C(0x85291ac7fffffff)},
    {UINT64_C(0x832830fffffffff), UINT64_C(0x832834fffffffff)},
    {UINT64_C(0x820807fffffffff), UINT64_C(0x8208e7fffffffff)},
    {UINT64_C(0x80c3fffffffffff), UINT64_C(0x80c3fffffffffff)},
    {UINT64_C(0x81083ffffffffff), UINT64_C(0x8108bffffffffff)},
    {UINT64_C(0x85285aa7fffffff), UINT64_C(0x851d9b1bfffffff)},
    {UINT64_C(0x85283473fffffff), UINT64_C(0x85283472fffffff)},
    {UINT64_C(0x200f202020202020), UINT64_C(0x85283473fffffff)},
};

typedef struct {
    const char *name;
    int uncompactRes;
    /* Space-separated lowercase hex, or NULL for every resolution 0 cell. */
    const char *cells;
} CuratedSet;

static const CuratedSet CURATED_SETS[] = {
    {"compact_test1", 6,
     "85283473fffffff 85283447fffffff 8528347bfffffff 85283463fffffff "
     "85283477fffffff "
     "8528340ffffffff 8528340bfffffff 85283457fffffff 85283443fffffff "
     "8528344ffffffff "
     "852836b7fffffff 8528346bfffffff 8528346ffffffff 85283467fffffff "
     "8528342bfffffff "
     "8528343bfffffff 85283407fffffff 85283403fffffff 8528341bfffffff"},
    {"multipolygon_test3", 6,
     "8528340bfffffff 85283447fffffff 8528347bfffffff 85283463fffffff "
     "85283477fffffff "
     "8528340ffffffff"},
    {"multipolygon_test4", 6,
     "8528340bfffffff 85283447fffffff 8528347bfffffff 85283463fffffff "
     "85283477fffffff "
     "8528340ffffffff 8528840ffffffff"},
    {"multipolygon_test5", 6,
     "85283473fffffff 85283447fffffff 8528347bfffffff 85283463fffffff "
     "85283477fffffff "
     "8528340ffffffff 8528340bfffffff 85283457fffffff 85283443fffffff "
     "8528344ffffffff "
     "852836b7fffffff 8528346bfffffff 8528346ffffffff 85283467fffffff "
     "8528342bfffffff "
     "8528343bfffffff 85283407fffffff 85283403fffffff 8528341bfffffff "
     "852834cffffffff "
     "85283453fffffff 8528345bfffffff 8528344bfffffff 852836b3fffffff "
     "852836a3fffffff "
     "852836a7fffffff 852830d3fffffff 852830d7fffffff 8528309bfffffff "
     "85283093fffffff "
     "8528342ffffffff 85283423fffffff 85283433fffffff 852834abfffffff "
     "85283417fffffff "
     "85283413fffffff 852834c7fffffff 852834c3fffffff 852834cbfffffff "
     "8529a927fffffff "
     "8529a92ffffffff 85283697fffffff 85283687fffffff 852836bbfffffff "
     "852836abfffffff "
     "852836affffffff 852830dbfffffff 852830c3fffffff 852830c7fffffff "
     "8528308bfffffff "
     "85283083fffffff 85283097fffffff 8528355bfffffff 85283427fffffff "
     "85283437fffffff "
     "852834affffffff 852834a3fffffff 852834bbfffffff 8528348ffffffff "
     "8528348bfffffff "
     "852834d7fffffff 852834d3fffffff 852834dbfffffff 8529a937fffffff "
     "8529a923fffffff "
     "8529a92bfffffff 85283693fffffff 85283683fffffff 8528368ffffffff "
     "85283617fffffff "
     "85283607fffffff 85283633fffffff 85283637fffffff 852830cbfffffff "
     "852830cffffffff "
     "8528301bfffffff 85283013fffffff 8528308ffffffff 85283087fffffff "
     "8528354bfffffff "
     "85283543fffffff 85283553fffffff 852835cbfffffff 852835dbfffffff "
     "852834a7fffffff "
     "852834b7fffffff 852834b3fffffff 85283487fffffff 85283483fffffff "
     "8528349bfffffff "
     "85291a6ffffffff 85291a6bfffffff 8529a9a7fffffff 8529a9affffffff "
     "8529a933fffffff "
     "8529a93bfffffff 8529a977fffffff 8529a967fffffff 8528369bfffffff "
     "8528368bfffffff "
     "85283613fffffff 85283603fffffff 8528360ffffffff 8528363bfffffff "
     "85283623fffffff "
     "85283627fffffff 85283053fffffff 85283057fffffff 8528300bfffffff "
     "85283003fffffff "
     "85283017fffffff 852830bbfffffff 852830b3fffffff 8528354ffffffff "
     "85283547fffffff "
     "85283557fffffff 852835cffffffff 852835c3fffffff 852835d3fffffff "
     "85291b6bfffffff "
     "85291b7bfffffff 85291b4ffffffff 85291b4bfffffff 85283497fffffff "
     "85283493fffffff "
     "85291a67fffffff 85291a63fffffff"},
    {"pentagon_children", 3,
     "820807fffffffff 820817fffffffff 82081ffffffffff 820827fffffffff "
     "82082ffffffffff "
     "820837fffffffff"},
    {"res0", 1, NULL},
};

/* Polygon vertices as lat, lng pairs in degrees. Fixtures the reference
 * holds in radians were converted once with %.17g. */
static const double SF_VERTS[] = {
    37.813318999889439,  -122.40898669969356, 37.786630199906988,
    -122.38054369969613, 37.719806199904276,  -122.35447369969584,
    37.707613199904031,  -122.51234369969448, 37.783587199903444,
    -122.52471869969825, 37.815157199906039,  -122.47987669969707,
};
static const double SF_HOLE_VERTS[] = {
    37.786980199908015,  -122.44711969969569, 37.766410199904307,
    -122.45907769969834, 37.771068199906722,  -122.41370969969519,
};
static const double EMPTY_VERTS[] = {
    37.813318999889439,  -122.40898669968212, 37.813318999946738,
    -122.40898669968784, 37.813319000004029,  -122.40898669969356,
};
static const double PRIME_MERIDIAN_VERTS[] = {
    0.57295779513082323,  0.57295779513082323,  0.57295779513082323,
    -0.57295779513082323, -0.57295779513082323, -0.57295779513082323,
    -0.57295779513082323, 0.57295779513082323,
};
static const double TRANSMERIDIAN_VERTS[] = {
    0.57295779513082323,  -179.4270422048692,   0.57295779513082323,
    179.4270422048692,    -0.57295779513082323, 179.4270422048692,
    -0.57295779513082323, -179.4270422048692,
};
static const double TRANSMERIDIAN_HOLE_VERTS[] = {
    0.28647889756541162,  -179.71352110243458,  0.28647889756541162,
    179.71352110243458,   -0.28647889756541162, 179.71352110243458,
    -0.28647889756541162, -179.71352110243458,
};
static const double TRANSMERIDIAN_COMPLEX_VERTS[] = {
    5.729577951308233,   -179.99942704220487, 5.729577951308233,
    179.99942704220487,  2.8647889756541165,  168.54084409738351,
    -5.729577951308233,  179.99942704220487,  -5.729577951308233,
    -179.99942704220487, -2.8647889756541165, -168.54084409738351,
};
static const double H3_136_VERTS[] = {
    5.7691065215330903, 51.112259557918499, 5.7484366798711299,
    51.084428587095701, 5.7486856571813103, 51.084115610613701,
    5.7693966192324702, 51.112087787177096,
};
static const double H3_595_VERTS[] = {
    37.345793375368473, -121.53625488281249, 37.616407055779923,
    -121.9317626953125, 37.330856613297144,  -122.29980468749999,
    37.050793129806571, -121.904296875,
};
static const double H3JS_67_VERTS[] = {
    -33.13755119234615, -56.25,    -34.30714385628804, -56.25,
    -34.30714385628804, -57.65625, -33.13755119234615, -57.65625,
};
static const double H3JS_67_SECOND_VERTS[] = {
    -34.30714385628804, -57.65625, -35.4606699514953,  -57.65625,
    -35.4606699514953,  -59.0625,  -34.30714385628804, -59.0625,
};
static const double WEST_HEMISPHERE_VERTS[] = {-90, -180, 90,  -180,
                                               90,  0,    -90, 0};
static const double EAST_HEMISPHERE_VERTS[] = {-90, 0,   90,  0,
                                               90,  180, -90, 180};
static const double POINT_VERTS[] = {0, 0};
static const double LINE_VERTS[] = {0, 0, 57.295779513082323, 0};
static const double CLI_TRIANGLE_VERTS[] = {
    37.813318999983238,    -122.4089866999972145, 37.7198061999978478,
    -122.3544736999993603, 37.8151571999998453,   -122.4798767000009008,
};
static const double CLI_DECAGON_VERTS[] = {
    37.784046,   -122.427089, 37.772267,   -122.434586, 37.761736,
    -122.425769, 37.762982,   -122.409455, 37.752446,   -122.400640,
    37.753689,   -122.384324, 37.765468,   -122.376819, 37.776004,
    -122.385635, 37.774761,   -122.401954, 37.785293,   -122.410771,
};

typedef struct {
    const char *name;
    int res;
    int numVerts;
    const double *verts;
    int numHoleVerts;
    const double *holeVerts;
} CuratedPolygon;

static const CuratedPolygon CURATED_POLYGONS[] = {
    {"sf", 9, 6, SF_VERTS, 0, NULL},
    {"sf", 9, 6, SF_VERTS, 3, SF_HOLE_VERTS},
    {"empty", 9, 3, EMPTY_VERTS, 0, NULL},
    {"primeMeridian", 7, 4, PRIME_MERIDIAN_VERTS, 0, NULL},
    {"transmeridian", 7, 4, TRANSMERIDIAN_VERTS, 0, NULL},
    {"transmeridian", 7, 4, TRANSMERIDIAN_VERTS, 4, TRANSMERIDIAN_HOLE_VERTS},
    {"transmeridianInner", 7, 4, TRANSMERIDIAN_HOLE_VERTS, 0, NULL},
    {"transmeridianComplex", 4, 6, TRANSMERIDIAN_COMPLEX_VERTS, 0, NULL},
    {"h3_136", 13, 4, H3_136_VERTS, 0, NULL},
    {"h3_595", 5, 4, H3_595_VERTS, 0, NULL},
    {"h3js_67", 7, 4, H3JS_67_VERTS, 0, NULL},
    {"h3js_67_second", 7, 4, H3JS_67_SECOND_VERTS, 0, NULL},
    {"westHemisphere", 0, 4, WEST_HEMISPHERE_VERTS, 0, NULL},
    {"westHemisphere", 1, 4, WEST_HEMISPHERE_VERTS, 0, NULL},
    {"westHemisphere", 2, 4, WEST_HEMISPHERE_VERTS, 0, NULL},
    {"eastHemisphere", 0, 4, EAST_HEMISPHERE_VERTS, 0, NULL},
    {"eastHemisphere", 1, 4, EAST_HEMISPHERE_VERTS, 0, NULL},
    {"eastHemisphere", 2, 4, EAST_HEMISPHERE_VERTS, 0, NULL},
    {"point", 5, 1, POINT_VERTS, 0, NULL},
    {"line", 5, 2, LINE_VERTS, 0, NULL},
    {"cliTriangle", 7, 3, CLI_TRIANGLE_VERTS, 0, NULL},
    {"cliDecagon", 7, 10, CLI_DECAGON_VERTS, 0, NULL},
};

/* finishFile closes a record file and fills in its manifest entry. */
static void finishFile(FILE *out, char *path, FileInfo *info) {
    if (fclose(out)) {
        fail("write");
    }
    describeFile(path, info);
    free(path);
}

/* writeCuratedIndexes writes one record per index with a group's writer. The
 * generator is fresh and the writers used here draw nothing. */
static void writeCuratedIndexes(const char *outDir, uint64_t seed,
                                FileInfo *info, const uint64_t *indexes,
                                size_t count, RecordWriter write) {
    Splitmix64 rng = {seed};
    char *path = joinPath(outDir, info->name);
    FILE *out = createFile(path);
    for (size_t i = 0; i < count; i++) {
        write(out, indexes[i], &rng);
    }
    finishFile(out, path, info);
}

typedef void (*PairWriter)(FILE *out, H3Index origin, H3Index target);

/* writeCuratedPairs writes one record per origin and target pair. */
static void writeCuratedPairs(const char *outDir, FileInfo *info,
                              const CuratedPair *pairs, size_t count,
                              PairWriter write) {
    char *path = joinPath(outDir, info->name);
    FILE *out = createFile(path);
    for (size_t i = 0; i < count; i++) {
        write(out, pairs[i].origin, pairs[i].target);
    }
    finishFile(out, path, info);
}

/* parseCells appends the space-separated hex indexes in text. */
static void parseCells(const char *text, IndexList *list) {
    while (*text) {
        char *end;
        uint64_t index = strtoull(text, &end, 16);
        if (end == text) {
            fail("malformed curated cell list");
        }
        push(list, index);
        text = end;
        while (*text == ' ') {
            text++;
        }
    }
}

/* writeCuratedSets writes each named input set under the id curated:name. */
static void writeCuratedSets(const char *outDir, FileInfo *info) {
    char *path = joinPath(outDir, info->name);
    FILE *out = createFile(path);
    for (size_t i = 0; i < COUNT(CURATED_SETS); i++) {
        IndexList input = {0};
        if (CURATED_SETS[i].cells) {
            parseCells(CURATED_SETS[i].cells, &input);
        } else {
            H3Index res0[NUM_BASE_CELLS];
            must(H3_EXPORT(getRes0Cells)(res0));
            for (int j = 0; j < NUM_BASE_CELLS; j++) {
                push(&input, res0[j]);
            }
        }
        char id[64];
        snprintf(id, sizeof id, "curated:%s", CURATED_SETS[i].name);
        writeSetsRecordNamed(out, id, &input, CURATED_SETS[i].uncompactRes);
        free(input.items);
    }
    finishFile(out, path, info);
}

/* curatedLoop converts a degree table into a loop in radians. */
static void curatedLoop(const double *verts, int numVerts, LatLng *loop) {
    for (int i = 0; i < numVerts; i++) {
        loop[i].lat = H3_EXPORT(degsToRads)(verts[2 * i]);
        loop[i].lng = H3_EXPORT(degsToRads)(verts[2 * i + 1]);
    }
}

/* writeCuratedPolygons writes each named polygon under the id name:res, with
 * :hole appended when it has one. */
static void writeCuratedPolygons(const char *outDir, FileInfo *info) {
    char *path = joinPath(outDir, info->name);
    FILE *out = createFile(path);
    for (size_t i = 0; i < COUNT(CURATED_POLYGONS); i++) {
        const CuratedPolygon *entry = &CURATED_POLYGONS[i];
        LatLng outer[MAX_CURATED_VERTS];
        LatLng inner[MAX_CURATED_VERTS];
        curatedLoop(entry->verts, entry->numVerts, outer);
        GeoLoop hole = {entry->numHoleVerts, inner};
        GeoPolygon polygon = {{entry->numVerts, outer}, 0, NULL};
        if (entry->numHoleVerts) {
            curatedLoop(entry->holeVerts, entry->numHoleVerts, inner);
            polygon.numHoles = 1;
            polygon.holes = &hole;
        }
        char id[64];
        snprintf(id, sizeof id, "%s:%d%s", entry->name, entry->res,
                 entry->numHoleVerts ? ":hole" : "");
        writeRegionsRecordNamed(out, id, &polygon, entry->res);
    }
    finishFile(out, path, info);
}

/* writeCuratedFiles writes the six curated files and their manifest entries.
 */
static void writeCuratedFiles(const char *outDir, uint64_t seed,
                              FileInfo *files) {
    files[0] = (FileInfo){"inspection/curated.jsonl", 0, ""};
    if (selected(files[0].name)) {
        writeCuratedIndexes(outDir, seed, &files[0], CURATED_INSPECTION,
                            COUNT(CURATED_INSPECTION), writeInspectionRecord);
    }
    files[1] = (FileInfo){"hierarchy/curated.jsonl", 0, ""};
    if (selected(files[1].name)) {
        writeCuratedIndexes(outDir, seed, &files[1], CURATED_HIERARCHY,
                            COUNT(CURATED_HIERARCHY), writeHierarchyRecord);
    }
    files[2] = (FileInfo){"traversal/curated.jsonl", 0, ""};
    if (selected(files[2].name)) {
        writeCuratedPairs(outDir, &files[2], CURATED_TRAVERSAL,
                          COUNT(CURATED_TRAVERSAL), writeTraversalPair);
    }
    files[3] = (FileInfo){"localij/curated.jsonl", 0, ""};
    if (selected(files[3].name)) {
        writeCuratedPairs(outDir, &files[3], CURATED_LOCALIJ,
                          COUNT(CURATED_LOCALIJ), writeLocalIJRecord);
    }
    files[4] = (FileInfo){"sets/curated.jsonl", 0, ""};
    if (selected(files[4].name)) {
        writeCuratedSets(outDir, &files[4]);
    }
    files[5] = (FileInfo){"regions/curated.jsonl", 0, ""};
    if (selected(files[5].name)) {
        writeCuratedPolygons(outDir, &files[5]);
    }
}

/* Every record file of the suite, in manifest order. */
static const char *const SUITE_FILES[] = {
    "inspection/cells.jsonl",    "hierarchy/cells.jsonl",
    "traversal/cells.jsonl",     "unsafe/cells.jsonl",
    "edges/cells.jsonl",         "vertexes/cells.jsonl",
    "localij/pairs.jsonl",       "sets/sets.jsonl",
    "regions/polygons.jsonl",    "floats/cells.jsonl",
    "floats/edges.jsonl",        "floats/vertexes.jsonl",
    "floats/distances.jsonl",    "floats/resolutions.jsonl",
    "digests/resolutions.jsonl", "digests/baseCells.jsonl",
    "digests/patterns.jsonl",    "inspection/curated.jsonl",
    "hierarchy/curated.jsonl",   "traversal/curated.jsonl",
    "localij/curated.jsonl",     "sets/curated.jsonl",
    "regions/curated.jsonl",
};

/* validateSelection fails on a -g name that matches neither a file nor a
 * directory of SUITE_FILES under the rules selected() applies, so that a
 * misspelled group cannot produce an empty run and an empty manifest. */
static void validateSelection(void) {
    for (int i = 0; i < numSelected; i++) {
        const char *name = selectedNames[i];
        bool known = false;
        for (size_t j = 0; j < COUNT(SUITE_FILES) && !known; j++) {
            const char *file = SUITE_FILES[j];
            const char *slash = strchr(file, '/');
            size_t dirLen = slash ? (size_t)(slash - file) : strlen(file);
            known = !strcmp(name, file) ||
                    (strlen(name) == dirLen && !strncmp(name, file, dirLen));
        }
        if (!known) {
            fprintf(stderr, "mkConformance: unknown group or file %s\n", name);
            exit(1);
        }
    }
}

/* writeManifestFromDisk describes every suite file as it is on disk and
 * writes the manifest, so that files produced by several runs (digests.sh)
 * are covered. */
static void writeManifestFromDisk(const char *outDir, const char *version,
                                  uint64_t seed) {
    FileInfo files[COUNT(SUITE_FILES)];
    size_t numFiles = 0;
    for (size_t i = 0; i < COUNT(SUITE_FILES); i++) {
        if (!selected(SUITE_FILES[i])) {
            continue;
        }
        files[numFiles] = (FileInfo){SUITE_FILES[i], 0, ""};
        char *path = joinPath(outDir, SUITE_FILES[i]);
        describeFile(path, &files[numFiles]);
        free(path);
        numFiles++;
    }
    writeManifest(outDir, version, seed, files, numFiles);
}

enum { MODE_SUITE, MODE_SLICE, MODE_ROW, MODE_MANIFEST };

static void usage(const char *program) {
    fprintf(stderr,
            "usage: %s -o <out-dir> [-s <seed>] [-g <group-or-file>]... "
            "[--manifest]\n"
            "       %s --digests <res> <first-base-cell> <last-base-cell>\n"
            "       %s --resolution-row <res> < base-cell-rows\n",
            program, program, program);
    exit(1);
}

int main(int argc, char *argv[]) {
    const char *outDir = NULL;
    char version[32];
    uint64_t seed = DEFAULT_SEED;
    int mode = MODE_SUITE;
    int res = 0, first = 0, last = 0;

    snprintf(version, sizeof version, "%d.%d.%d", H3_VERSION_MAJOR,
             H3_VERSION_MINOR, H3_VERSION_PATCH);

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-o") && i + 1 < argc) {
            outDir = argv[++i];
        } else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            seed = strtoull(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "-g") && i + 1 < argc) {
            selectGroup(argv[++i]);
        } else if (!strcmp(argv[i], "--manifest")) {
            mode = MODE_MANIFEST;
        } else if (!strcmp(argv[i], "--digests") && i + 3 < argc) {
            mode = MODE_SLICE;
            res = atoi(argv[++i]);
            first = atoi(argv[++i]);
            last = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--resolution-row") && i + 1 < argc) {
            mode = MODE_ROW;
            res = atoi(argv[++i]);
        } else {
            usage(argv[0]);
        }
    }
    if (res < 0 || res > DIGESTS_SLICE_MAX_RES || first < 0 || last < first ||
        last >= NUM_BASE_CELLS) {
        usage(argv[0]);
    }
    validateSelection();
    if (mode == MODE_SLICE) {
        writeDigestSlice(res, first, last);
        return 0;
    }
    if (mode == MODE_ROW) {
        writeResolutionRow(res);
        return 0;
    }
    if (!outDir) {
        usage(argv[0]);
    }
    if (mode == MODE_MANIFEST) {
        writeManifestFromDisk(outDir, version, seed);
        return 0;
    }

    IndexGroup groups[] = {
        {{"inspection/cells.jsonl", 0, ""},
         sampleInspection,
         writeInspectionRecord},
        {{"hierarchy/cells.jsonl", 0, ""},
         sampleHierarchy,
         writeHierarchyRecord},
        {{"traversal/cells.jsonl", 0, ""},
         sampleTraversal,
         writeTraversalRecord},
        {{"unsafe/cells.jsonl", 0, ""}, sampleUnsafe, writeUnsafeRecord},
        {{"edges/cells.jsonl", 0, ""}, sampleTraversal, writeEdgesRecord},
        {{"vertexes/cells.jsonl", 0, ""}, sampleTraversal, writeVertexesRecord},
        {{"localij/pairs.jsonl", 0, ""}, sampleTraversal, writeLocalIJSubject},
        {{"sets/sets.jsonl", 0, ""}, sampleSets, writeSetsSubject},
        {{"regions/polygons.jsonl", 0, ""}, sampleRegions, writeRegionsSubject},
        {{"floats/cells.jsonl", 0, ""}, sampleTraversal, writeFloatCellRecord},
        {{"floats/edges.jsonl", 0, ""}, sampleTraversal, writeFloatEdgeSubject},
        {{"floats/vertexes.jsonl", 0, ""},
         sampleTraversal,
         writeFloatVertexSubject},
        {{"floats/distances.jsonl", 0, ""},
         sampleTraversal,
         writeDistanceSubject},
    };
    size_t numGroups = sizeof groups / sizeof groups[0];
    FileInfo files[sizeof groups / sizeof groups[0] + 4 + NUM_CURATED_FILES];
    for (size_t i = 0; i < numGroups; i++) {
        if (selected(groups[i].info.name)) {
            writeIndexGroup(outDir, seed, &groups[i]);
        }
        files[i] = groups[i].info;
    }
    files[numGroups] = (FileInfo){"floats/resolutions.jsonl", 0, ""};
    if (selected(files[numGroups].name)) {
        writeFloatResolutionsFile(outDir, &files[numGroups]);
    }
    files[numGroups + 1] = (FileInfo){"digests/resolutions.jsonl", 0, ""};
    files[numGroups + 2] = (FileInfo){"digests/baseCells.jsonl", 0, ""};
    if (selected(files[numGroups + 1].name) ||
        selected(files[numGroups + 2].name)) {
        writeDigestsFiles(outDir, &files[numGroups + 1], &files[numGroups + 2]);
    }
    files[numGroups + 3] = (FileInfo){"digests/patterns.jsonl", 0, ""};
    if (selected(files[numGroups + 3].name)) {
        writePatternsFile(outDir, seed, &files[numGroups + 3]);
    }
    writeCuratedFiles(outDir, seed, &files[numGroups + 4]);
    if (numGroups + 4 + NUM_CURATED_FILES != COUNT(SUITE_FILES)) {
        fail("SUITE_FILES is out of step with the writers");
    }
    writeManifestFromDisk(outDir, version, seed);
    return 0;
}
