/*
 * Copyright 2025 Uber Technologies, Inc.
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
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "adder.h"
#include "benchmark.h"
#include "constants.h"
#include "h3api.h"
#include "iterators.h"

static void doResSum(int res, bool print) {
    Adder adder = {};
    double cellArea;
    IterCellsResolution iter = iterInitRes(res);

    for (; iter.h; iterStepRes(&iter)) {
        H3_EXPORT(cellAreaRads2)(iter.h, &cellArea);
        kadd(&adder, cellArea);
    }

    double total_area = adder.sum;
    double diff = fabs(total_area - 4 * M_PI);
    if (print) {
        printf("res: %d, diff: %e\n", res, diff);
    }
}

BEGIN_BENCHMARKS();

static int MAX_RES = 3;

BENCHMARK(allCellsAtRes_print, 1, {
    for (int i = 0; i <= MAX_RES; i++) {
        doResSum(i, true);
    }
});

BENCHMARK(allCellsAtRes_noprint, 10, {
    for (int i = 0; i <= MAX_RES; i++) {
        doResSum(i, false);
    }
});

END_BENCHMARKS();
