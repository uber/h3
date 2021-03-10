/*
 * Copyright 2021 Uber Technologies, Inc.
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
 * @brief Fuzzer program for gridDisk
 */

#include "h3api.h"
#include "utility.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        error("Should have one argument (test case file)\n");
    }
    const char* filename = argv[1];
    FILE* fp = fopen(filename, "rb");
    struct {
        H3Index index;
        int k;
    } args;
    if (fread(&args, sizeof(args), 1, fp) != 1) {
        error("Error reading\n");
    }
    fclose(fp);

    int sz = H3_EXPORT(maxGridDiskSize)(args.k);
    H3Index* results = calloc(sizeof(H3Index), sz);
    if (results != NULL) {
        H3_EXPORT(gridDisk)(args.index, args.k, results);
        h3Println(results[0]);
    }
    free(results);
}
