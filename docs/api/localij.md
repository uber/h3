# Local IJ coordinate functions

These functions operate on a local IJ coordinate space. The
coordinates are anchored by an **origin index**. The I and J
axes are 120&deg; apart. The origin index might not be at
(0, 0) in the local coordinate system.

LOcal IJ coordinates are only valid within a certain radius of the origin.
The coordinate space used by these functions may have deleted regions or
warping due to pentagonal distortion.

## experimentalH3ToLocalIj

```
int experimentalH3ToLocalIj(H3Index origin, H3Index h3, CoordIJ *out);
```

Produces local IJ coordinates for an index anchored by an origin.

This function is experimental, and its output is not gauranteed
to be compatible across different versions of H3.

## experimentalLocalIjToH3

```
int experimentalLocalIjToH3(H3Index origin, const CoordIJ *ij, H3Index *out);
```

Produces an index for local IJ coordinates anchored by an origin.

This function is experimental, and its output is not gauranteed
to be compatible across different versions of H3.

## h3Distance

```
int h3Distance(H3Index origin, H3Index h3);
```

Returns the distance in grid cells between the two indexes.

Returns a negative number if finding the distance failed. Finding the distance can fail because the two
indexes are not comparable (different resolutions), too far apart, or are separated by pentagonal
distortion.
