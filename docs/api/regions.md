# Region functions

These functions convert H3 indexes to and from polygonal areas.

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
Polygon outlines will follow GeoJSON MultiPolygon order: Each polygon will
have one outer loop, which is first in the list, followed by any holes.

It is the responsibility of the caller to call destroyLinkedPolygon on the
populated linked geo structure, or the memory for that structure will
not be freed.

It is expected that all hexagons in the set have the same resolution and
that the set contains no duplicates. Behavior is undefined if duplicates
or multiple resolutions are present, and the algorithm may produce
unexpected or invalid output.

### destroyLinkedPolygon

```
void destroyLinkedPolygon(LinkedGeoPolygon* polygon);
```

Free all allocated memory for a linked geo structure. The caller is
responsible for freeing memory allocated to the input polygon struct.
