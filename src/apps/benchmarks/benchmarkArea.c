#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#include "adder.h"
#include "benchmark.h"
#include "h3api.h"
#include "iterators.h"
#include "utility.h"

static void do_res_sum(int res) {
    Adder adder = {0, 0};
    double cellArea;
    IterCellsResolution iter = iterInitRes(res);

    for (; iter.h; iterStepRes(&iter)) {
        H3_EXPORT(cellAreaRads2)(iter.h, &cellArea);
        kadd(&adder, cellArea);
    }

    double total_area = adder.sum;
    double diff = fabs(total_area - 4 * M_PI);
    printf("res: %d, diff: %e\n", res, diff);
}

BEGIN_BENCHMARKS();

BENCHMARK(directedEdgeToBoundary, 1, {
    int MAX_RES = 6;
    for (int i = 0; i <= MAX_RES; i++) {
        do_res_sum(i);
    }
});

END_BENCHMARKS();
