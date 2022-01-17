# RFC: Canonicalization for H3 cell sets

* **Authors**: AJ Friend
* **Date**: 2021-12-122
* **Status**: Draft

## Abstract

We propose a canonical form for **sets** of H3 cells based on
the "lower 52 bit" ordering. We also introduce fast "spatial join" operations
on the cell sets (like cell-in-set, intersection, union, etc.) that exploit
the canonical structure for speed gains.


## Motivation

A canonical form for cell sets is useful when testing if two sets are equal.
That is, we'd like to be able to tell if two H3 cell arrays represent
the same mathematical set of cells, ignoring ordering or duplicated cells.

If we have a function to canonicalize an H3 cell array, then we would
consider two arrays to be equivalent (as sets) if they each canonicalize
to the exact same (canonical) cell array.

A canonical form is also useful if a user wanted to deterministically
hash H3 cell sets, and wanted the hash to be independent of ordering
or duplicates.

The canonical form we'll propose also has the added benefit of allowing
for fast "spatial join" operations on canonicalized sets. For example,
we'll be able to do a fast binary search to see if a cell is a member
of a set, and an **even faster** binary search if the set is both
compacted and canonicalized.

We'll get the same benefits computing the intersection of sets, or
simply testing for intersection.

The canonical form also suggests a new "in-memory" cell compaction algorithm,
which avoids any dynamic memory allocation. This new compact algorithm
has the added benefit of returning cell arrays already in canonical form.

## Terminology

We propose a canonical form based on the "lower 52 bit" ordering, that is,
the ordering you would get if you only considered the lower 52 bits of the
H3 cell indexes. The lower 52 bits of an H3 index consist of 7 bits for the
base cell, and 3 bits for each of the 15 resolution digits. That sums up
to `7 + 3*15 = 52`.

We'll only define this ordering for H3 **cells**. We're not considering
vertices or edges in this RFC.

The lower 52 bit ordering can be implemented, for example, by
the `cmpLow52` comparison function given below.


```c
int cmpLow52(H3Index a, H3Index b) {
    a <<= 12;
    b <<= 12;

    if (a < b) return -1;
    if (a > b) return +1;
    return 0;
```

This ordering has the property that children cells are always less than
their parent cells. Ordered in an array with cells of multiple resolutions,
children cells are always to the left of their parents.

We can also get slightly richer ordering information with a comparison function
with a declaration like

```c
int cmpCanon(H3Index a, H3Index b);
```

defined so that:

- `cmpCanon(a, b) ==  0` if `a` and `b` are the same cell
- `cmpCanon(a, b) == -1` if `a` is a child (or further descendant) of `b`
- `cmpCanon(a, b) == +1` if `b` ... `a`
- `cmpCanon(a, b) == -2` if `a` < `b` in the low52 ordering, but they are not related
- `cmpCanon(a, b) == +2` if `b` < `a` ...

Note that these two functions produce the same ordering when given to
the C standard library's `qsort`.

### Array classifications

Given these comparison functions, we can define 3 increasingly strict properties
on arrays of H3 cells:

1. "lower 52" ordered
2. canonical
3. compacted and canonical

#### Low52 ordered

An H3 cell array `a` is "low52 ordered" if its elements are such that

- `cmpLow52(a[i-1], a[i]) <= 0` or, equivalently,
- `cmpCanon(a[i-1], a[i]) <= 0`.

Note that in this classification, arrays can have duplicated cell. We can also
have the parents, children, ancestors, or descendants of other cells in
the array.

### Canonical

We'll define a "canonical" H3 cell array to be one that is low52 ordered and
has the additional property that no duplicates, parents, children, ancestors,
or descendants of other cells are in the array.

We can check this property by ensuring that

```c
cmpCanon(a[i-1], a[i]) == -2
```

for each adjacent pair of cells in the array.

### Compacted and canonical

A compacted and canonical H3 set is just what it sounds like.

Many of the fast spatial join operations will work on canonical sets, but
will be faster on compacted canonical sets.

## Proposal

