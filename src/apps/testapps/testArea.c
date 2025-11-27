#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#include "adder.h"
#include "h3api.h"
#include "iterators.h"
#include "test.h"
#include "utility.h"

static void do_res_sum(int res) {
    Kahan k = {0, 0};

    for (IterCellsResolution iter = iterInitRes(res); iter.h;
         iterStepRes(&iter)) {
        double cellArea;
        H3_EXPORT(cellAreaRads2)(iter.h, &cellArea);

        kadd(&k, cellArea);
    }

    double area = k.sum;
    double diff = fabs(area - 4 * M_PI);
    printf("res: %d, diff: %e\n", res, diff);
}

SUITE(test_for_area) {
    TEST(some_area_test) {
        // the numerical test is how close to 4*pi do we get when adding up all
        // cells at smaller and smaller resolution?

        printf("\n");

        do_res_sum(0);
        do_res_sum(1);
        do_res_sum(2);
        do_res_sum(3);
        do_res_sum(4);
        // do_res_sum(5);
        // do_res_sum(6);
        // do_res_sum(7);
        // do_res_sum(8);

        // res: 0, diff: 7.105427e-15
        // res: 1, diff: 3.907985e-14
        // res: 2, diff: 1.421085e-13
        // res: 3, diff: 4.689582e-13
        // res: 4, diff: 2.161826e-12
        // res: 5, diff: 1.543654e-12
        // res: 6, diff: 1.768363e-11
        // res: 7, diff: 1.490719e-11
        // res: 8, diff: 2.937917e-11
    }
}
