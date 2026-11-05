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
/** @file conformance.c
 * @brief Shared code for the conformance suite runners.
 */

#include "conformance.h"

#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

/** The file being read, for failure messages. */
static ConformanceFile *current = NULL;

/** How much of a failing record to quote. */
#define QUOTE_LENGTH 200

static const char *const ERROR_NAMES[] = {
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
#define NUM_ERROR_NAMES (sizeof(ERROR_NAMES) / sizeof(ERROR_NAMES[0]))

void conformanceFail(const char *format, ...) {
    if (current) {
        fprintf(stderr, "%s:%ld: ", current->path, current->lineNumber);
    }
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    if (current && current->line) {
        size_t len = strlen(current->line);
        while (len > 0 && (current->line[len - 1] == '\n' ||
                           current->line[len - 1] == '\r')) {
            len--;
        }
        fprintf(stderr, "  record: %.*s%s\n",
                (int)(len < QUOTE_LENGTH ? len : QUOTE_LENGTH), current->line,
                len > QUOTE_LENGTH ? "..." : "");
    }
    exit(1);
}

void conformanceOpen(ConformanceFile *f, const char *path) {
    memset(f, 0, sizeof *f);
    f->path = path;
    f->file = fopen(path, "rb");
    current = f;
    if (!f->file) {
        conformanceFail("could not open record file");
    }
    f->capacity = 4096;
    f->line = malloc(f->capacity);
    if (!f->line) {
        conformanceFail("out of memory");
    }
}

/** Reads one line of any length into f->line. Returns false at end of
 * file. */
static bool readLine(ConformanceFile *f) {
    size_t len = 0;
    f->line[0] = '\0';
    while (fgets(f->line + len, (int)(f->capacity - len), f->file)) {
        len += strlen(f->line + len);
        if (len > 0 && f->line[len - 1] == '\n') {
            return true;
        }
        if (len + 1 >= f->capacity) {
            f->capacity *= 2;
            char *grown = realloc(f->line, f->capacity);
            if (!grown) {
                conformanceFail("out of memory");
            }
            f->line = grown;
        }
    }
    if (ferror(f->file)) {
        conformanceFail("read error");
    }
    if (len > 0) {
        // Every line, including the last, ends in a newline.
        f->lineNumber++;
        conformanceFail("last line is not terminated");
    }
    return false;
}

cJSON *conformanceNext(ConformanceFile *f) {
    if (f->record) {
        cJSON_Delete(f->record);
        f->record = NULL;
    }
    if (!readLine(f)) {
        return NULL;
    }
    f->lineNumber++;
    f->record = cJSON_Parse(f->line);
    if (!cJSON_IsObject(f->record)) {
        conformanceFail("line is not a JSON object");
    }
    return f->record;
}

void conformanceClose(ConformanceFile *f) {
    if (f->record) {
        cJSON_Delete(f->record);
        f->record = NULL;
    }
    free(f->line);
    f->line = NULL;
    if (f->file) {
        fclose(f->file);
        f->file = NULL;
    }
    current = NULL;
}

int conformanceMain(int argc, char *argv[], void (*check)(const cJSON *)) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <record-file>\n", argv[0]);
        return 1;
    }
    ConformanceFile f;
    conformanceOpen(&f, argv[1]);
    long records = 0;
    const cJSON *record;
    while ((record = conformanceNext(&f))) {
        check(record);
        records++;
    }
    conformanceClose(&f);
    printf("%s: %ld records match\n", argv[1], records);
    return 0;
}

/* ---- JSON accessors ---- */

cJSON *jsonItem(const cJSON *object, const char *key) {
    cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);
    if (!item) {
        conformanceFail("%s: key missing", key);
    }
    return item;
}

int jsonInt(const cJSON *object, const char *key) {
    cJSON *item = jsonItem(object, key);
    if (!cJSON_IsNumber(item)) {
        conformanceFail("%s: not a number", key);
    }
    return item->valueint;
}

int64_t jsonInt64(const cJSON *object, const char *key) {
    cJSON *item = jsonItem(object, key);
    if (!cJSON_IsNumber(item)) {
        conformanceFail("%s: not a number", key);
    }
    return (int64_t)item->valuedouble;
}

bool jsonBool(const cJSON *object, const char *key) {
    cJSON *item = jsonItem(object, key);
    if (!cJSON_IsBool(item)) {
        conformanceFail("%s: not a boolean", key);
    }
    return cJSON_IsTrue(item);
}

double jsonDouble(const cJSON *object, const char *key) {
    cJSON *item = jsonItem(object, key);
    if (!cJSON_IsNumber(item)) {
        conformanceFail("%s: not a number", key);
    }
    return item->valuedouble;
}

