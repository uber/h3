# Region functions

## polyfill

```
void polyfill(const GeoPolygon* geoPolygon, int res, H3Index* out);
```

polyfill takes a given GeoJSON-like data structure and preallocated,
zeroed memory, and fills it with the hexagons that are contained by
the GeoJSON-like data structure.

The current implementation is very primitive and slow, but correct,
performing a point-in-poly operation on every hexagon in a k-ring defined
around the given geofence.

### maxPolyfillSize

```
int maxPolyfillSize(const GeoPolygon* geoPolygon, int res);
```

maxPolyfillSize returns the number of hexagons to allocate space for when
performing a polyfill on the given GeoJSON-like data structure.

## h3SetToLinkedGeo

```
void h3SetToLinkedGeo(const H3Index* h3Set, const int numHexes, LinkedGeoPolygon* out);
```

Create a LinkedGeoPolygon describing the outline(s) of a set of  hexagons.
It is the responsibility of the caller to call destroyLinkedPolygon on the
populated linked geo structure, or the memory for that structure will
not be freed.

It is expected that all hexagons in the set will have the same resolution.
Using different resolution hexagons is not recommended.

At present, if the set of hexagons is not contiguous, this function
will return a single polygon with multiple outer loops. The correct GeoJSON
output should only have one outer loop per polygon. It appears that most
GeoJSON consumers are fine with the first input format, but it's less correct
than the second format. The function may be updated to produce
multiple polygons in the future.

### destroyLinkedPolygon

```
void destroyLinkedPolygon(LinkedGeoPolygon* polygon);
```

Free all allocated memory for a linked geo structure. The caller is
responsible for freeing memory allocated to input polygon struct.
