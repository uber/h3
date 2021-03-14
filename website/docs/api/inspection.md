# Index inspection functions

These functions provide metadata about an H3 index, such as its resolution or base cell, and provide utilities for converting into and out of the 64-bit representation of an H3 index.

## h3GetResolution

```
int h3GetResolution(H3Index h);
```

Returns the resolution of the index.

## h3GetBaseCell

```
int h3GetBaseCell(H3Index h);
```

Returns the base cell number of the index.

## stringToH3

```
H3Index stringToH3(const char *str);
```

Converts the string representation to `H3Index` (`uint64_t`) representation.

Returns 0 on error.

## h3ToString

```
void h3ToString(H3Index h, char *str, size_t sz);
```

Converts the `H3Index` representation of the index to the string representation. `str` must be at least of length 17.

## h3IsValid

```
int h3IsValid(H3Index h);
```

Returns non-zero if this is a valid **H3** index.

## h3IsResClassIII

```
int h3IsResClassIII(H3Index h);
```

Returns non-zero if this index has a resolution with Class III orientation.

## h3IsPentagon

```
int h3IsPentagon(H3Index h);
```

Returns non-zero if this index represents a pentagonal cell.

## h3GetFaces

```
void h3GetFaces(H3Index h, int* out);
```

Find all icosahedron faces intersected by a given H3 index and places them in the array `out`. `out` must be at least of length `maxFaceCount(h)`.

Faces are represented as integers from 0-19, inclusive. The array is sparse, and empty (no intersection) array values are represented by -1.

### maxFaceCount

```
int maxFaceCount(H3Index h3);
```

Returns the maximum number of icosahedron faces the given H3 index may intersect.
