# RFC: Polyfill modes

* **Authors**: Isaac Brodsky (@isaacbrodsky)
* **Date**: September 17, 2021
* **Status**: Draft

## Abstract

Our current polyfill algorithm allocates cells to polygons based on whether the center of the cell falls within the polygon, using Cartesian geometry. This proposal outlines options for expanding the polyfill algorithm to allow for different containment modes (e.g. wholly contained cells, intersecting cells) and spherical geometry.

## Motivation

*Why is this important?*

* Inclusion test

A use case we want to support is testing whether a point could possibly be in a polygon by set containment. In Java this would look like:

```java
final int R = ...;
// Indexed as res R
Set<String> cellsCoveringPolygon = ...;

public boolean polygonContainsPoint(double latitude, double longitude) {
    String h3Index = h3.latLngToCell(latitude, longitude, R);
    if (cellsCoveringPolygon.contains(h3Index)) {
        // The polygon possibly contains the point, so use a more expensive point
        // in polygon algorithm to determine.
        return polygonContainsPointExpensive(latitude, longitude);
    }
    // The polygon definitely does not contain the point, the expensive check
    // is not needed.
    return false;
}
```

* Analytics

There is no convenient function to index a polygon into the H3 grid for analytics purposes. Polygons that are small enough or shaped in specific ways will not be converted into cells, requiring workarounds in order to use them.

* Spherical geometry consistency

Most of the H3 library uses spherical geoemtry. For example, cell boundaries are spherical hexagons
and pentagons. The `polyfill` function is different that it assumes Cartesian geometry. For
consistency with the rest of the library, the `polyfill` functions should be able to use the same
cell boundaries.

Maintaining a Cartesian option is useful for cases where polygons have been drawn on a projected map
and the boundaries should be the same.

* Very large polygons

Polyfills of very large polygons require allocating large blocks of memory, and spending large
amounts of time in a library function without progress information being available to the caller.
(To be determined if this is in scope for this RFC or for another.)

## Approaches

*What are the various options to address this issue?*

On an API level, we will need to expose the containment mode and spherical/Cartesian choice as
options to the caller.

Internally, we would like to reuse the same implementation as much as possible, but change the
exact inclusion check used for each mode.

### Comparisons

#### S2

Contains separate functions for [intersection and containment](http://s2geometry.io/devguide/basic_types#s2polygon)

#### Turf library

Contains separate functions for [intersection](http://turfjs.org/docs/#booleanIntersects), [containment](http://turfjs.org/docs/#booleanContains), etc.

#### PostGIS

Contains separate functions for [intersection](https://postgis.net/docs/ST_Intersects.html), [containment](https://postgis.net/docs/ST_Contains.html), etc.

See also [Simple Feature Access - SQL](https://www.ogc.org/standards/sfs).

#### JTS

Separate predicatess. (Reference: [JTS Developer Guide](https://github.com/locationtech/jts/blob/master/doc/JTS%20Developer%20Guide.pdf))

#### GeoPandas

Contains separate functions for [intersection](https://geopandas.org/en/stable/docs/reference/api/geopandas.GeoSeries.intersection.html), [containment](https://geopandas.org/en/stable/docs/reference/api/geopandas.GeoSeries.contains.html), etc.

## Proposal

*What is the recommended approach?*

The signature for `polygonToCells` and `maxPolygonToCellsSize` would be changed as follows:

```
/** @brief maximum number of hexagons that could be in the geoloop */
DECLSPEC H3Error H3_EXPORT(maxPolygonToCellsSize)(const GeoPolygon *geoPolygon,
                                                  int res, uint32_t flags, int64_t *out);

/** @brief hexagons within the given geopolygon */
DECLSPEC H3Error H3_EXPORT(polygonToCells)(const GeoPolygon *geoPolygon,
                                           int res, uint32_t flags, H3Index *out);
```

`flags` would have the following possible bit layout:

|       Bits | Meaning
| ---------- | -------
| 1-2 (LSB)  | If 0, containment mode centroid.<br>If 1, containment mode cover.<br>If 2, containment mode intersects.<br>3 is a reserved value.
| 3          | If 0, spherical containment.<br>If 1, cartesian containment (same as H3 version 3).
| All others | Reserved and must be set to 0.

The same value used for `maxPolygonToCellsSize` must be used for the subsequent call to `polygonToCells`, just as the `GeoPolygon` and `res` must be the same.

In bindings, this could be expressed using enums, for example:

```python
polygon_to_cells(polygon, res=res, cartesian=True, containment=h3.Containment.CENTROID)
```

```js
polygonToCells(polygon, {res, cartesian: true, containment: h3.Containment.CENTROID})
```

```java
polygon(polygon).cartesian(true).containment(h3.Containment.CENTROID).toCells(res)
```
