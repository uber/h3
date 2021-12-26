#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "h3api.h"
#include "mathExtensions.h"

#define HIGH_BITS(h, t) ((h) >> (64 - (t)))

static int cmpLow52(H3Index a, H3Index b) {
    a <<= 12;
    b <<= 12;

    if (a < b) return -1;
    if (a > b) return +1;
    return 0;
}

// todo: could have a version that assumes a <= b already. save any time?
static int cmpCanon(H3Index a, H3Index b) {
    // skip high, mode, reserved bits
    a <<= 8;
    b <<= 8;

    // pull 4 resolution bits
    int res_a = HIGH_BITS(a, 4);
    int res_b = HIGH_BITS(b, 4);

    // move past 4 resolution bits
    a <<= 4;
    b <<= 4;

    // 7 bits for base cell, 3 for each shared resolution level
    int common = 7 + 3 * MIN(res_a, res_b);
    bool are_related = (HIGH_BITS(a, common) == HIGH_BITS(b, common));

    if (are_related) {
        if (a < b) return -1;
        if (a > b) return +1;
    } else {
        if (a < b) return -2;
        if (a > b) return +2;
    }

    return 0;
}

static int cmpLow52Ptr(const void *a_, const void *b_) {
    H3Index a = *(const H3Index *)a_;
    H3Index b = *(const H3Index *)b_;

    return cmpLow52(a, b);
}

int isLow52Sorted(const H3Index *cells, const int64_t N) {
    // note: returns true if N == 0 or N == 1
    for (int64_t i = 1; i < N; i++) {
        if (cmpLow52(cells[i - 1], cells[i]) <= 0) {
            // this pair is OK
        } else {
            return false;
        }
    }
    return true;
}

/*
Return True if cells is low52 ordered, and there no children/descendants/dupes.

TODO: could maybe speed up by keeping resolution calculation since cells
appear in both LHS and RHS
 */
int isCanonicalCells(const H3Index *cells, const int64_t N) {
    for (int64_t i = 0; i < N; i++) {
        if (!H3_EXPORT(isValidCell)(cells[i])) {
            return false;
        }
    }

    // note: returns true if N == 0 or N == 1
    for (int64_t i = 1; i < N; i++) {
        if (cmpCanon(cells[i - 1], cells[i]) == -2) {
            // this pair is OK
        } else {
            return false;
        }
    }
    return true;
}

// note that this places any zeros at the start of the array
H3Error low52Sort(H3Index *cells, const int64_t N) {
    qsort(cells, N, sizeof(H3Index), cmpLow52Ptr);
    return E_SUCCESS;
}

// if c is a descendant of p (including being the same cell)
static bool isDesc(H3Index c, H3Index p) {
    int x = cmpCanon(c, p);
    return ((x == -1) || (x == 0));
}

/*
Remove cells which have a parent in the sorted array.
Assume the cell array is ordered by the lower52 order.
Walk from the right to the left, looking for descendants.

TODO: we can maybe do the descendant test even faster, since we
assume the array is sorted!
 */
static void setDescToZero(H3Index *cells, const int64_t N) {
    H3Index p = 0;  // current parent

    for (int64_t i = N - 1; i >= 0; i--) {
        if (cells[i] == 0) {
            continue;
        }

        if (p == 0) {
            p = cells[i];
        } else if (isDesc(cells[i], p)) {
            cells[i] = 0;
        } else {
            p = cells[i];
        }
    }
}

/*
Shift all the nonzero elements of the array to the left while preserving
their order. Return the number of nonzero elements.
 */
static int64_t shiftOutZeros(H3Index *cells, const int64_t N) {
    int64_t i, k;

    for (k = 0; k < N; k++) {
        if (cells[k] == 0) {
            break;
        }
    }

    for (i = 0; i < N; i++) {
        if ((cells[i] != 0) && (k < i)) {
            cells[k] = cells[i];
            cells[i] = 0;
            k++;
        }
    }

    return k;
}

/*
Canonicalize an array of cells. The array is permitted to have
valid H3 cells and H3_NULL as elements.

numAfter is the number of nonzero cells in the canonicalized array.
All nonzero elements are moved to the front/left of the array.
 */
H3Error canonicalizeCells(H3Index *cells, const int64_t numBefore,
                          int64_t *numAfter) {
    low52Sort(cells, numBefore);
    setDescToZero(cells, numBefore);
    *numAfter = shiftOutZeros(cells, numBefore);

    return E_SUCCESS;
}

// bsearch works on any canonical (maybe just low52ordered), but is faster on
// canonical compact low52ordered < canonical < canonical compact? feature
// matrix comparison bool is_canonical_compacted(const H3Index *cells, const
// int64_t N); // don't add this

/*
bswarch works on low52_sorted. faster on canonical. fastest on canonical
compacted.
 */

/*
    Compact hex set binary search.

    Determine if h is in s_hexes, or a child of any hex in s_hexes.

    s_hexes is sorted asc by lower52(). this means children are to the left
    of parents.

    ## Loop invariant

    We know that h is not before i or after j, so we only search
    in the interval [i, j].

 */

