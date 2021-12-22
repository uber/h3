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
/** @file aflHarness.h
 * @brief   Adapter from LLVM fuzzer to AFL++
 */
#ifndef AFLHARNESS_H
#define AFLHARNESS_H

#include "utility.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

#define AFL_HARNESS_MAIN(expectedSize) int main(int argc, char *argv[]) {\
    if (argc != 2) {\
        error("Should have one argument (test case file)\n");\
    }\
    const char *filename = argv[1];\
    FILE *fp = fopen(filename, "rb");\
    uint8_t data[expectedSize];\
    if (fread(&data, expectedSize, 1, fp) != 1) {\
        error("Error reading\n");\
    }\
    fclose(fp);\
    return LLVMFuzzerTestOneInput(data, expectedSize);\
}

#endif // AFLHARNESS_H