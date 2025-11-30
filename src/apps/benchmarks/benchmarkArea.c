#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "adder.h"
#include "benchmark.h"
#include "constants.h"
#include "h3api.h"
#include "iterators.h"

static void doResSum(int res, bool print) {
    Adder adder = {0};
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

static int MAX_RES = 4;

BENCHMARK(allCellsAtRes_print, 1, {
    for (int i = 0; i <= MAX_RES; i++) {
        doResSum(i, true);
    }
});

BENCHMARK(allCellsAtRes_noprint, 100, {
    for (int i = 0; i <= MAX_RES; i++) {
        doResSum(i, false);
    }
});

END_BENCHMARKS();
