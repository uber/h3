/*
 * Copyright 2017-2018 Uber Technologies, Inc.
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
/** @file mathExtensions.c
 * @brief   Math functions that should've been in math.h but aren't
 */

#include "mathExtensions.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * _ipow does integer exponentiation efficiently. Taken from StackOverflow.
 *
 * @param base the integer base
 * @param exp the integer exponent
 *
 * @return the exponentiated value
 */
int _ipow(int base, int exp) {
    int result = 1;
    while (exp) {
        if (exp & 1) result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

#define S0 1.58962301576546568060E-10
#define S1 -2.50507477628578072866E-8
#define S2 2.75573136213857245213E-6
#define S3 -1.98412698295895385996E-4
#define S4 8.33333333332211858878E-3
#define S5 -1.66666666666666307295E-1

#define C0 -1.13585365213876817300E-11
#define C1 2.08757008419747316778E-9
#define C2 -2.75573141792967388112E-7
#define C3 2.48015872888517045348E-5
#define C4 -1.38888888888730564116E-3
#define C5 4.16666666666665929218E-2

#define PI4A 7.85398125648498535156E-1
#define PI4B 3.77489470793079817668E-8
#define PI4C 2.69515142907905952645E-15
#define M4PI 1.273239544735162542821171882678754627704620361328125  // 4/pi

// Based on Go implementation, which is based on Cephes library implementation.
// See here for details:
// https://github.com/golang/go/blob/master/src/math/sin.go.
void _sincos(double x, double* sin, double* cos) {
    assert(sin != NULL);
    assert(cos != NULL);

    if (x == 0) {
        *sin = x;
        *cos = 1;
        return;
    }

    if (isnan(x) || isinf(x)) {
        *sin = NAN;
        *cos = NAN;
        return;
    }

    bool sinSign = false;
    bool cosSign = false;
    if (x < 0) {
        x = -x;
        sinSign = true;
    }

    // Integer part of x/(pi/4), as integer for tests on the phase angle
    int64_t j = x * M4PI;
    double y = j;

    // Map zeros to origin.
    if ((j & 1) == 1) {
        j++;
        y++;
    }

    // Octant modulo 2*pi radians.
    j &= 7;

    // Reflect in x axis.
    if (j > 3) {
        j -= 4;
        sinSign = !sinSign;
        cosSign = !cosSign;
    }

    if (j > 1) {
        cosSign = !cosSign;
    }

// Extended precision modular arithmetic.
#define Z (((x - y * PI4A) - y * PI4B) - (y * PI4C))
#define ZZ ((Z) * (Z))

    double cosValue =
        1.0 - 0.5 * ZZ +
        ZZ * ZZ *
            ((((((C0 * ZZ) + C1) * ZZ + C2) * ZZ + C3) * ZZ + C4) * ZZ + C5);
    double sinValue =
        Z +
        Z * ZZ *
            ((((((S0 * ZZ) + S1) * ZZ + S2) * ZZ + S3) * ZZ + S4) * ZZ + S5);
#undef Z
#undef ZZ

    if (j == 1 || j == 2) {
        double tmp = sinValue;
        sinValue = cosValue;
        cosValue = tmp;
    }
    if (cosSign) {
        cosValue = -cosValue;
    }
    if (sinSign) {
        sinValue = -sinValue;
    }

    *sin = sinValue;
    *cos = cosValue;
}
