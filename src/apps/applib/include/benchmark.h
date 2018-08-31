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

#include <math.h>
#include <stdio.h>

#define MICROSECONDS_PER_SECOND 1E6
#define NANOSECONDS_PER_SECOND 1E9
#define NANOSECONDS_PER_MICROSECOND 1E3

#ifdef _WIN32

#include <Windows.h>

#define START_TIMER                   \
    LARGE_INTEGER start;              \
    LARGE_INTEGER freq;               \
    QueryPerformanceFrequency(&freq); \
    QueryPerformanceCounter(&start)

#define END_TIMER(var)                                                       \
    LARGE_INTEGER end;                                                       \
    QueryPerformanceCounter(&end);                                           \
    const long double var = ((long double)(end.QuadPart - start.QuadPart)) / \
                            freq.QuadPart * MICROSECONDS_PER_SECOND

#else  // !defined(_WIN32)

#include <time.h>

#define START_TIMER        \
    struct timespec start; \
    clock_gettime(CLOCK_MONOTONIC, &start)

#define END_TIMER(var)                                                \
    struct timespec end;                                              \
    clock_gettime(CLOCK_MONOTONIC, &end);                             \
    struct timespec elapsed;                                          \
    elapsed.tv_nsec = end.tv_nsec - start.tv_nsec;                    \
    elapsed.tv_sec = end.tv_sec - start.tv_sec;                       \
    if (elapsed.tv_nsec < 0) {                                        \
        elapsed.tv_sec--;                                             \
        elapsed.tv_nsec = NANOSECONDS_PER_SECOND + elapsed.tv_nsec;   \
    }                                                                 \
    const long double var =                                           \
        (elapsed.tv_sec * NANOSECONDS_PER_SECOND + elapsed.tv_nsec) / \
        NANOSECONDS_PER_MICROSECOND

#endif

#define BEGIN_BENCHMARKS() int main(int argc, char* argv[]) {
#define BENCHMARK(NAME, ITERATIONS, BODY)                                   \
    do {                                                                    \
        const int iterations = ITERATIONS;                                  \
        const char* name = #NAME;                                           \
        START_TIMER;                                                        \
        for (int i = 0; i < iterations; i++) {                              \
            BODY;                                                           \
        }                                                                   \
        END_TIMER(duration);                                                \
        printf("\t-- %s: %Lf microseconds per iteration (%d iterations)\n", \
               name, (duration / iterations), iterations);                  \
    } while (0)
#define END_BENCHMARKS() }

#endif
