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
/** @file conformance.h
 * @brief Shared code for the conformance suite runners: reading record
 * files, decoding the value conventions of dev-docs/conformance.md, and the
 * comparison rules for sets, sequences, points and measures.
 *
 * A runner is a test app that takes one record file on the command line,
 * calls `check` on each record and exits non-zero on the first record that
 * does not match the library. Every failure names the file, the line and
 * the key that differed, so that a line can be quoted as it is.
 */

#ifndef CONFORMANCE_H
#define CONFORMANCE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cJSON.h"
#include "h3api.h"

/** The specification version this runner implements. */
#define CONFORMANCE_FORMAT "h3-conformance/1"
/** Angular tolerance on coordinates, in degrees of arc. */
#define CONFORMANCE_ANGULAR_DEG 1e-12
/** Relative tolerance on lengths and areas. */
#define CONFORMANCE_RELATIVE 1e-9

/** The units of a length or area output, for the tolerance floor. */
typedef enum {
    UNIT_RADS,
    UNIT_KM,
    UNIT_M,
    UNIT_RADS2,
    UNIT_KM2,
    UNIT_M2
} MeasureUnit;

/** A record file being read: a line at a time, each parsed as JSON. */
typedef struct {
    const char *path;
    FILE *file;
    char *line;
    size_t capacity;
    long lineNumber;
    cJSON *record;
} ConformanceFile;

/** Opens a record file, exiting with a message on failure. */
void conformanceOpen(ConformanceFile *f, const char *path);
/** Returns the next record, or NULL at end of file. The previous record is
 * freed. A line that is not a JSON object is a failure. */
cJSON *conformanceNext(ConformanceFile *f);
/** Frees the current record and closes the file. */
void conformanceClose(ConformanceFile *f);

/** Prints the current file, line and record, then the message, and exits
 * with status 1. */
void conformanceFail(const char *format, ...);

/** Runs `check` on every record of the file named by argv[1] and returns the
 * process exit status. */
int conformanceMain(int argc, char *argv[], void (*check)(const cJSON *));

/** Shorthand for a conformance assertion with a message. */
#define c_assert(condition, message)                        \
    do {                                                    \
        if (!(condition)) {                                 \
            conformanceFail("%s: %s", message, #condition); \
        }                                                   \
    } while (0)

/* JSON accessors. Each fails when the key is missing or has the wrong type,
 * because every key of a group is present on every line. */
cJSON *jsonItem(const cJSON *object, const char *key);
int jsonInt(const cJSON *object, const char *key);
int64_t jsonInt64(const cJSON *object, const char *key);
bool jsonBool(const cJSON *object, const char *key);
double jsonDouble(const cJSON *object, const char *key);
const char *jsonString(const cJSON *object, const char *key);
cJSON *jsonArray(const cJSON *object, const char *key);
/** Decodes a `[latitude, longitude]` pair in degrees to radians. */
void jsonPoint(const cJSON *item, LatLng *out);

/** The H3Error enum name, as record files spell errors. */
const char *errorName(H3Error err);
/** Whether an item is an error object, `{"err": "E_..."}`. */
bool isErrorObject(const cJSON *item);
/** Compares a function's status with a result item. Returns true when the
 * item is an error object, after checking the function failed with that
 * error; returns false when the item is a value, after checking the function
 * succeeded. `what` names the key in failure messages. */
bool expectResult(const cJSON *item, H3Error got, const char *what);

/** Parses an index string and checks it formats back to the same string. */
H3Index parseIndex(const char *text);
/** Formats an index the way record files do. */
void formatIndex(H3Index h, char out[17]);
/** parseIndex on a key of an object. */
H3Index jsonIndex(const cJSON *object, const char *key);
/** Parses an array of index strings into a malloc'd buffer. */
H3Index *parseIndexArray(const cJSON *array, int64_t *count);

/** Checks an index against an index string item. */
void assertIndex(const cJSON *item, H3Index got, const char *what);
/** Checks a set of indexes: both sides are sorted with zero padding and
 * duplicates removed before comparison. `got` is reordered. */
void assertIndexSet(const cJSON *array, H3Index *got, int64_t count,
                    const char *what);
/** Checks a sequence of indexes in order. */
void assertIndexSequence(const cJSON *array, const H3Index *got, int64_t count,
                         const char *what);
/** Sorts indexes ascending, drops zeros and duplicates, returns the count. */
int64_t normalizeIndexSet(H3Index *indexes, int64_t count);

/** Whether two points, in radians, are within the angular tolerance. */
bool withinAngular(const LatLng *got, const LatLng *want);
/** Checks a point against a `[lat, lng]` item. */
void assertPoint(const cJSON *item, const LatLng *got, const char *what);
/** Checks a sequence of points against an array of `[lat, lng]` items. */
void assertPointSequence(const cJSON *array, const LatLng *got, int count,
                         const char *what);
/** Whether a measure is within the relative tolerance of the expected value
 * or within the absolute floor for its unit. */
bool withinMeasure(double got, double want, MeasureUnit unit);
/** Checks a measure, or its error, against a key of an object. */
void assertMeasure(const cJSON *object, const char *key, H3Error err,
                   double got, MeasureUnit unit);

#endif
