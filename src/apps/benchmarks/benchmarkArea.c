#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#include "adder.h"
#include "benchmark.h"
#include "h3api.h"
#include "iterators.h"
#include "utility.h"

// TOOD: demonstrate the area alg works for global polygons.

// TODO: is kadd faster without pointers? k = kadd(k, cellArea);

static void do_res_sum(int res) {
    Adder adder = {0, 0};
    double cellArea;
    IterCellsResolution iter = iterInitRes(res);

    for (; iter.h; iterStepRes(&iter)) {
        H3_EXPORT(cellAreaRads2)(iter.h, &cellArea);
        kadd(&adder, cellArea);
    }

    double total_area = kresult(adder);
    double diff = fabs(total_area - 4 * M_PI);
    printf("res: %d, diff: %e\n", res, diff);
}

BEGIN_BENCHMARKS();

BENCHMARK(directedEdgeToBoundary, 1, {
    for (int i = 0; i <= 6; i++) {
        do_res_sum(i);
    }
});

END_BENCHMARKS();
