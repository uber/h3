# Hierarchical grid functions

These functions permit moving between resolutions in the H3 grid system. The functions produce parent (coarser) or children (finer) cells.

## h3ToParent

```
H3Index h3ToParent(H3Index h, int parentRes);
```

Returns the parent (coarser) index containing `h`.

## h3ToChildren

```
void h3ToChildren(H3Index h, int childRes, H3Index *children);
```

Populates `children` with the indexes contained by `h` at resolution `childRes`. `children` must be an array of at least size `maxH3ToChildrenSize(h, childRes)`.

### maxH3ToChildrenSize

```
int maxH3ToChildrenSize(H3Index h, int childRes);
```

Returns the size of the array needed by `h3ToChildren` for these inputs.

## h3ToCenterChild

```
H3Index h3ToCenterChild(H3Index h, int childRes);
```

Returns the center child (finer) index contained by `h` at resolution `childRes`.

## compact

```
int compact(const H3Index *h3Set, H3Index *compactedSet, const int numHexes);
```

Compacts the set `h3Set` of indexes as best as possible, into the array `compactedSet`. `compactedSet` must be at least the size of `h3Set` in case the set cannot be compacted.

Returns 0 on success.

## uncompact

```
int uncompact(const H3Index *compactedSet, const int numHexes, H3Index *h3Set, const int maxHexes, const int res);
```

Uncompacts the set `compactedSet` of indexes to the resolution `res`. `h3Set` must be at least of size `maxUncompactSize(compactedSet, numHexes, res)`.

Returns 0 on success.

### maxUncompactSize

```
int maxUncompactSize(const H3Index *compactedSet, const int numHexes, const int res)
```

Returns the size of the array needed by `uncompact`.
