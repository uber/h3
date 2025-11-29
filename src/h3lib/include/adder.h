#ifndef ADDER_H
#define ADDER_H

/*
This is a header-only implementation of a "compensated summation" algorithm,
which allows us to add up sequences of floating point numbers with better
precision than naive summation, especially when the terms in the sum
can vary in magnitude.
See: https://en.wikipedia.org/wiki/Kahan_summation_algorithm

This is useful when computing the area of (multi-)polygons, which can involve
adding many small terms to large aggregates.
For example, D3 uses this when computing polygonal areas via the `Kahan` class:
https://github.com/d3/d3-geo/blob/main/src/area.js

There are a few potential algorithms we might consider for summation:

1. Naive sum
2. Kahan summation
3. Neumaier summation
4. Other approaches like pairwise summation, or Pythons `fsum`

We considered the first three for simplicity, and settled on Kahan summation,
as it acheives noticably better accuracy than naive summation and almost as good
accuracy as Neumaier, while only being slightly slower than naive, but slightly
faster and simpler than Neumaier.

See also: https://github.com/python/cpython/issues/100425 for discussions of
tradeoffs between Kahan, Neumaier, and `fsum`.
*/

typedef struct {
    double sum;  // running total
    double c;    // compensation term
} Adder;

static inline void simple_add(Adder *adder, double x) { adder->sum += x; }

static inline void kahan_add(Adder *adder, double x) {
    double y = x - adder->c;
    double t = adder->sum + y;
    adder->c = (t - adder->sum) - y;
    adder->sum = t;
}

static inline void neumaier_add(Adder *adder, double x) {
    double t = adder->sum + x;

    if (fabs(adder->sum) >= fabs(x)) {
        adder->c += (adder->sum - t) + x;
    } else {
        adder->c += (x - t) + adder->sum;
    }

    adder->sum = t;
}

static inline double simple_result(Adder adder) { return adder.sum; }
static inline double kahan_result(Adder adder) { return adder.sum; }
static inline double neumaier_result(Adder adder) {
    return adder.sum + adder.c;
}

static inline void kadd(Adder *adder, double x) {
    // simple_add(adder, x);
    // kahan_add(adder, x);
    neumaier_add(adder, x);
}

static inline double kresult(Adder adder) {
    // return simple_result(adder);
    // return kahan_result(adder);
    return neumaier_result(adder);
}

#endif
