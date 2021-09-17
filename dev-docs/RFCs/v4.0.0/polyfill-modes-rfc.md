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

There is no convenient function to index a polygon into the H3 grid for analytics purposes. Polygons that are small enough, or shaped in specific ways, will not be converted into points, requiring workarouns in order to use them.

* Spherical geometry consistency

## Approaches

*What are the various options to address this issue?*

### S2

Contains separate functions for [intersection and containment](http://s2geometry.io/devguide/basic_types#s2polygon)

### GeoHash

### Turf library

Contains separate functions for [intersection](http://turfjs.org/docs/#booleanIntersects), [containment](http://turfjs.org/docs/#booleanContains), etc.

### PostGIS

Contains separate functions for [intersection](https://postgis.net/docs/ST_Intersects.html), [containment](https://postgis.net/docs/ST_Contains.html), etc.

See also [https://www.ogc.org/standards/sfs](Simple Feature Access - SQL).

### QGIS

### JTS

### GeoPandas

## Proposal

*What is the recommended approach?*