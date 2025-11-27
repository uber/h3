#ifndef ADDER_H
#define ADDER_H

typedef struct {
    double sum;  // running total
    double c;    // compensation term
} Kahan;

static inline void kadd(Kahan *k, double x) {
    double y = x - k->c;
    double t = k->sum + y;
    k->c = (t - k->sum) - y;
    k->sum = t;
}

#endif
