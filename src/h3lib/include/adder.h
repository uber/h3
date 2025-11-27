#ifndef ADDER_H
#define ADDER_H

typedef struct {
    double sum;  // running total
    double c;    // compensation term
} Kahan;

static inline void simple_add(Kahan *k, double x) { k->sum += x; }

static inline void kahan_add(Kahan *k, double x) {
    double y = x - k->c;
    double t = k->sum + y;
    k->c = (t - k->sum) - y;
    k->sum = t;
}

static inline void neumaier_add(Kahan *k, double x) {
    double t = k->sum + x;

    if (fabs(k->sum) >= fabs(x)) {
        k->c += (k->sum - t) + x;
    } else {
        k->c += (x - t) + k->sum;
    }

    k->sum = t;
}

static inline double simple_result(Kahan k) { return k.sum; }
static inline double kahan_result(Kahan k) { return k.sum; }
static inline double neumaier_result(Kahan k) { return k.sum + k.c; }

static inline void kadd(Kahan *k, double x) {
    // simple_add(k, x);
    // kahan_add(k, x);
    neumaier_add(k, x);
}

static inline double kresult(Kahan k) {
    // return simple_result(k);
    // return kahan_result(k);
    return neumaier_result(k);
}

#endif
