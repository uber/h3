/*
 * Copyright 2017 Uber Technologies, Inc.
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
/** @file benchmark.h
 * @brief Benchmark harness functions and macros.
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h>

#define BEGIN_BENCHMARKS()             \
    int main(int argc, char* argv[]) { \
        clock_t start;                 \
        clock_t diff;                  \
        double duration;               \
        int iterations;                \
        char* name;

#define BENCHMARK(NAME, ITERATIONS, BODY)                                    \
    start = clock();                                                         \
    iterations = ITERATIONS;                                                 \
    name = #NAME;                                                            \
    for (int i = 0; i < iterations; i++) {                                   \
        BODY;                                                                \
    }                                                                        \
    diff = clock() - start;                                                  \
    duration = diff * 1000 / CLOCKS_PER_SEC;                                 \
    printf("\t-- %s: %f milliseconds per iteration (%d iterations)\n", name, \
           duration / iterations, iterations);

#define END_BENCHMARKS() \
    ;                    \
    }

#endif
