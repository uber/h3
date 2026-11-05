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
 * @brief Checks the conformance suite manifest (dev-docs/conformance.md).
 *
 *  usage: `testConformanceManifest <manifest.json>`
 *
 *  The manifest must name the format version this runner implements, the
 *  version of the library it is built against and the tolerances the
 *  runners apply, and every record file it lists must be on disk with the
 *  listed line count and SHA-256.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conformance.h"
#include "h3api.h"
#include "sha256.h"

/** Reads a whole file; exits on failure. */
static unsigned char *readFile(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        conformanceFail("could not open %s", path);
    }
    if (fseek(f, 0, SEEK_END)) {
        conformanceFail("seek %s", path);
    }
    long length = ftell(f);
    if (length < 0 || fseek(f, 0, SEEK_SET)) {
        conformanceFail("tell %s", path);
    }
    unsigned char *data = malloc((size_t)length + 1);
    if (!data) {
        conformanceFail("out of memory");
    }
    if (fread(data, 1, (size_t)length, f) != (size_t)length) {
        conformanceFail("read %s", path);
    }
    fclose(f);
    data[length] = '\0';
    *size = (size_t)length;
    return data;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <manifest.json>\n", argv[0]);
        return 1;
    }
    const char *path = argv[1];
    size_t size;
    unsigned char *text = readFile(path, &size);
    cJSON *manifest = cJSON_Parse((const char *)text);
    if (!cJSON_IsObject(manifest)) {
        conformanceFail("%s: not a JSON object", path);
    }
    free(text);

    c_assert(!strcmp(jsonString(manifest, "format"), CONFORMANCE_FORMAT),
             "format: not the version this runner implements");

    char version[32];
    snprintf(version, sizeof version, "%d.%d.%d", H3_VERSION_MAJOR,
             H3_VERSION_MINOR, H3_VERSION_PATCH);
    if (strcmp(jsonString(manifest, "h3Version"), version) != 0) {
        conformanceFail(
            "h3Version: manifest says %s, library is %s; regenerate the "
            "suite with mkConformance",
            jsonString(manifest, "h3Version"), version);
    }

    const cJSON *generator = jsonItem(manifest, "generator");
    c_assert(cJSON_IsObject(generator), "generator: not an object");
    c_assert(strlen(jsonString(generator, "name")) > 0,
             "generator.name: empty");
    c_assert((uint64_t)jsonInt64(generator, "seed") == CONFORMANCE_SEED,
             "generator.seed: not the seed the runners replay");

    const cJSON *tolerances = jsonItem(manifest, "tolerances");
    c_assert(cJSON_IsObject(tolerances), "tolerances: not an object");
    c_assert(jsonDouble(tolerances, "angularDeg") == CONFORMANCE_ANGULAR_DEG,
             "tolerances.angularDeg: not the value the runners apply");
    c_assert(jsonDouble(tolerances, "relative") == CONFORMANCE_RELATIVE,
             "tolerances.relative: not the value the runners apply");

    // Record files are relative to the manifest's directory.
    const char *slash = strrchr(path, '/');
#ifdef _WIN32
    const char *backslash = strrchr(path, '\\');
    if (!slash || (backslash && backslash > slash)) {
        slash = backslash;
    }
#endif
    size_t dirLen = slash ? (size_t)(slash - path + 1) : 0;

    const cJSON *files = jsonItem(manifest, "files");
    c_assert(cJSON_IsObject(files), "files: not an object");
    int checked = 0;
    const cJSON *entry;
    cJSON_ArrayForEach(entry, files) {
        c_assert(cJSON_IsObject(entry), "files: entry is not an object");
        size_t len = dirLen + strlen(entry->string) + 1;
        char *filePath = malloc(len);
        c_assert(filePath, "out of memory");
        snprintf(filePath, len, "%.*s%s", (int)dirLen, path, entry->string);

        size_t fileSize;
        unsigned char *data = readFile(filePath, &fileSize);
        long records = 0;
        for (size_t i = 0; i < fileSize; i++) {
            records += data[i] == '\n';
        }
        char hex[65];
        sha256Hex(data, fileSize, hex);
        free(data);

        if (records != (long)jsonInt64(entry, "records")) {
            conformanceFail("%s: %ld records, manifest says %" PRId64,
                            entry->string, records,
                            jsonInt64(entry, "records"));
        }
        if (strcmp(hex, jsonString(entry, "sha256")) != 0) {
            conformanceFail(
                "%s: sha256 %s, manifest says %s; regenerate "
                "the suite with mkConformance",
                entry->string, hex, jsonString(entry, "sha256"));
        }
        free(filePath);
        checked++;
    }
    c_assert(checked > 0, "files: manifest lists no record files");
    cJSON_Delete(manifest);
    printf("%s: %d record files match\n", path, checked);
    return 0;
}
