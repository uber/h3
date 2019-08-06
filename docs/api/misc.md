# Miscellaneous H3 functions

These functions include descriptions of the H3 grid system.

## degsToRads

```
double degsToRads(double degrees);
```

Converts degrees to radians.

## radsToDegs

```
double radsToDegs(double radians);
```

Converts radians to degrees.

## hexAreaKm2

```
double hexAreaKm2(int res);
```

Average hexagon area in square kilometers at the given resolution.

## hexAreaM2

```
double hexAreaM2(int res);
```

Average hexagon area in square meters at the given resolution.

## edgeLengthKm

```
double edgeLengthKm(int res);
```

Average hexagon edge length in kilometers at the given resolution.

## edgeLengthM

```
double edgeLengthM(int res);
```

Average hexagon edge length in meters at the given resolution.

## numHexagons

```
int64_t numHexagons(int res);
```

Number of unique **H3** indexes at the given resolution.

## getRes0Indexes

```
void getRes0Indexes(H3Index *out);
```

All the resolution 0 **H3** indexes.
`out` must be an array of at least size `res0IndexCount()`.

## res0IndexCount

```
int res0IndexCount();
```

Number of resolution 0 **H3** indexes.

## getPentagonIndexes

```
void getPentagonIndexes(int res, H3Index *out);
```

All the pentagon **H3** indexes at the specified resolution.
`out` must be an array of at least size `pentagonIndexCount()`.

## pentagonIndexCount

```
int pentagonIndexCount();
```

Number of pentagon **H3** indexes per resolution. This is always 12, but provided as a convenience.
