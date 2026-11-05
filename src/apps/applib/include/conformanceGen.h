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
/** @file conformanceGen.h
 * @brief The parts of the conformance suite generator that the runners
 * share with it: the pseudo-random source and index construction of
 * dev-docs/conformance.md, "Sampling", the scaled cell boundaries of the
 * regions group, and the digest streams of the digests group. Having one
 * implementation of the streams means the digests runners check the
 * library's current answers against the committed digests, while the
 * serialization itself is checked by regenerating the discrete files.
 */

#ifndef CONFORMANCE_GEN_H
#define CONFORMANCE_GEN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "h3api.h"
#include "sha256.h"

#define MAX_RES 15
#define NUM_BASE_CELLS 122
#define NUM_DIGITS 7
#define NUM_MODES 16
#define NUM_HEX_EDGES 6
#define NUM_PENTAGONS 12
#define MAX_INDEX_LEN 17

#define MODE_OFFSET 59
#define RES_OFFSET 52
#define BASE_CELL_OFFSET 45
#define RESERVED_OFFSET 56
#define DIGIT_BITS 3
#define DIGIT_MASK UINT64_C(7)
#define HIGH_BIT (UINT64_C(1) << 63)
#define CELL_MODE 1

#define DIGESTS_MAX_RES 6
#define DIGESTS_SLICE_MAX_RES 7
#define POLYGONS_MAX_RES 2
#define POLYGONS_FILL_STEP 2
#define NUM_CONTAINMENT_MODES 4
#define PATTERN_KINDS 8
#define PATTERN_TIERS 3
#define NUM_ICOSA_FACES 20
#define REGIONS_SHRINK 0.8
#define REGIONS_EXPAND 1.2
#define MAX_DISK_K 3
#define NUM_VERTEX_NUMBERS 6

/** The H3Error enum name, as record files and digest streams spell it. */
const char *conformanceErrorName(H3Error err);

/* ---- pseudo-random source (conformance.md, "Pseudo-random source") ---- */

typedef struct {
    uint64_t state;
} Splitmix64;

uint64_t draw(Splitmix64 *rng);
int drawN(Splitmix64 *rng, int n);

/* ---- index construction (conformance.md, "Index construction") ---- */

unsigned digitShift(int r);
uint64_t buildIndex(int mode, int res, int baseCell, const int *digits);
uint64_t setField(uint64_t index, unsigned offset, unsigned width,
                  uint64_t value);
uint64_t drawCell(Splitmix64 *rng, int res);
uint64_t drawValidCell(Splitmix64 *rng, int res);

/** qsort comparators for indexes (unsigned) and ints. */
int compareIndexes(const void *a, const void *b);
int compareInts(const void *a, const void *b);

/** maxGridDiskSize(k) for small k. */
int diskSize(int k);

/* ---- scaled boundaries (conformance.md, "regions/polygons.jsonl") ---- */

double wrapDegrees(double lng);
void scaledBoundary(H3Index h, double factor, CellBoundary *loop);

/* ---- digest streams (conformance.md, "digests") ---- */

/** One text line of a digest stream, built field by field. */
typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} Line;

void lineAppend(Line *line, const char *text);
void lineStart(Line *line, H3Index h);
void lineIndex(Line *line, H3Index h);
void lineInt(Line *line, int64_t value);
void lineError(Line *line, H3Error err);
int64_t lineIndexSet(Line *line, H3Index *indexes, int64_t count);

/** The digest streams, in the order their keys are written. */
enum {
    S_CELLS,
    S_CHILDREN,
    S_COMPACT,
    S_DISKS,
    S_DISTANCES,
    S_EDGES,
    S_INSPECTION,
    S_LOCAL_IJ,
    S_PARENTS,
    S_POLYGONS,
    S_RINGS,
    S_ROUND_TRIP,
    S_VERTEXES,
    NUM_STREAMS
};

extern const char *const STREAM_KEYS[NUM_STREAMS];
/** The pattern counts at which digests/patterns.jsonl records a digest. */
extern const int64_t PATTERN_COUNTS[PATTERN_TIERS];

/** Whether the polygons stream exists at a resolution. */
bool hasPolygons(int res);
/** Appends one cell's line to every stream. */
void digestCell(Sha256 *streams, Line *line, H3Index h, int res);
/** Writes each stream's hex digest. */
void finalizeStreams(Sha256 *streams, char hex[NUM_STREAMS][65]);
/** Runs every stream over the cells of res descending from baseCell, in
 * ascending order, and returns their count. */
int64_t digestBaseCell(int res, int baseCell, Line *line,
                       char hex[NUM_STREAMS][65]);

/** Accumulates one digests/resolutions.jsonl line: per stream, the SHA-256
 * over the 122 per-base-cell hex digests of that stream in base-cell
 * order. */
typedef struct {
    Sha256 streams[NUM_STREAMS];
    int64_t count;
    int next;
} ResolutionDigest;

void resolutionInit(ResolutionDigest *digest);
/** Absorbs one base cell's digests, which must arrive in base-cell order. */
void resolutionAdd(ResolutionDigest *digest, int res, int baseCell,
                   int64_t count, char hex[NUM_STREAMS][65]);

/** Draws one 64-bit word for the validity stream
 * (conformance.md, "digests/patterns.jsonl"). */
uint64_t drawPattern(Splitmix64 *rng);

#endif
