#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#include "adder.h"
#include "benchmark.h"
#include "h3api.h"
#include "iterators.h"
#include "utility.h"

// TODO: turn this into a benchmark, report accuracy and time
// see if we get any benefit by avoiding the repeated sin/cos computation

// TODO: document algos:
// https://en.wikipedia.org/wiki/Kahan_summation_algorithm#Further_enhancements
// TODO: better more general name for accumulator struct and functions
// TOOD: demonstrate the area alg works for global polygons.

// TODO: is kadd faster without pointers? k = kadd(k, cellArea);

static void do_res_sum(int res) {
    Kahan k = {0, 0};
    IterCellsResolution iter = iterInitRes(res);

    for (; iter.h; iterStepRes(&iter)) {
        double cellArea;
        H3_EXPORT(cellAreaRads2)(iter.h, &cellArea);

        kadd(&k, cellArea);
    }

    double area = kresult(k);
    double diff = fabs(area - 4 * M_PI);
    printf("res: %d, diff: %e\n", res, diff);
}

BEGIN_BENCHMARKS();

BENCHMARK(directedEdgeToBoundary, 1, {
    for (int i = 0; i <= 8; i++) {
        do_res_sum(i);
    }
});

END_BENCHMARKS();
