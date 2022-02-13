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
 * @brief Fuzzer program for h3ToString and stringToH3
 */

#include "aflHarness.h"
#include "h3api.h"
#include "utility.h"

#define STRING_LENGTH 32

typedef struct {
    H3Index index;
    char str[STRING_LENGTH];
} inputArgs;

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < sizeof(inputArgs)) {
        return 0;
    }
    inputArgs args;
    // Copy the input data array since we need to modify it on the next line
    // to ensure it is zero terminated. (We are not interested in non-zero
    // terminated bugs since that fails the contract.)
    memcpy(&args, data, sizeof(inputArgs));
    args.str[STRING_LENGTH - 1] = 0;

    char str[STRING_LENGTH];
    H3_EXPORT(h3ToString)(args.index, str, STRING_LENGTH);
    H3Index index;
    H3_EXPORT(stringToH3)(args.str, &index);

    return 0;
}

AFL_HARNESS_MAIN(sizeof(inputArgs));
