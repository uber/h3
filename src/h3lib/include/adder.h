#ifndef ADDER_H
#define ADDER_H

/*
Header-only implementation of a "compensated summation" algorithm (Kahan
summation), which allows us to add up sequences of floating-point numbers
with better precision than naive summation, especially when the terms in
the sum vary significantly in magnitude.
See: https://en.wikipedia.org/wiki/Kahan_summation_algorithm

This is useful when computing the area of (multi)polygons, which
often involves adding many small terms to a large aggregate. For example,
D3 uses an improved accuracy summation when computing polygonal areas via its
`Adder` class:
https://github.com/d3/d3-geo/blob/main/src/area.js

There are a few potential algorithms we might consider for summation:

1. Naive sum
2. Kahan summation
3. Neumaier summation
4. Other approaches like pairwise summation, or Python's `fsum`

We considered the first three for simplicity, and settled on Kahan summation:
it achieves noticeably better accuracy than naive summation and almost as
good accuracy as Neumaier, while being only slightly slower than naive and
slightly faster and simpler than Neumaier.

See also: https://github.com/python/cpython/issues/100425 for discussion of
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
    simple_add(adder, x);
    // kahan_add(adder, x);
    // neumaier_add(adder, x);
}

static inline double kresult(Adder adder) {
    return simple_result(adder);
    // return kahan_result(adder);
    // return neumaier_result(adder);
}

#endif