/*

i = 0
j = 7
k = 3

| . | . | . | X | . | . | . |
i           k               j


two outcomes if we don't find a match:

| . | . | . |
i           j


                | . | . | . |
                i           j


i = 0
j = 2
k = 1

| . | X |
 \   \   \
  i   k   j


i = 0
j = 1
k = 0

| X |
i   j
k


| X |
 \   \
  i,k \
       j

i = 0
j = 0
k = 0

|    (empty array)
 \
  i,j,k

 */

/*
At each iteration, we can select any k from i to j-1.
Typically, we'll just select a k midway between i and j.
This strategy makes us test the endpoints of the array first,
hoping for an early exit if h is clearly not in the set.
This is totally a heuristic, but should work well on typical
"clumps" of geo data.
 */
static inline int64_t kStrategy(int64_t i, int64_t j, int64_t N) {
    if (i == 0) return 0;
    if (j == N) return N - 1;

    return i + (j - i) / 2;
}

int canonSearch(const H3Index *cells, const int64_t N, const H3Index h) {
    int64_t i = 0;
    int64_t j = N;

    while (i < j) {
        int64_t k = kStrategy(i, j, N);
        int cmp = cmpCanon(h, cells[k]);

        if (cmp == -1 || cmp == 0) {
            return true;
        } else if (cmp == -2) {
            j = k;
        } else if (cmp == +2) {
            i = k + 1;
        } else {
            // This conclusion depends on `cells` being canonical.
            // Not true if it is only low 52 sorted.
            // TODO: also provide one that works on just low 52 arrays?
            return false;
        }
    }

    return false;
}

// could also just take in a search interval struct...
// returns -1 if h intersects with cells[i:j]
static int64_t disjointInsertionPoint(const H3Index *cells, int64_t i,
                                      int64_t j, const H3Index h) {
    while (i < j) {
        int64_t k = i + (j - i) / 2;
        int cmp = cmpCanon(h, cells[k]);

        if (cmp == -2) {
            j = k;
        } else if (cmp == +2) {
            i = k + 1;
        } else {
            // cmp == -1, +1, or 0
            return -1;  // h intersects with cells!
        }
    }

    return i;
}

typedef struct {
    const H3Index *cells;
    int64_t N, i, j;
} SearchInterval;

static bool wayLessThan(const SearchInterval A, const SearchInterval B) {
    if (A.N > 0 && B.N > 0 && (cmpCanon(A.cells[A.N - 1], B.cells[0]) == -2)) {
        return true;
    } else {
        return false;
    }
}

static void ensureASmaller(SearchInterval *A, SearchInterval *B) {
    // ensure A is the smaller of the two sets.
    SearchInterval temp;

    if ((B->j - B->i) < (A->j - A->i)) {
        temp = *A;
        *A = *B;
        *B = temp;
    }
}

/*
Double binary search for a fast intersection test on canonical sets.
Faster if compact and canonical.

Yoda naming until we come up with something better
 */
int intersectTheyDo(const H3Index *_A, const int64_t aN, const H3Index *_B,
                    const int64_t bN) {
    SearchInterval A = {.cells = _A, .N = aN, .i = 0, .j = aN};
    SearchInterval B = {.cells = _B, .N = bN, .i = 0, .j = bN};

    // check for a quick exit
    if (wayLessThan(A, B)) return false;
    if (wayLessThan(B, A)) return false;

    bool usingLeft = true;

    while ((A.i < A.j) && (B.i < B.j)) {
        ensureASmaller(&A, &B);

        // take A[i] or A[j-1] and see what happens when we look into B[i:j]
        usingLeft = !usingLeft;
        H3Index h = (usingLeft) ? A.cells[A.i] : A.cells[A.j - 1];
        int64_t k = disjointInsertionPoint(B.cells, B.i, B.j, h);

        if (k == -1) return true;  // h found in B, so they intersect!

        if (usingLeft) {
            B.i = k;
            A.i++;
        } else {
            B.j = k;
            A.j--;
        }
    }

    return false;
}

/*
Just for comparison:

This implementation is also **correct**, but I'm guessing it will be slower
on real data. The implementation above has a few heuristics that I think
will help with speed.
 */
int intersectTheyDo_slow(const H3Index *_A, const int64_t aN, const H3Index *_B,
                         const int64_t bN) {
    SearchInterval A = {.cells = _A, .N = aN, .i = 0, .j = aN};
    SearchInterval B = {.cells = _B, .N = bN, .i = 0, .j = bN};

    while ((A.i < A.j) && (B.i < B.j)) {
        // take A[i] and see what happens when we look into B[i:j]
        H3Index h = A.cells[A.i];
        int64_t k = disjointInsertionPoint(B.cells, B.i, B.j, h);

        if (k == -1) return true;  // they intersect!

        B.i = k;
        A.i++;
    }

    return false;
}