const char *jsonString(const cJSON *object, const char *key) {
    cJSON *item = jsonItem(object, key);
    if (!cJSON_IsString(item) || !item->valuestring) {
        conformanceFail("%s: not a string", key);
    }
    return item->valuestring;
}

cJSON *jsonArray(const cJSON *object, const char *key) {
    cJSON *item = jsonItem(object, key);
    if (!cJSON_IsArray(item)) {
        conformanceFail("%s: not an array", key);
    }
    return item;
}

void jsonPoint(const cJSON *item, LatLng *out) {
    if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2) {
        conformanceFail("point is not a [lat, lng] pair");
    }
    const cJSON *lat = cJSON_GetArrayItem(item, 0);
    const cJSON *lng = cJSON_GetArrayItem(item, 1);
    if (!cJSON_IsNumber(lat) || !cJSON_IsNumber(lng)) {
        conformanceFail("point coordinates are not numbers");
    }
    out->lat = H3_EXPORT(degsToRads)(lat->valuedouble);
    out->lng = H3_EXPORT(degsToRads)(lng->valuedouble);
}

/* ---- errors ---- */

const char *errorName(H3Error err) {
    if (err < NUM_ERROR_NAMES) {
        return ERROR_NAMES[err];
    }
    return "E_UNKNOWN";
}

bool isErrorObject(const cJSON *item) {
    if (!cJSON_IsObject(item)) {
        return false;
    }
    const cJSON *err = cJSON_GetObjectItemCaseSensitive(item, "err");
    return cJSON_IsString(err) && err->valuestring;
}

bool expectResult(const cJSON *item, H3Error got, const char *what) {
    if (isErrorObject(item)) {
        const char *want =
            cJSON_GetObjectItemCaseSensitive(item, "err")->valuestring;
        if (strcmp(errorName(got), want) != 0) {
            conformanceFail("%s: got %s, want %s", what, errorName(got), want);
        }
        return true;
    }
    if (got) {
        conformanceFail("%s: got %s, want a value", what, errorName(got));
    }
    return false;
}

/* ---- indexes ---- */

void formatIndex(H3Index h, char out[17]) {
    if (H3_EXPORT(h3ToString)(h, out, 17)) {
        conformanceFail("h3ToString failed");
    }
}

H3Index parseIndex(const char *text) {
    H3Index h;
    if (H3_EXPORT(stringToH3)(text, &h)) {
        conformanceFail("stringToH3(%s) failed", text);
    }
    char back[17];
    formatIndex(h, back);
    if (strcmp(back, text) != 0) {
        conformanceFail("index %s is not canonical, formats as %s", text, back);
    }
    return h;
}

H3Index jsonIndex(const cJSON *object, const char *key) {
    return parseIndex(jsonString(object, key));
}

H3Index *parseIndexArray(const cJSON *array, int64_t *count) {
    if (!cJSON_IsArray(array)) {
        conformanceFail("expected an array of indexes");
    }
    *count = cJSON_GetArraySize(array);
    H3Index *out = calloc(*count ? (size_t)*count : 1, sizeof *out);
    if (!out) {
        conformanceFail("out of memory");
    }
    int64_t i = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, array) {
        if (!cJSON_IsString(item) || !item->valuestring) {
            conformanceFail("array entry %" PRId64 " is not an index string",
                            i);
        }
        out[i++] = parseIndex(item->valuestring);
    }
    return out;
}

static int compareIndexes(const void *a, const void *b) {
    H3Index x = *(const H3Index *)a;
    H3Index y = *(const H3Index *)b;
    return x < y ? -1 : x > y;
}

int64_t normalizeIndexSet(H3Index *indexes, int64_t count) {
    qsort(indexes, (size_t)count, sizeof *indexes, compareIndexes);
    int64_t kept = 0;
    for (int64_t i = 0; i < count; i++) {
        if (indexes[i] == 0) {
            continue;
        }
        if (kept > 0 && indexes[kept - 1] == indexes[i]) {
            continue;
        }
        indexes[kept++] = indexes[i];
    }
    return kept;
}

static void failIndexes(const char *what, const char *reason,
                        const H3Index *got, int64_t gotCount,
                        const H3Index *want, int64_t wantCount) {
    fprintf(stderr, "%s: %s\n  got (%" PRId64 "):", what, reason, gotCount);
    for (int64_t i = 0; i < gotCount; i++) {
        char buf[17];
        formatIndex(got[i], buf);
        fprintf(stderr, " %s", buf);
    }
    fprintf(stderr, "\n  want (%" PRId64 "):", wantCount);
    for (int64_t i = 0; i < wantCount; i++) {
        char buf[17];
        formatIndex(want[i], buf);
        fprintf(stderr, " %s", buf);
    }
    fputc('\n', stderr);
    conformanceFail("%s differs", what);
}

