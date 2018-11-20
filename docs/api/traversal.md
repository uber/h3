# Grid traversal functions

## kRing

```
void kRing(H3Index origin, int k, H3Index* out);
```

k-rings produces indices within k distance of the origin index.

k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
all neighboring indices, and so on.

Output is placed in the provided array in no particular order. Elements of
the output array may be left zero, as can happen when crossing a pentagon.

### maxKringSize

```
int maxKringSize(int k);
```

Maximum number of indices that result from the kRing algorithm with the given k.

## kRingDistances

```
void kRingDistances(H3Index origin, int k, H3Index* out, int* distances);
```

k-rings produces indices within k distance of the origin index.

k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
all neighboring indices, and so on.

Output is placed in the provided array in no particular order. Elements of
the output array may be left zero, as can happen when crossing a pentagon.

## hexRange

```
int hexRange(H3Index origin, int k, H3Index* out);
```

hexRange produces indexes within k distance of the origin index.
Output behavior is undefined when one of the indexes returned by this
function is a pentagon or is in the pentagon distortion area.

k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
all neighboring indexes, and so on.

Output is placed in the provided array in order of increasing distance from
the origin.

Returns 0 if no pentagonal distortion is encountered.
 
## hexRangeDistances

```
int hexRangeDistances(H3Index origin, int k, H3Index* out, int* distances);
```

hexRange produces indexes within k distance of the origin index.
Output behavior is undefined when one of the indexes returned by this
function is a pentagon or is in the pentagon distortion area.

k-ring 0 is defined as the origin index, k-ring 1 is defined as k-ring 0 and
all neighboring indexes, and so on.

Output is placed in the provided array in order of increasing distance from
the origin. The distances in hexagons is placed in the distances array at
the same offset.
 
Returns 0 if no pentagonal distortion is encountered.

## hexRanges

```
int hexRanges(H3Index* h3Set, int length, int k, H3Index* out);
```

hexRanges takes an array of input hex IDs and a max k-ring and returns an
array of hexagon IDs sorted first by the original hex IDs and then by the
k-ring (0 to max), with no guaranteed sorting within each k-ring group.

Returns 0 if no pentagonal distortion was encountered. Otherwise, output
is undefined

## hexRing

```
int hexRing(H3Index origin, int k, H3Index* out);
```

Produces the hollow hexagonal ring centered at origin with sides of length k.
 
Returns 0 if no pentagonal distortion was encountered.

## h3Distance

```
int h3Distance(H3Index origin, H3Index h3);
```

Returns the distance in grid cells between the two indexes.

Returns a negative number if finding the distance failed. Finding the distance can fail because the two
indexes are not comparable (different resolutions), too far apart, or are separated by pentagonal
distortion. This is the same set of limitations as the local IJ coordinate space functions.

## experimentalH3ToLocalIj

```
int experimentalH3ToLocalIj(H3Index origin, H3Index h3, CoordIJ *out);
```

Produces local IJ coordinates for an **H3** index anchored by an origin.

This function is experimental, and its output is not guaranteed
to be compatible across different versions of **H3**.

## experimentalLocalIjToH3

```
int experimentalLocalIjToH3(H3Index origin, const CoordIJ *ij, H3Index *out);
```

Produces an **H3** index from local IJ coordinates anchored by an origin.

This function is experimental, and its output is not guaranteed
to be compatible across different versions of **H3**.