void assertIndex(const cJSON *item, H3Index got, const char *what) {
    if (!cJSON_IsString(item) || !item->valuestring) {
        conformanceFail("%s: not an index string", what);
    }
    H3Index want = parseIndex(item->valuestring);
    if (got != want) {
        char buf[17];
        formatIndex(got, buf);
        conformanceFail("%s: got %s, want %s", what, buf, item->valuestring);
    }
}

void assertIndexSet(const cJSON *array, H3Index *got, int64_t count,
                    const char *what) {
    int64_t wantCount;
    H3Index *want = parseIndexArray(array, &wantCount);
    wantCount = normalizeIndexSet(want, wantCount);
    count = normalizeIndexSet(got, count);
    if (count != wantCount ||
        memcmp(got, want, (size_t)count * sizeof *got) != 0) {
        failIndexes(what, "sets differ", got, count, want, wantCount);
    }
    free(want);
}

void assertIndexSequence(const cJSON *array, const H3Index *got, int64_t count,
                         const char *what) {
    int64_t wantCount;
    H3Index *want = parseIndexArray(array, &wantCount);
    if (count != wantCount ||
        memcmp(got, want, (size_t)count * sizeof *got) != 0) {
        failIndexes(what, "sequences differ", got, count, want, wantCount);
    }
    free(want);
}

/* ---- tolerances ---- */

bool withinAngular(const LatLng *got, const LatLng *want) {
    double separation = H3_EXPORT(greatCircleDistanceRads)(got, want);
    return separation <= H3_EXPORT(degsToRads)(CONFORMANCE_ANGULAR_DEG);
}

void assertPoint(const cJSON *item, const LatLng *got, const char *what) {
    LatLng want;
    jsonPoint(item, &want);
    if (!withinAngular(got, &want)) {
        conformanceFail(
            "%s: got [%.17g,%.17g], want [%.17g,%.17g], %.3g "
            "degrees apart",
            what, H3_EXPORT(radsToDegs)(got->lat),
            H3_EXPORT(radsToDegs)(got->lng), H3_EXPORT(radsToDegs)(want.lat),
            H3_EXPORT(radsToDegs)(want.lng),
            H3_EXPORT(radsToDegs)(
                H3_EXPORT(greatCircleDistanceRads)(got, &want)));
    }
}

void assertPointSequence(const cJSON *array, const LatLng *got, int count,
                         const char *what) {
    if (!cJSON_IsArray(array)) {
        conformanceFail("%s: not an array", what);
    }
    if (cJSON_GetArraySize(array) != count) {
        conformanceFail("%s: got %d points, want %d", what, count,
                        cJSON_GetArraySize(array));
    }
    int i = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, array) { assertPoint(item, &got[i++], what); }
}

bool withinMeasure(double got, double want, MeasureUnit unit) {
    double diff = fabs(got - want);
    if (diff <= CONFORMANCE_RELATIVE * fabs(want)) {
        return true;
    }
    double angular = H3_EXPORT(degsToRads)(CONFORMANCE_ANGULAR_DEG);
    double scale = 1;
    bool area = false;
    switch (unit) {
        case UNIT_RADS:
            break;
        case UNIT_KM:
            scale = EARTH_RADIUS_KM;
            break;
        case UNIT_M:
            scale = EARTH_RADIUS_KM * 1000;
            break;
        case UNIT_RADS2:
            area = true;
            break;
        case UNIT_KM2:
            scale = EARTH_RADIUS_KM;
            area = true;
            break;
        case UNIT_M2:
            scale = EARTH_RADIUS_KM * 1000;
            area = true;
            break;
    }
    double floor = angular * scale;
    if (area) {
        floor = angular * sqrt(fabs(want)) * scale;
    }
    return diff <= floor;
}

void assertMeasure(const cJSON *object, const char *key, H3Error err,
                   double got, MeasureUnit unit) {
    const cJSON *item = jsonItem(object, key);
    if (expectResult(item, err, key)) {
        return;
    }
    if (!cJSON_IsNumber(item)) {
        conformanceFail("%s: not a number", key);
    }
    if (!withinMeasure(got, item->valuedouble, unit)) {
        conformanceFail("%s: got %.17g, want %.17g", key, got,
                        item->valuedouble);
    }
}
